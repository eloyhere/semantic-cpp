#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>
#include <memory>
#include <condition_variable>
#include <stdexcept>
#include <iostream>

namespace pool
{
class ThreadPool
{
  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::mutex workersMutex;
    std::atomic<bool> active{true};
    std::atomic<bool> emergency{false};
    std::condition_variable condition;
    std::atomic<std::size_t> targetSize{0};
    std::mutex exitMutex;
    std::condition_variable exitCondition;
    std::atomic<std::size_t> exitingCount{0};

    void workerLoop()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] {
                    return !tasks.empty() || !active.load() || emergency.load();
                });
                if (emergency.load())
                {
                    notifyExit();
                    return;
                }
                if (!active.load() && tasks.empty())
                {
                    notifyExit();
                    return;
                }
                if (tasks.empty())
                {
                    continue;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            try
            {
                task();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown exception in worker thread." << std::endl;
            }
        }
    }

    void notifyExit()
    {
        ++exitingCount;
        exitCondition.notify_one();
    }

  public:
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    ~ThreadPool()
    {
        if (active.load())
        {
            shutdown();
        }
    }

    explicit ThreadPool(std::size_t size = std::thread::hardware_concurrency())
        : targetSize(size)
    {
        std::lock_guard<std::mutex> lock(workersMutex);
        for (std::size_t i = 0; i < size; ++i)
        {
            addWorker();
        }
    }

    void addWorker()
    {
        workers.emplace_back([this] { workerLoop(); });
    }

    void increase()
    {
        std::lock_guard<std::mutex> lock(workersMutex);
        addWorker();
        targetSize.store(workers.size());
    }

    void decrease()
    {
        std::unique_lock<std::mutex> workersLock(workersMutex);
        if (workers.empty())
        {
            return;
        }
        targetSize.store(workers.size() - 1);
        workersLock.unlock();

        std::unique_lock<std::mutex> exitLock(exitMutex);
        condition.notify_all();
        exitCondition.wait(exitLock, [this] {
            return exitingCount.load() > 0;
        });
        exitingCount.store(0);

        workersLock.lock();
        for (auto it = workers.begin(); it != workers.end(); ++it)
        {
            if (!it->joinable())
            {
                workers.erase(it);
                break;
            }
        }
    }

    void boot()
    {
        if (!active.exchange(true))
        {
            return;
        }
        emergency.store(false);
        std::size_t desired = targetSize.load();
        std::lock_guard<std::mutex> lock(workersMutex);
        while (workers.size() < desired)
        {
            addWorker();
        }
        condition.notify_all();
    }

    void shutdown()
    {
        if (!active.exchange(false))
        {
            return;
        }
        condition.notify_all();
        std::lock_guard<std::mutex> lock(workersMutex);
        for (auto &worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
        workers.clear();
    }

    void emergencyShutdown()
    {
        emergency.store(true);
        active.store(false);
        condition.notify_all();
        std::lock_guard<std::mutex> lock(workersMutex);
        for (auto &worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
        workers.clear();
    }

    std::future<void> submit(std::function<void()> task)
    {
        auto packaged = std::make_shared<std::packaged_task<void()>>(task);
        auto result = packaged->get_future();
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!active.load() || emergency.load())
            {
                throw std::runtime_error("ThreadPool is not active.");
            }
            tasks.emplace([packaged] { (*packaged)(); });
        }
        condition.notify_one();
        return result;
    }

    template <typename R>
    std::future<R> submit(std::function<R()> task)
    {
        auto packaged = std::make_shared<std::packaged_task<R()>>(task);
        auto result = packaged->get_future();
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!active.load() || emergency.load())
            {
                throw std::runtime_error("ThreadPool is not active.");
            }
            tasks.emplace([packaged] { (*packaged)(); });
        }
        condition.notify_one();
        return result;
    }
};

} // namespace pool