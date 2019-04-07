#include <chrono>
#include <cstdlib>
#include <gmock/gmock.h>
#include <iostream>
#include <azul/ipc/sync/condition_variable.hpp>
#include <azul/ipc/sync/robust_mutex.hpp>
#include <mutex>
#include <thread>

class ProcessTestFixture : public testing::Test
{
};

#include <iostream>
int ConsumerProcessMain()
{
    const auto WaitAndCrash = []()
    {
        // mutex under test, this is the mutex we want to lock in the unit test after this process has crashed
        azul::ipc::sync::RobustMutex recoverable_mutex("435ba46e", false);
        std::unique_lock<azul::ipc::sync::RobustMutex> never_released_lock(recoverable_mutex);

        // waiting for a notification from the unit test so that we can crash this process
        azul::ipc::sync::RobustMutex inform_client_mutex("7f2b191f", false);
        azul::ipc::sync::ConditionVariable inform_client_cond("7f2b191f", false);

        std::unique_lock<azul::ipc::sync::RobustMutex> lock(inform_client_mutex);
        inform_client_cond.Wait(lock);

        // exit without releasing mutexes
        exit(0);
    };

    std::thread waiting_thread([&WaitAndCrash]() { WaitAndCrash(); });

    // wait long enough for other thread to create all objects and wait on the inform_client_cond
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // tell unit test that the test utility is running and intialized
    azul::ipc::sync::ConditionVariable inform_server_cond("562dcf7d", false);
    inform_server_cond.NotifyOne();

    // just block here and wait for process to crash
    waiting_thread.join();

    return 0;
}

#ifndef _WIN32

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static void SpawnProcess(const std::function<void ()> &func)
{
    const auto pid = fork();
    if (pid == 0)
    {
        func();
        exit(0);
    }
}

TEST_F(ProcessTestFixture, lockMutex_mutexAbandoned_succeeds)
{

    // the mutex which will be abandoned by another process
    azul::ipc::sync::RobustMutex recoverable_mutex("435ba46e", true);

    // cond-var which we use to tell the process we want to crash it
    azul::ipc::sync::RobustMutex inform_client_mutex("7f2b191f", true);
    azul::ipc::sync::ConditionVariable inform_client_cond("7f2b191f", true);

    // cond-var on which we wait for the client to tell us that it is ready
    azul::ipc::sync::RobustMutex inform_server_mutex("562dcf7d", true);
    azul::ipc::sync::ConditionVariable inform_server_cond("562dcf7d", true);

    std::thread wait_for_client_initialization([&]() {
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(inform_server_mutex);
        inform_server_cond.Wait(lock);
    });

    std::cout << "About to run additional process." << std::endl;
    SpawnProcess(ConsumerProcessMain);

    wait_for_client_initialization.join();

    std::cout << "Client up and running, telling it to crash itself." << std::endl;
    inform_client_cond.NotifyOne();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    {
        std::cout << "Attempting to lock abandoned mutex." << std::endl;
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(recoverable_mutex);
    }

    {
        std::cout << "Attempting to aquire normal lock on the now non-abandoned mutex." << std::endl;
        std::unique_lock<azul::ipc::sync::RobustMutex> lock(recoverable_mutex);
    }
}
#endif
