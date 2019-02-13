#include <gmock/gmock.h>
#include <future>
#include <stdexcept>
#include <thread>

// these tests make sure that the std::future implementation/specification behaves as expected

class stdfuture_fixture : public testing::Test
{
};

TEST_F(stdfuture_fixture, get_promiseDestroyedBeforeResultProvided_throwsException)
{
    std::future<int> future_;
    {
        std::promise<int> promise_;
        future_ = std::move(promise_.get_future());
    }

    ASSERT_THROW(future_.get(), std::future_error);
}

TEST_F(stdfuture_fixture, get_promiseDestroyedAfterResultProvided_futureReturnsStoredValue)
{
    std::future<int> future_;
    {
        std::promise<int> promise_;
        future_ = std::move(promise_.get_future());
        promise_.set_value(1337);
    }

    ASSERT_EQ(1337, future_.get());
}

TEST_F(stdfuture_fixture, get_otherThreadThrowsException_exceptionRethrownWithCorrectType)
{
    std::promise<int> promise_;
    std::future<int> future_ = std::move(promise_.get_future());

    {
        std::thread other_thread([moved_promise=std::move(promise_)]() mutable {
            try
            {
                throw std::invalid_argument("content unimportant for test");
            }
            catch(...)
            {
                moved_promise.set_exception(std::current_exception());
            }
        });

        other_thread.join();
    }

    ASSERT_THROW(future_.get(), std::invalid_argument);
}
