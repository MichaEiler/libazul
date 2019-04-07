#include <atomic>
#include <chrono>
#include <gmock/gmock.h>
#include <azul/ipc/sync/robust_mutex.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

class robust_mutex_fixture : public testing::Test
{
};

TEST_F(robust_mutex_fixture, lock_noowner_succeeds)
{
    auto mutex = azul::ipc::sync::robust_mutex("aef94b74", true);
    mutex.lock();
    mutex.unlock();
}

TEST_F(robust_mutex_fixture, lock_ownedByOtherThread_blocks)
{
    auto mutex = azul::ipc::sync::robust_mutex("69ed4121", true);
    bool lock_acquired = false;
    std::atomic<bool> continue_waiting(true);

    std::shared_ptr<std::thread> other_thread;

    std::thread thread([&]() {
        mutex.lock();

        other_thread = std::make_shared<std::thread>([&](){
             
            mutex.lock();
            lock_acquired = true;
            mutex.unlock();
        });

        while (continue_waiting.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        mutex.unlock();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_FALSE(lock_acquired);

    continue_waiting.store(false);

    thread.join();
    other_thread->join();

    ASSERT_TRUE(lock_acquired);
}

TEST_F(robust_mutex_fixture, unlock_byWrongThread_throwsRuntimeError)
{
    auto mutex = azul::ipc::sync::robust_mutex("a3e45f16", true);

    mutex.lock();

    std::thread other_thread([&mutex]() { EXPECT_THROW(mutex.unlock(), std::runtime_error); });

    other_thread.join();
}

TEST_F(robust_mutex_fixture, lock_attemptRecursiveLock_throwsRuntimeError)
{
    auto mutex = azul::ipc::sync::robust_mutex("c2bf254c", true);
    mutex.lock();
    EXPECT_THROW(mutex.lock(), std::runtime_error);
    mutex.unlock();

    // invalid operation did not mess with mutex functionality
    mutex.lock();
    mutex.unlock();
}

/*TEST_F(robust_mutex_fixture, lock_lockedByAlreadyFinishedThread_lockInMainThreadSucceeds)
{
    auto mutex = azul::ipc::sync::robust_mutex("4b13a5c1", true);

    std::thread other_thread([&mutex]() { mutex.lock(); });
    other_thread.join();

    mutex.lock();
    mutex.unlock();
}*/
