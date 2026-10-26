#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace test
{
class Failure : public std::exception
{
  public:
    explicit Failure(std::string message) : message(std::move(message)) {}

    const char *what() const noexcept override { return message.c_str(); }

  private:
    std::string message;
};

struct Case
{
    std::string name;
    std::function<void()> function;
};

inline std::vector<Case> &cases()
{
    static std::vector<Case> value;
    return value;
}

struct Registrar
{
    Registrar(std::string name, std::function<void()> function)
    {
        cases().push_back(Case{std::move(name), std::move(function)});
    }
};

[[noreturn]] inline void fail(const char *file, int line, const std::string &message)
{
    std::ostringstream stream;
    stream << file << ':' << line << ": " << message;
    throw Failure(stream.str());
}

template <typename Exception, typename Function>
void checkThrows(Function &&function, const char *file, int line, const char *expression)
{
    try
    {
        function();
    }
    catch (const Exception &)
    {
        return;
    }
    catch (...)
    {
        fail(file, line, std::string("wrong exception type from ") + expression);
    }
    fail(file, line, std::string("expected an exception from ") + expression);
}

inline int run()
{
    int failures = 0;
    for (const Case &testCase : cases())
    {
        try
        {
            testCase.function();
            std::cout << "[PASS] " << testCase.name << '\n';
        }
        catch (const Failure &failure)
        {
            ++failures;
            std::cout << "[FAIL] " << testCase.name << "\n       " << failure.what() << '\n';
        }
        catch (const std::exception &exception)
        {
            ++failures;
            std::cout << "[FAIL] " << testCase.name << "\n       unexpected exception: " << exception.what() << '\n';
        }
        catch (...)
        {
            ++failures;
            std::cout << "[FAIL] " << testCase.name << "\n       unexpected non-standard exception\n";
        }
    }

    std::cout << "\n" << cases().size() - static_cast<std::size_t>(failures) << " passed, " << failures << " failed, "
              << cases().size() << " total\n";
    return failures == 0 ? 0 : 1;
}
} // namespace test

#define TEST_DETAIL_CONCAT_IMPL(left, right) left##right
#define TEST_DETAIL_CONCAT(left, right) TEST_DETAIL_CONCAT_IMPL(left, right)

#define TEST_CASE(name)                                                                        \
    static void TEST_DETAIL_CONCAT(testFunction, __LINE__)();                                  \
    static ::test::Registrar TEST_DETAIL_CONCAT(testRegistrar, __LINE__)(                     \
        name, TEST_DETAIL_CONCAT(testFunction, __LINE__));                                     \
    static void TEST_DETAIL_CONCAT(testFunction, __LINE__)()

#define CHECK(expression)                                                                      \
    do                                                                                         \
    {                                                                                          \
        if (!(expression))                                                                     \
        {                                                                                      \
            ::test::fail(__FILE__, __LINE__, "check failed: " #expression);                    \
        }                                                                                      \
    } while (false)

#define CHECK_FALSE(expression) CHECK(!(expression))

#define CHECK_EQ(actual, expected)                                                             \
    do                                                                                         \
    {                                                                                          \
        auto testActualValue = (actual);                                                       \
        auto testExpectedValue = (expected);                                                   \
        if (!(testActualValue == testExpectedValue))                                           \
        {                                                                                      \
            ::test::fail(__FILE__, __LINE__, "expected " #actual " == " #expected);            \
        }                                                                                      \
    } while (false)

#define CHECK_THROWS_AS(expression, exceptionType)                                             \
    do                                                                                         \
    {                                                                                          \
        ::test::checkThrows<exceptionType>([&] { (void)(expression); }, __FILE__, __LINE__,    \
                                           #expression);                                       \
    } while (false)
