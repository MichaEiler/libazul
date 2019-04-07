#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <impulso/async/detail/task.hpp>
#include <stdexcept>
#include <thread>

class task_fixture : public testing::Test
{
};

TEST_F(task_fixture, wrapDependencies_twoInputFutures_resultSet)
{
    impulso::async::promise<int> promise;
    auto future = promise.get_future();

    impulso::async::promise<bool> promise2;
    auto future2 = promise2.get_future();

    auto final_future = impulso::async::detail::wrap_dependencies(future, future2);

    ASSERT_FALSE(future.is_ready());
    ASSERT_FALSE(future2.is_ready());
    ASSERT_FALSE(final_future.is_ready());

    promise.set_value(42);

    ASSERT_TRUE(future.is_ready());
    ASSERT_FALSE(future2.is_ready());
    ASSERT_FALSE(final_future.is_ready());

    promise2.set_value(true);

    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future2.is_ready());
    ASSERT_TRUE(final_future.is_ready());
}

TEST_F(task_fixture, wrapDependencies_zeroInputFutures_resultImmediatelySet)
{
    auto future = impulso::async::detail::wrap_dependencies();
    ASSERT_TRUE(future.is_ready());
}

TEST_F(task_fixture, executeTask_taskThrowsException_exceptionCorrectlyForwarded)
{
    auto task_action = []() { throw std::invalid_argument(""); };
    impulso::async::detail::task_type<void> task(task_action);
    impulso::async::future<void> future = task.get_future();

    task();

    ASSERT_THROW(future.get(), std::invalid_argument);
}

TEST_F(task_fixture, executeTask_taskDestroyedResultReady_noExceptionThrown)
{
    impulso::async::future<void> future;
    
    {
        auto task_action = [](){};
        impulso::async::detail::task_type<void> task(task_action);
        future = task.get_future();

        task();
    }

    ASSERT_NO_THROW(future.get());
}

TEST_F(task_fixture, executeTask_taskDestroyedExceptionCached_exceptionCorrectlyForwarded)
{
    impulso::async::future<void> future;

    {
        auto task_action = [](){ throw std::invalid_argument(""); };
        impulso::async::detail::task_type<void> task(task_action);
        future = task.get_future();

        task();
    }

    ASSERT_THROW(future.get(), std::invalid_argument);
}

TEST_F(task_fixture, executeTask_taskProvidesReturnValue_futureReturnsResult)
{
    auto task_action = [](){ return 1337; };
    impulso::async::detail::task_type<int> task(task_action);
    auto future = task.get_future();
    task();

    ASSERT_EQ(1337, future.get());
}

TEST_F(task_fixture, getValue_taskInOtherThread_blocksUntilTaskProcessed)
{
    bool result_available = false;

    impulso::async::detail::task_type<void> task([](){});
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

TEST_F(task_fixture, isReady_noDependency_returnsTrue)
{
    auto task = impulso::async::detail::task_type<void>([](){});
    ASSERT_TRUE(task.is_ready());
}

TEST_F(task_fixture, isReady_dependencyNotReady_returnsFalse)
{
    impulso::async::promise<void> promise;
    auto future = promise.get_future();
    auto task = impulso::async::detail::task_type<void>([](){}, future);
    ASSERT_FALSE(task.is_ready());
}

TEST_F(task_fixture, isReady_dependencyReady_returnsTrue)
{
    impulso::async::promise<void> promise;
    auto future = promise.get_future();
    promise.set_value();
    auto task = impulso::async::detail::task_type<void>([](){}, future);
    ASSERT_TRUE(task.is_ready());
}