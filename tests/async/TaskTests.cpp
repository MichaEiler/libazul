#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <azul/async/Task.hpp>
#include <stdexcept>
#include <thread>

class TaskTestFixture : public testing::Test
{
};

TEST_F(TaskTestFixture, WrapDependencies_TwoInputFutures_ResultSet)
{
    azul::async::Promise<int> promise;
    auto future = promise.GetFuture();

    azul::async::Promise<bool> promise2;
    auto future2 = promise2.GetFuture();

    auto finalFuture = azul::async::WhenAll(future, future2);

    ASSERT_FALSE(future.IsReady());
    ASSERT_FALSE(future2.IsReady());
    ASSERT_FALSE(finalFuture.IsReady());

    promise.SetValue(42);

    ASSERT_TRUE(future.IsReady());
    ASSERT_FALSE(future2.IsReady());
    ASSERT_FALSE(finalFuture.IsReady());

    promise2.SetValue(true);

    ASSERT_TRUE(future.IsReady());
    ASSERT_TRUE(future2.IsReady());
    ASSERT_TRUE(finalFuture.IsReady());
}

TEST_F(TaskTestFixture, WrapDependencies_ZeroInputFutures_ResultImmediatelySet)
{
    auto future = azul::async::WhenAll();
    ASSERT_TRUE(future.IsReady());
}

TEST_F(TaskTestFixture, ExecuteTask_TaskThrowsException_exceptionCorrectlyForwarded)
{
    auto taskAction = []() { throw std::invalid_argument(""); };
    azul::async::Task<void> task(taskAction);
    auto future = task.GetFuture();

    task();

    ASSERT_THROW(future.Get(), std::invalid_argument);
}

TEST_F(TaskTestFixture, ExecuteTask_TaskDestroyedResultReady_NoExceptionThrown)
{
    azul::async::Future<void> future;
    
    {
        auto taskAction = [](){};
        azul::async::Task<void> task(taskAction);
        future = task.GetFuture();

        task();
    }

    ASSERT_NO_THROW(future.Get());
}

TEST_F(TaskTestFixture, ExecuteTask_TaskDestroyedExceptionCached_ExceptionCorrectlyForwarded)
{
    azul::async::Future<void> future;

    {
        auto taskAction = [](){ throw std::invalid_argument(""); };
        azul::async::Task<void> task(taskAction);
        future = task.GetFuture();

        task();
    }

    ASSERT_THROW(future.Get(), std::invalid_argument);
}

TEST_F(TaskTestFixture, ExecuteTask_TaskProvidesReturnValue_FutureReturnsResult)
{
    auto taskAction = []() -> int { return 1337; };
    azul::async::Task<int> task(taskAction);
    auto future = task.GetFuture();
    task();

    ASSERT_EQ(1337, future.Get());
}

