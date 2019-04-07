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
