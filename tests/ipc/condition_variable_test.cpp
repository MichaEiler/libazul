#include <atomic>
#include <chrono>
#include <gmock/gmock.h>
#include <azul/ipc/sync/condition_variable.hpp>
#include <azul/ipc/sync/robust_mutex.hpp>
#include <memory>
#include <thread>

class condition_variable_fixture : public testing::Test
{
};

TEST_F(condition_variable_fixture, signal_singleWaitingThreadWithoutTimeout_threadNotified)
{
    auto mutex = azul::ipc::sync::robust_mutex("76bda1f5", true);
    auto cond = azul::ipc::sync::condition_variable("57a16bdf", true);

    std::atomic<bool> notification_received(false);

    std::thread other_thread([&]() {
        std::unique_lock<azul::ipc::sync::robust_mutex> lock(mutex);
        cond.wait(lock);
        notification_received.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_FALSE(notification_received.load());

    while (!notification_received.load())
    {
        cond.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    other_thread.join();

    ASSERT_TRUE(notification_received);
}

TEST_F(condition_variable_fixture, signal_singleWaitingThreadWithTimeout_doesNotTimeout)
{
    auto mutex = azul::ipc::sync::robust_mutex("7a16bdf5", true);
    auto cond = azul::ipc::sync::condition_variable("df57a16b", true);

    bool timeout = true;

    std::thread other_thread([&mutex, &cond, &timeout]() {
        std::unique_lock<azul::ipc::sync::robust_mutex> lock(mutex);
        timeout = cond.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::timeout;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    cond.notify_one();
    other_thread.join();

    ASSERT_FALSE(timeout);
}

TEST_F(condition_variable_fixture, signal_twoWaitingThreadsWithTimeouts_oneThreadNotified)
{
    auto mutex = azul::ipc::sync::robust_mutex("76bda1f5", true);
    auto cond = azul::ipc::sync::condition_variable("57a16bdf", true);

    std::atomic<int> timeouts_occured(0);
    std::atomic<int> notifications_received(0);

    const auto task = [&mutex, &timeouts_occured, &notifications_received, &cond]() {
        std::unique_lock<azul::ipc::sync::robust_mutex> lock(mutex);
        bool notified = cond.wait_for(lock, std::chrono::milliseconds(500)) != std::cv_status::timeout;

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

    cond.notify_one();

    waiting_thread1.join();
    waiting_thread2.join();

    ASSERT_EQ(1, notifications_received);
    ASSERT_EQ(1, timeouts_occured);
}

TEST_F(condition_variable_fixture, broadcast_twoWaitingThreadsWithTimeouts_bothThreadsNotified)
{
    auto mutex = azul::ipc::sync::robust_mutex("76bda1f5", true);
    auto cond = azul::ipc::sync::condition_variable("57a16bdf", true);

    std::atomic<int> timeouts_occured(0);
    std::atomic<int> notifications_received(0);

    const auto task = [&mutex, &timeouts_occured, &notifications_received, &cond]() {
        std::unique_lock<azul::ipc::sync::robust_mutex> lock(mutex);
        bool notified = cond.wait_for(lock, std::chrono::milliseconds(500)) != std::cv_status::timeout;

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

    cond.notify_all();

    waiting_thread1.join();
    waiting_thread2.join();

    ASSERT_EQ(2, notifications_received.load());
    ASSERT_EQ(0, timeouts_occured.load());
}

