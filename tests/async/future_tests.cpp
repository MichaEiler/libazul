#include <gmock/gmock.h>
#include <azul/async/future.hpp>
#include <stdexcept>
#include <thread>

class future_fixture : public testing::Test
{
};

TEST_F(future_fixture, get_promiseBroken_throwsFutureError)
{
    azul::async::future<int> future;
    {
        azul::async::promise<int> promise;
        future = promise.get_future();
    }

    ASSERT_THROW(future.get(), azul::async::future_error);
}

TEST_F(future_fixture, get_resultSet_returns)
{
    const int expected_result = 42;

    azul::async::future<int> future;
    {
        azul::async::promise<int> promise;
        future = promise.get_future();
        promise.set_value(expected_result);
    }

    ASSERT_EQ(expected_result, future.get());
}

TEST_F(future_fixture, get_storesException_rethrowsException)
{
    azul::async::future<int> future;
    {
        azul::async::promise<int> promise;
        future = promise.get_future();

        try
        {
            throw std::runtime_error("Some random exception.");
        } catch(const std::runtime_error&)
        {
            promise.set_exception(std::current_exception());
        }
    }

    ASSERT_THROW(future.get(), std::runtime_error);
}


TEST_F(future_fixture, get_storesException_canRethrowMultipleTimes)
{
    azul::async::future<int> future;
    {
        azul::async::promise<int> promise;
        future = promise.get_future();

        try
        {
            throw std::runtime_error("Some random exception.");
        } catch(const std::runtime_error&)
        {
            promise.set_exception(std::current_exception());
        }
    }

    ASSERT_THROW(future.get(), std::runtime_error);
    ASSERT_THROW(future.get(), std::runtime_error);
    ASSERT_THROW(future.get(), std::runtime_error);
}

TEST_F(future_fixture, get_futureWithVoidTemplateParam_resultTypeIsVoid)
{
    azul::async::promise<void> promise;
    auto future = promise.get_future();
    promise.set_value();

    ASSERT_NO_THROW(future.get());
    ASSERT_TRUE(std::is_void_v<decltype(future.get())>);
}

TEST_F(future_fixture, isReady_unmodifiedPromise_returnsFalse)
{
    azul::async::promise<int> promise;
    auto future = promise.get_future();
    ASSERT_FALSE(future.is_ready());
}

TEST_F(future_fixture, isReady_resultSet_returnsTrue)
{
    azul::async::promise<int> promise;
    auto future = promise.get_future();
    promise.set_value(42);
    ASSERT_TRUE(future.is_ready());
}

TEST_F(future_fixture, isReady_storesException_returnsTrue)
{
    azul::async::promise<int> promise;
    auto future = promise.get_future();

    try
    {
        throw std::runtime_error("");
    }
    catch(const std::runtime_error&)
    {
        promise.set_exception(std::current_exception());
    }
    
    ASSERT_TRUE(future.is_ready());
}

TEST_F(future_fixture, isReady_promiseBroken_returnsTrue)
{
    azul::async::future<int> future;
    {
        azul::async::promise<int> promise;
        future = promise.get_future();
    }
    ASSERT_TRUE(future.is_ready());
}

TEST_F(future_fixture, wait_delayedResult_blocksInitially)
{
    azul::async::promise<int> promise;
    azul::async::future<int> future = promise.get_future();
    bool result_available = false;

    std::thread otherThread([future, &result_available](){
        future.wait();
        result_available = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(result_available);

    promise.set_value(42);
    otherThread.join();
    ASSERT_TRUE(result_available);
}

TEST_F(future_fixture, waitFor_delayedResult_blocksInitially)
{
    azul::async::promise<int> promise;
    azul::async::future<int> future = promise.get_future();
    bool timeout_occured = false;

    std::thread otherThread([future, &timeout_occured](){
        timeout_occured = !future.wait_for(std::chrono::milliseconds(1000));
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    promise.set_value(42);

    otherThread.join();
    ASSERT_FALSE(timeout_occured);
}

TEST_F(future_fixture, waitFor_noResult_timesOut)
{
    azul::async::promise<int> promise;
    azul::async::future<int> future = promise.get_future();
    bool timeout_occured = false;

    std::thread otherThread([future, &timeout_occured](){
        timeout_occured = !future.wait_for(std::chrono::milliseconds(10));
    });

    otherThread.join();
    ASSERT_TRUE(timeout_occured);
}

TEST_F(future_fixture, then_continuationReturningInt_resultAvailable)
{
    azul::async::promise<int> promise;
    azul::async::future<int> future = promise.get_future();

    auto future2 = future.then([](azul::async::future<int> f){ return f.get(); });

    const int expected_value = 42;

    promise.set_value(expected_value);
    ASSERT_NO_THROW(future2.wait());
    ASSERT_EQ(expected_value, future2.get());
}

TEST_F(future_fixture, then_continuationReturningVoid_continuationCalled)
{
    azul::async::promise<int> promise;
    azul::async::future future = promise.get_future();

    auto future2 = future.then([](auto){ });
    promise.set_value(42);
    ASSERT_NO_THROW(future2.wait());
    ASSERT_NO_THROW(future2.get());
}
