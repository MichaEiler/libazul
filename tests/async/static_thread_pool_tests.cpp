#include <gmock/gmock.h>
#include <impulso/async/static_thread_pool.hpp>
#include <thread>

class static_thread_pool_fixture : public testing::Test
{
};

TEST_F(static_thread_pool_fixture, execute_emptyTask_futureReady)
{
    impulso::async::static_thread_pool thread_pool(1);

    auto result = thread_pool.execute([](){});   
    result.wait();

    ASSERT_TRUE(result.is_ready());
    ASSERT_NO_THROW(result.get());
}

TEST_F(static_thread_pool_fixture, execute_taskEnqueued_success)
{
    impulso::async::static_thread_pool executor(1);

    auto result = executor.execute([](){ return 42; });

    ASSERT_EQ(42, result.get());
}

TEST_F(static_thread_pool_fixture, execute_taskThrowsException_exceptionForwarded)
{
    impulso::async::static_thread_pool executor(1);

    auto result = executor.execute([](){ throw std::invalid_argument(""); });
    ASSERT_THROW(result.get(), std::invalid_argument);
}

TEST_F(static_thread_pool_fixture, execute_multipleTasksAndThreads_success)
{
    impulso::async::static_thread_pool executor(4);

    std::vector<impulso::async::future<int>> results;

    const int tasks_to_execute = 100;

    for (int i = 0; i < tasks_to_execute; ++i)
    {
        auto result = executor.execute([i](){ return i; });
        results.emplace_back(std::move(result));
    }

    for (int i = 0; i < tasks_to_execute; ++i)
    {
        ASSERT_EQ(i, results[i].get());
    }
}

TEST_F(static_thread_pool_fixture, execute_oneDependency_executedInOrder)
{
    impulso::async::static_thread_pool executor(2);

    const auto first_task_id = 1;
    const auto second_task_id = 2;

    std::vector<int> ids;

    auto action1 = [&]() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); ids.push_back(first_task_id); };
    auto action2 = [&]() { ids.push_back(second_task_id); };

    auto future1 = executor.execute(action1);
    auto future2 = executor.execute(action2, future1);

    ASSERT_NO_THROW(future1.get());
    ASSERT_NO_THROW(future2.get());

    ASSERT_EQ(first_task_id, ids[0]);
    ASSERT_EQ(second_task_id, ids[1]);
}

TEST_F(static_thread_pool_fixture, execute_dependencyThrowingException_followingTaskStillExecuted)
{
    impulso::async::static_thread_pool executor(2);

    auto action1 = [&]() { throw std::runtime_error(""); };
    auto action2 = [&]() { };

    auto future1 = executor.execute(action1);
    auto future2 = executor.execute(action2, future1);

    ASSERT_THROW(future1.get(), std::runtime_error);
    ASSERT_NO_THROW(future2.get());
}

TEST_F(static_thread_pool_fixture, execute_multipleDependencies_validExecutionOrder)
{
    std::vector<int> executed_tasks;
    std::mutex mutex_;
    auto log_task_id = [&mutex_, &executed_tasks](const int id) mutable {
        std::lock_guard<std::mutex> lock(mutex_);
        executed_tasks.push_back(id);
    };

    impulso::async::static_thread_pool executor(2);

    // dependency graph:
    // task1   <---- task3  <------ task4
    // task2   <-----------------|

    auto future1 = executor.execute(std::bind(log_task_id, 1));
    auto future2 = executor.execute(std::bind(log_task_id, 2));
    auto future3 = executor.execute(std::bind(log_task_id, 3), future1);
    auto future4 = executor.execute(std::bind(log_task_id, 4), future3, future2);

    ASSERT_NO_THROW(future4.get());

    ASSERT_EQ(3, executed_tasks[2]);
    ASSERT_EQ(4, executed_tasks[3]);
}

TEST_F(static_thread_pool_fixture, execute_multipleTasksNoDependencies_allThreadsOccupied)
{
    std::vector<std::thread::id> utilized_thread_ids;
    impulso::async::static_thread_pool executor(4);

    
    std::mutex mutex_;

    auto log_thread_id = [&mutex_, &utilized_thread_ids]() mutable {
        std::lock_guard<std::mutex> lock(mutex_);
        utilized_thread_ids.push_back(std::this_thread::get_id());
    };

    auto action = [&log_thread_id](){
        log_thread_id();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };

    std::vector<impulso::async::future<void>> futures;
    for (std::size_t i = 0; i < executor.thread_count(); ++i)
    {
        futures.emplace_back(executor.execute(action));
    }
    std::for_each(futures.begin(), futures.end(), [](auto f){ f.wait(); });

    ASSERT_EQ(utilized_thread_ids.size(), executor.thread_count());
}

