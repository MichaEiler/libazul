#include <gmock/gmock.h>
#include <azul/async/Future.hpp>
#include <stdexcept>
#include <thread>

class FutureTestFixture : public testing::Test
{
};

TEST_F(FutureTestFixture, Get_PromiseBroken_ThrowsFutureError)
{
    azul::async::Future<int> future;
    {
        azul::async::Promise<int> promise;
        future = promise.GetFuture();
    }

    ASSERT_THROW(future.Get(), azul::async::FutureError);
}

TEST_F(FutureTestFixture, Get_ResultSet_Returns)
{
    const int expectedResult = 42;

    azul::async::Future<int> future;
    {
        azul::async::Promise<int> promise;
        future = promise.GetFuture();
        promise.SetValue(expectedResult);
    }

    ASSERT_EQ(expectedResult, future.Get());
}

TEST_F(FutureTestFixture, Get_StoresException_RethrowsException)
{
    azul::async::Future<int> future;
    {
        azul::async::Promise<int> promise;
        future = promise.GetFuture();

        try
        {
            throw std::runtime_error("Some random exception.");
        } catch(const std::runtime_error&)
        {
            promise.SetException(std::current_exception());
        }
    }

    ASSERT_THROW(future.Get(), std::runtime_error);
}


TEST_F(FutureTestFixture, Get_StoresException_CanRethrowMultipleTimes)
{
    azul::async::Future<int> future;
    {
        azul::async::Promise<int> promise;
        future = promise.GetFuture();

        try
        {
            throw std::runtime_error("Some random exception.");
        } catch(const std::runtime_error&)
        {
            promise.SetException(std::current_exception());
        }
    }

    ASSERT_THROW(future.Get(), std::runtime_error);
    ASSERT_THROW(future.Get(), std::runtime_error);
    ASSERT_THROW(future.Get(), std::runtime_error);
}

TEST_F(FutureTestFixture, Get_FutureWithVoidTemplateParam_ResultTypeIsVoid)
{
    azul::async::Promise<void> promise;
    auto future = promise.GetFuture();
    promise.SetValue();

    ASSERT_NO_THROW(future.Get());
    ASSERT_TRUE(std::is_void_v<decltype(future.Get())>);
}

TEST_F(FutureTestFixture, IsReady_UnmodifiedPromise_ReturnsFalse)
{
    azul::async::Promise<int> promise;
    auto future = promise.GetFuture();
    ASSERT_FALSE(future.IsReady());
}

TEST_F(FutureTestFixture, IsReady_ResultSet_ReturnsTrue)
{
    azul::async::Promise<int> promise;
    auto future = promise.GetFuture();
    promise.SetValue(42);
    ASSERT_TRUE(future.IsReady());
}

TEST_F(FutureTestFixture, IsReady_StoresException_ReturnsTrue)
{
    azul::async::Promise<int> promise;
    auto future = promise.GetFuture();

    try
    {
        throw std::runtime_error("");
    }
    catch(const std::runtime_error&)
    {
        promise.SetException(std::current_exception());
    }
    
    ASSERT_TRUE(future.IsReady());
}

TEST_F(FutureTestFixture, IsReady_PromiseBroken_ReturnsTrue)
{
    azul::async::Future<int> future;
    {
        azul::async::Promise<int> promise;
        future = promise.GetFuture();
    }
    ASSERT_TRUE(future.IsReady());
}

TEST_F(FutureTestFixture, Wait_DelayedResult_BlocksInitially)
{
    azul::async::Promise<int> promise;
    azul::async::Future<int> future = promise.GetFuture();
    bool resultAvailable = false;

    std::thread otherThread([future, &resultAvailable](){
        future.Wait();
        resultAvailable = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(resultAvailable);

    promise.SetValue(42);
    otherThread.join();
    ASSERT_TRUE(resultAvailable);
}

TEST_F(FutureTestFixture, WaitFor_DelayedResult_BlocksInitially)
{
    azul::async::Promise<int> promise;
    azul::async::Future<int> future = promise.GetFuture();
    bool timeoutOccured = false;

    std::thread otherThread([future, &timeoutOccured](){
        timeoutOccured = !future.WaitFor(std::chrono::milliseconds(1000));
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    promise.SetValue(42);

    otherThread.join();
    ASSERT_FALSE(timeoutOccured);
}

TEST_F(FutureTestFixture, WaitFor_NoResult_TimesOut)
{
    azul::async::Promise<int> promise;
    azul::async::Future<int> future = promise.GetFuture();
    bool timeoutOccured = false;

    std::thread otherThread([future, &timeoutOccured](){
        timeoutOccured = !future.WaitFor(std::chrono::milliseconds(10));
    });

    otherThread.join();
    ASSERT_TRUE(timeoutOccured);
}

TEST_F(FutureTestFixture, Then_ContinuationReturningInt_ResultAvailable)
{
    azul::async::Promise<int> promise;
    auto future = promise.GetFuture();

    auto future2 = future.Then([](auto f){ return f.Get(); });

    const int expectedValue = 42;

    promise.SetValue(expectedValue);
    ASSERT_NO_THROW(future2.Wait());
    ASSERT_EQ(expectedValue, future2.Get());
}

TEST_F(FutureTestFixture, Then_ContinuationReturningVoid_ContinuationCalled)
{
    azul::async::Promise<int> promise;
    auto future = promise.GetFuture();

    auto future2 = future.Then([](auto){ });
    promise.SetValue(42);
    ASSERT_NO_THROW(future2.Wait());
    ASSERT_NO_THROW(future2.Get());
}

TEST_F(FutureTestFixture, Then_OriginalPromiseDestroyedEarly_ResultFutureThrowsFutureError)
{
    azul::async::Future<void> future;
    azul::async::Future<void> future2;
    {
        azul::async::Promise<void> promise;
        future = promise.GetFuture();
        future2 = future.Then([](auto){});
    }
    

    ASSERT_THROW(future2.Get(), azul::async::FutureError);
}

TEST_F(FutureTestFixture, OrOperator_FirstArgumentReady_ResultReady)
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

TEST_F(FutureTestFixture, OrOperator_SecondArgumentReady_ResultReady)
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

TEST_F(FutureTestFixture, AndOperator_BothArgumentsSetToReady_ResultReady)
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

TEST_F(FutureTestFixture, Operators_BooleanExpression1_ResultCorrect)
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

TEST_F(FutureTestFixture, Operators_BooleanExpression2_ResultCorrect)
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

