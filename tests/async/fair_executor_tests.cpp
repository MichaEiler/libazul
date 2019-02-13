#include <chrono>
#include <gmock/gmock.h>
#include <impulso/async/fair_executor.hpp>
#include <stdexcept>
#include <vector>

class fair_executor_fixture : public testing::Test
{
};

TEST_F(fair_executor_fixture, execute_taskEnqueued_success)
{
    impulso::async::fair_executor executor;

    auto result = executor.execute<int>([](){ return 42; });
    ASSERT_EQ(std::future_status::timeout, result.wait_for(std::chrono::milliseconds(10)));

    executor.add_threads(1);
    ASSERT_EQ(42, result.get());
}

TEST_F(fair_executor_fixture, execute_taskThrowsException_exceptionForwarded)
{
    impulso::async::fair_executor executor;
    executor.add_threads(1);

    auto result = executor.execute([](){ throw std::invalid_argument(""); });
    ASSERT_THROW(result.get(), std::invalid_argument);
}

TEST_F(fair_executor_fixture, execute_multipleTasksAndThreads_success)
{
    impulso::async::fair_executor executor;
    executor.add_threads(4);

    std::vector<std::future<int>> results;

    const int tasks_to_execute = 100;

    for (int i = 0; i < tasks_to_execute; ++i)
    {
        auto result = executor.execute<int>([i](){ return i; });
        results.emplace_back(std::move(result));
    }

    for (int i = 0; i < tasks_to_execute; ++i)
    {
        ASSERT_EQ(i, results[i].get());
    }
}
