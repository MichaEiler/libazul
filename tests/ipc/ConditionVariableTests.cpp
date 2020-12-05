#include <atomic>
#include <chrono>
#include <gmock/gmock.h>
#include <azul/ipc/sync/ConditionVariable.hpp>
#include <azul/ipc/sync/RobustMutex.hpp>
#include <memory>
#include <thread>

class ConditionVariableTestFixture : public testing::Test
{
};

TEST_F(ConditionVariableTestFixture, Signal_SingleWaitingThreadWithoutTimeout_ThreadNotified)
{
    auto mutex = azul::ipc::sync::RobustMutex("76bda1f5", true);
    auto cond = azul::ipc::sync::ConditionVariable("57a16bdf", true);

    std::atomic<bool> notification_received(false);

    std::thread other_thread([&]() {
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(mutex);
        cond.Wait(lock);
        notification_received.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_FALSE(notification_received.load());

    while (!notification_received.load())
    {
        cond.NotifyOne();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    other_thread.join();

    ASSERT_TRUE(notification_received);
}

TEST_F(ConditionVariableTestFixture, Signal_SingleWaitingThreadWithTimeout_DoesNotTimeout)
{
    auto mutex = azul::ipc::sync::RobustMutex("7a16bdf5", true);
    auto cond = azul::ipc::sync::ConditionVariable("df57a16b", true);

    bool timeout = true;

    std::thread other_thread([&mutex, &cond, &timeout]() {
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(mutex);
        timeout = cond.WaitFor(lock, std::chrono::milliseconds(1000)) == std::cv_status::timeout;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    cond.NotifyOne();
    other_thread.join();

    ASSERT_FALSE(timeout);
}

TEST_F(ConditionVariableTestFixture, Signal_TwoWaitingThreadsWithTimeouts_OneThreadNotified)
{
    auto mutex = azul::ipc::sync::RobustMutex("76bda1f5", true);
    auto cond = azul::ipc::sync::ConditionVariable("57a16bdf", true);

    std::atomic<int> timeouts_occured(0);
    std::atomic<int> notifications_received(0);

    const auto task = [&mutex, &timeouts_occured, &notifications_received, &cond]() {
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(mutex);
        bool notified = cond.WaitFor(lock, std::chrono::milliseconds(500)) != std::cv_status::timeout;

        if (notified)
        {
            notifications_received++;
        }
        else
        {
            timeouts_occured++;
        }
    };

    std::thread waiting_thread1(task);
    std::thread waiting_thread2(task);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(0, notifications_received.load());
    ASSERT_EQ(0, timeouts_occured.load());

    cond.NotifyOne();

    waiting_thread1.join();
    waiting_thread2.join();

    ASSERT_EQ(1, notifications_received);
    ASSERT_EQ(1, timeouts_occured);
}

TEST_F(ConditionVariableTestFixture, SignalOne_NoThreadWaiting_NoExceptionThrown)
{
    auto mutex = azul::ipc::sync::RobustMutex("76bda222", true);
    auto cond = azul::ipc::sync::ConditionVariable("57a43322", true);

    ASSERT_NO_THROW(cond.NotifyOne());
}

TEST_F(ConditionVariableTestFixture, Broadcast_TwoWaitingThreadsWithTimeouts_BothThreadsNotified)
{
    auto mutex = azul::ipc::sync::RobustMutex("76bda1f5", true);
    auto cond = azul::ipc::sync::ConditionVariable("57a16bdf", true);

    std::atomic<int> timeouts_occured(0);
    std::atomic<int> notifications_received(0);

    const auto task = [&mutex, &timeouts_occured, &notifications_received, &cond]() {
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(mutex);
        bool notified = cond.WaitFor(lock, std::chrono::milliseconds(500)) != std::cv_status::timeout;

        if (notified)
        {
            notifications_received++;
        }
        else
        {
            timeouts_occured++;
        }
    };

    std::thread waiting_thread1(task);
    std::thread waiting_thread2(task);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(0, notifications_received.load());
    ASSERT_EQ(0, timeouts_occured.load());

    cond.NotifyAll();

    waiting_thread1.join();
    waiting_thread2.join();

    ASSERT_EQ(2, notifications_received.load());
    ASSERT_EQ(0, timeouts_occured.load());
}

