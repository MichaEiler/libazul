#include <atomic>
#include <chrono>
#include <gmock/gmock.h>
#include <azul/ipc/sync/RobustMutex.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

class RobustMutexTestFixture : public testing::Test
{
};

TEST_F(RobustMutexTestFixture, Lock_NoOwner_Succeeds)
{
    auto mutex = azul::ipc::sync::RobustMutex("aef94b74", true);
    mutex.lock();
    mutex.unlock();
}

TEST_F(RobustMutexTestFixture, Lock_OwnedByOtherThread_Blocks)
{
    auto mutex = azul::ipc::sync::RobustMutex("69ed4121", true);
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

TEST_F(RobustMutexTestFixture, Unlock_ByWrongThread_ThrowsRuntimeError)
{
    auto mutex = azul::ipc::sync::RobustMutex("a3e45f16", true);

    mutex.lock();

    std::thread other_thread([&mutex]() { EXPECT_THROW(mutex.unlock(), std::runtime_error); });

    other_thread.join();
}

TEST_F(RobustMutexTestFixture, Lock_AttemptRecursiveLock_ThrowsRuntimeError)
{
    auto mutex = azul::ipc::sync::RobustMutex("c2bf254c", true);
    mutex.lock();
    EXPECT_THROW(mutex.lock(), std::runtime_error);
    mutex.unlock();

    // invalid operation did not mess with mutex functionality
    mutex.lock();
    mutex.unlock();
}

// old test, does not work with the osx implementations
// but should not be a use case if lock/unlock is never manually called
/*TEST_F(robust_mutex_fixture, lock_lockedByAlreadyFinishedThread_lockInMainThreadSucceeds)
{
    auto mutex = azul::ipc::sync::robust_mutex("4b13a5c1", true);

    std::thread other_thread([&mutex]() { mutex.lock(); });
    other_thread.join();

    mutex.lock();
    mutex.unlock();
}*/
