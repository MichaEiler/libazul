#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <impulso/async/task.hpp>
#include <stdexcept>
#include <thread>

class task_fixture : public testing::Test
{
};

TEST_F(task_fixture, executeTask_taskThrowsException_exceptionCorrectlyForwarded)
{
    auto task_action = []() { throw std::invalid_argument(""); };
    impulso::async::task_type<void> task(task_action);
    std::future<void> future = task.get_future();

    task();

    ASSERT_THROW(future.get(), std::invalid_argument);
}

TEST_F(task_fixture, executeTask_taskDestroyedResultReady_noExceptionThrown)
{
    std::future<void> future;
    
    {
        auto task_action = [](){};
        impulso::async::task_type<void> task(task_action);
        future = std::move(task.get_future());

        task();
    }

    ASSERT_NO_THROW(future.get());
}

TEST_F(task_fixture, executeTask_taskDestroyedExceptionCached_exceptionCorrectlyForwarded)
{
    std::future<void> future;

    {
        auto task_action = [](){ throw std::invalid_argument(""); };
        impulso::async::task_type<void> task(task_action);
        future = std::move(task.get_future());

        task();
    }

    ASSERT_THROW(future.get(), std::invalid_argument);
}

TEST_F(task_fixture, executeTask_taskProvidesReturnValue_futureReturnsResult)
{
    auto task_action = [](){ return 1337; };
    impulso::async::task_type<int> task(task_action);
    auto future = task.get_future();
    task();

    ASSERT_EQ(1337, future.get());
}

TEST_F(task_fixture, getValue_taskInOtherThread_blocksUntilTaskProcessed)
{
    bool result_available = false;

    impulso::async::task_type<void> task([](){});
    auto future = task.get_future();

    std::thread other_thread([&](){
        future.get();
        result_available = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_FALSE(result_available);

    task();
    other_thread.join();
    ASSERT_TRUE(result_available);   
}