TEST_F(TaskTestFixture, GetValue_TaskInOtherThread_BlocksUntilTaskProcessed)
{
    bool resultAvailable = false;

    azul::async::Task<void> task([](){});
    auto future = task.GetFuture();

    std::thread other_thread([&](){
        future.Get();
        resultAvailable = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_FALSE(resultAvailable);

    task();
    other_thread.join();
    ASSERT_TRUE(resultAvailable);   
}

TEST_F(TaskTestFixture, IsReady_NoDependency_ReturnsTrue)
{
    auto task = azul::async::Task<void>([](){});
    ASSERT_TRUE(task.IsReady());
}

TEST_F(TaskTestFixture, IsReady_DependencyNotReady_ReturnsFalse)
{
    azul::async::Promise<void> promise;
    auto future = promise.GetFuture();
    auto task = azul::async::Task<void>([](){}, future);
    ASSERT_FALSE(task.IsReady());
}

TEST_F(TaskTestFixture, IsReady_DependencyReady_ReturnsTrue)
{
    azul::async::Promise<void> promise;
    auto future = promise.GetFuture();
    promise.SetValue();
    auto task = azul::async::Task<void>([](){}, future);
    ASSERT_TRUE(task.IsReady());
}

TEST_F(TaskTestFixture, SetValue_CalledTwice_ThrowsFutureError)
{
    azul::async::Promise<int> promise;
    promise.SetValue(42);
    ASSERT_THROW(promise.SetValue(42), azul::async::FutureError);
}

TEST_F(TaskTestFixture, SetValue_CalledTwiceOnVoidFuture_ContinuationOnlyCalledOnce)
{
    int executionCount = 0;
    auto continuation = [&](auto) { executionCount++; };

    azul::async::Promise<void> promise;
    auto future = promise.GetFuture();
    future.Then(continuation);

    promise.SetValue();
    promise.SetValue();

    ASSERT_EQ(1, executionCount);
}

TEST_F(TaskTestFixture, SetValue_CalledTwiceOnVoidFuture_NoExceptionThrown)
{
    azul::async::Promise<void> promise;
    promise.SetValue();
    ASSERT_NO_THROW(promise.SetValue());
}

TEST_F(TaskTestFixture, OrOperator_FirstArgumentReady_ResultReady)
{
    azul::async::Promise<void> promiseA;
    azul::async::Promise<void> promiseB;
    auto futureA = promiseA.GetFuture();
    auto futureB = promiseB.GetFuture();

    ASSERT_FALSE(futureA.IsReady());
    ASSERT_FALSE(futureB.IsReady());

    using namespace azul::async;
    auto resultFuture = futureA || futureB;
    ASSERT_FALSE(resultFuture.IsReady());

    promiseA.SetValue();
    ASSERT_TRUE(resultFuture.IsReady());
}

TEST_F(TaskTestFixture, OrOperator_SecondArgumentReady_ResultReady)
{
    azul::async::Promise<void> promiseA;
    azul::async::Promise<void> promiseB;
    auto futureA = promiseA.GetFuture();
    auto futureB = promiseB.GetFuture();

    ASSERT_FALSE(futureA.IsReady());
    ASSERT_FALSE(futureB.IsReady());

    using namespace azul::async;
    auto resultFuture = futureA || futureB;
    ASSERT_FALSE(resultFuture.IsReady());

    promiseB.SetValue();
    ASSERT_TRUE(resultFuture.IsReady());
}

TEST_F(TaskTestFixture, AndOperator_BothArgumentsSetToReady_ResultReady)
{
    azul::async::Promise<void> promiseA;
    azul::async::Promise<void> promiseB;
    auto futureA = promiseA.GetFuture();
    auto futureB = promiseB.GetFuture();

    ASSERT_FALSE(futureA.IsReady());
    ASSERT_FALSE(futureB.IsReady());

    using namespace azul::async;
    auto resultFuture = futureA && futureB;
    ASSERT_FALSE(resultFuture.IsReady());

    promiseA.SetValue();
    ASSERT_FALSE(resultFuture.IsReady());
    promiseB.SetValue();
    ASSERT_TRUE(resultFuture.IsReady());
}

TEST_F(TaskTestFixture, Operators_BooleanExpression1_ResultCorrect)
{
    azul::async::Promise<void> promiseA;
    azul::async::Promise<void> promiseB;
    azul::async::Promise<void> promiseC;
    auto futureA = promiseA.GetFuture();
    auto futureB = promiseB.GetFuture();
    auto futureC = promiseC.GetFuture();

    ASSERT_FALSE(futureA.IsReady());
    ASSERT_FALSE(futureB.IsReady());
    ASSERT_FALSE(futureC.IsReady());

    using namespace azul::async;
    auto resultFuture = (futureA && futureB) || futureC;

    ASSERT_FALSE(resultFuture.IsReady());

    promiseA.SetValue();
    ASSERT_FALSE(resultFuture.IsReady());
    promiseB.SetValue();
    ASSERT_TRUE(resultFuture.IsReady());
}

TEST_F(TaskTestFixture, Operators_BooleanExpression2_ResultCorrect)
{
    azul::async::Promise<void> promiseA;
    azul::async::Promise<void> promiseB;
    azul::async::Promise<void> promiseC;
    auto futureA = promiseA.GetFuture();
    auto futureB = promiseB.GetFuture();
    auto futureC = promiseC.GetFuture();

    ASSERT_FALSE(futureA.IsReady());
    ASSERT_FALSE(futureB.IsReady());
    ASSERT_FALSE(futureC.IsReady());

    using namespace azul::async;
    auto resultFuture = (futureA && futureB) || futureC;
    
    ASSERT_FALSE(resultFuture.IsReady());

    promiseC.SetValue();
    ASSERT_TRUE(resultFuture.IsReady());
}