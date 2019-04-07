#include <gmock/gmock.h>
#include <azul/async/static_thread_pool.hpp>
#include <thread>

class StaticThreadPoolTestFixture : public testing::Test
{
};

TEST_F(StaticThreadPoolTestFixture, Execute_EmptyTask_FutureReady)
{
    azul::async::StaticThreadPool threadPool(1);

    auto result = threadPool.Execute([](){});   
    result.Wait();

    ASSERT_TRUE(result.IsReady());
    ASSERT_NO_THROW(result.Get());
}

TEST_F(StaticThreadPoolTestFixture, Execute_TaskEnqueued_Success)
{
    azul::async::StaticThreadPool executor(1);

    auto result = executor.Execute([](){ return 42; });

    ASSERT_EQ(42, result.Get());
}

TEST_F(StaticThreadPoolTestFixture, Execute_TaskThrowsException_ExceptionForwarded)
{
    azul::async::StaticThreadPool executor(1);

    auto result = executor.Execute([](){ throw std::invalid_argument(""); });
    ASSERT_THROW(result.Get(), std::invalid_argument);
}

TEST_F(StaticThreadPoolTestFixture, Execute_MultipleTasksAndThreads_Success)
{
    azul::async::StaticThreadPool executor(4);

    std::vector<azul::async::Future<int>> results;

    const int tasksToExecute = 100;

    for (int i = 0; i < tasksToExecute; ++i)
    {
        auto result = executor.Execute([i](){ return i; });
        results.emplace_back(std::move(result));
    }

    for (int i = 0; i < tasksToExecute; ++i)
    {
        ASSERT_EQ(i, results[i].Get());
    }
}

TEST_F(StaticThreadPoolTestFixture, Execute_OneDependency_ExecutedInOrder)
{
    azul::async::StaticThreadPool executor(2);

    const auto firstTaskId = 1;
    const auto secondTaskId = 2;

    std::vector<int> ids;

    auto action1 = [&]() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); ids.push_back(firstTaskId); };
    auto action2 = [&]() { ids.push_back(secondTaskId); };

    auto future1 = executor.Execute(action1);
    auto future2 = executor.Execute(action2, future1);

    ASSERT_NO_THROW(future1.Get());
    ASSERT_NO_THROW(future2.Get());

    ASSERT_EQ(firstTaskId, ids[0]);
    ASSERT_EQ(secondTaskId, ids[1]);
}

TEST_F(StaticThreadPoolTestFixture, Execute_DependencyThrowingException_FollowingTaskStillExecuted)
{
    azul::async::StaticThreadPool executor(2);

    auto action1 = [&]() { throw std::runtime_error(""); };
    auto action2 = [&]() { };

    auto future1 = executor.Execute(action1);
    auto future2 = executor.Execute(action2, future1);

    ASSERT_THROW(future1.Get(), std::runtime_error);
    ASSERT_NO_THROW(future2.Get());
}

TEST_F(StaticThreadPoolTestFixture, Execute_MultipleDependencies_ValidExecutionOrder)
{
    std::vector<int> executedTasks;
    std::mutex mutex;
    auto log_task_id = [&mutex, &executedTasks](const int id) mutable {
        std::lock_guard<std::mutex> lock(mutex);
        executedTasks.push_back(id);
    };

    azul::async::StaticThreadPool executor(2);

    // dependency graph:
    // task1   <---- task3  <------ task4
    // task2   <-----------------|

    auto future1 = executor.Execute(std::bind(log_task_id, 1));
    auto future2 = executor.Execute(std::bind(log_task_id, 2));
    auto future3 = executor.Execute(std::bind(log_task_id, 3), future1);
    auto future4 = executor.Execute(std::bind(log_task_id, 4), future3, future2);

    ASSERT_NO_THROW(future4.Get());

    ASSERT_EQ(3, executedTasks[2]);
    ASSERT_EQ(4, executedTasks[3]);
}

TEST_F(StaticThreadPoolTestFixture, Execute_MultipleTasksNoDependencies_AllThreadsOccupied)
{
    std::vector<std::thread::id> utlizedThreadIds;
    std::mutex mutex;
    auto logThreadId = [&mutex, &utlizedThreadIds]() mutable {
        std::lock_guard<std::mutex> lock(mutex);
        utlizedThreadIds.push_back(std::this_thread::get_id());
    };

    azul::async::StaticThreadPool executor(4);
    auto action = [&logThreadId](){
        logThreadId();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };

    std::vector<azul::async::Future<void>> futures;
    for (std::size_t i = 0; i < executor.ThreadCount(); ++i)
    {
        futures.emplace_back(executor.Execute(action));
    }
    std::for_each(futures.begin(), futures.end(), [](auto f){ f.Wait(); });

    ASSERT_EQ(utlizedThreadIds.size(), executor.ThreadCount());
}

