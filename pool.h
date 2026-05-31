#pragma once
#include "function.h"
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>
#include <memory>
#include <condition_variable>
#include <stdexcept>
#include <exception>

namespace pool
{

using Timestamp = function::Timestamp;
using Module = function::Module;
using Runnable = function::Runnable;

template <typename R>
using Supplier = function::Supplier<R>;

class ThreadPool
{
  private:
    std::vector<std::thread> workers;
    std::queue<Runnable> tasks;
    std::mutex mutex;
    std::atomic<bool> active{true};
    std::atomic<bool> emergency{false};
    std::condition_variable condition;
    static std::atomic<std::size_t> instanceCount;
    static std::mutex terminateMutex;
    static bool terminateHandlerRegistered;

    static void registerTerminateHandler()
    {
        std::lock_guard<std::mutex> lock(terminateMutex);
        if (!terminateHandlerRegistered)
        {
            std::set_terminate(onTerminate);
            terminateHandlerRegistered = true;
        }
    }

    static void onTerminate()
    {
        ThreadPool::emergencyShutdownAll();
        std::abort();
    }

  public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool()
    {
        --instanceCount;
        if (active.load())
        {
            shutdown();
        }
    }

    explicit ThreadPool(const Module& size = std::thread::hardware_concurrency())
    {
        ++instanceCount;
        registerTerminateHandler();
        for (Module i = 0; i < size; ++i)
        {
            increase();
        }
    }

    void increase()
    {
        workers.emplace_back([this] {
            while (true)
            {
                Runnable task;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    condition.wait(lock, [this] {
                        return !tasks.empty() || !active.load() || emergency.load();
                    });
                    if (emergency.load())
                    {
                        return;
                    }
                    if (!active.load() && tasks.empty())
                    {
                        return;
                    }
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                try
                {
                    task();
                }
                catch (const std::exception& exception)
                {
                    std::cerr << exception.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Unknown exception in worker thread." << std::endl;
                }
            }
        });
    }

    void decrease()
    {
        if (workers.empty())
        {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace([] {});
        }
        condition.notify_one();
        if (workers.back().joinable())
        {
            workers.back().join();
        }
        workers.pop_back();
    }

    void boot()
    {
        if (!active.exchange(true))
        {
            emergency.store(false);
            condition.notify_all();
        }
    }

    void shutdown()
    {
        if (!active.exchange(false))
        {
            return;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    void emergencyShutdown()
    {
        emergency.store(true);
        active.store(false);
        condition.notify_all();
        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.detach();
            }
        }
    }

    static void emergencyShutdownAll()
    {
        std::lock_guard<std::mutex> lock(terminateMutex);
        emergency.store(true);
        active.store(false);
        condition.notify_all();
    }

    auto submit(const Runnable& task) -> std::future<void>
    {
        if (!active.load() || emergency.load())
        {
            throw std::runtime_error("ThreadPool is not active.");
        }
        auto packaged = std::make_shared<std::packaged_task<void()>>(task);
        auto result = packaged->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace([packaged] {
                (*packaged)();
            });
        }
        condition.notify_one();
        return result;
    }

    template <typename R>
    auto submit(const Supplier<R>& task) -> std::future<R>
    {
        if (!active.load() || emergency.load())
        {
            throw std::runtime_error("ThreadPool is not active.");
        }
        auto packaged = std::make_shared<std::packaged_task<R()>>(task);
        auto result = packaged->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace([packaged] {
                (*packaged)();
            });
        }
        condition.notify_one();
        return result;
    }
};

inline ThreadPool pool;
std::atomic<std::size_t> ThreadPool::instanceCount{0};
std::mutex ThreadPool::terminateMutex;
bool ThreadPool::terminateHandlerRegistered = false;

}