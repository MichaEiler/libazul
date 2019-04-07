#include "azul/ipc/sync/ConditionVariable.hpp"

#include <Queue.hpp>
#include <azul/ipc/sync/RobustMutex.hpp>
#include <azul/ipc/SharedMemory.hpp>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <Windows.h>

namespace
{
    constexpr int _threadQueueSTORAGE_SIZE = 128 * 1024;

    class ConditionVariable final
    {
    private:
        std::unordered_map<std::thread::id, HANDLE> _semaphores;
        ::azul::ipc::SharedMemory _threadQueueMemory;
        azul::ipc::detail::Queue<std::thread::id> _threadQueue;
        ::azul::ipc::sync::RobustMutex _threadQueueMutex;
        std::string const _name;

        HANDLE GetSemaphore(std::thread::id const& _threadId)
        {
            if (_semaphores.find(_threadId) == _semaphores.end())
            {
                std::stringstream stream;
                stream << _threadId;

                const std::string semaphore_name = _name + stream.str();
                const auto semaphoreHandle = CreateSemaphoreA(nullptr, 0, 1, semaphore_name.c_str());
                if (semaphoreHandle == nullptr)
                {
                    throw std::runtime_error("CreateSemaphoreA failed, error: " + std::to_string(GetLastError()));
                }

                _semaphores[_threadId] = semaphoreHandle;
                return semaphoreHandle;
            }

            auto semaphoreHandle = _semaphores[_threadId];
            WaitForSingleObject(semaphoreHandle, 0); // reset
            return semaphoreHandle;
        }

    public:
        explicit ConditionVariable(std::string const& name, bool const isOwner)
            : _threadQueueMemory(name + std::string("_thread_queue"), _threadQueueSTORAGE_SIZE, isOwner),
              _threadQueue(_threadQueueMemory.Address(), _threadQueueSTORAGE_SIZE, isOwner),
              _threadQueueMutex(name + std::string("_cond_mutex"), isOwner),
              _name(name)
        {
        }

        void NotifyOne()
        {
            std::unique_lock<::azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);

            auto id = _threadQueue.Front();
            _threadQueue.Pop();

            ReleaseSemaphore(GetSemaphore(id), 1, nullptr);
        }

        void NotifyAll()
        {
            std::unique_lock<::azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);

            while (_threadQueue.Count() > 0)
            {
                auto id = _threadQueue.Front();
                _threadQueue.Pop();

                ReleaseSemaphore(GetSemaphore(id), 1, nullptr);
            }
        }

        void Wait(std::unique_lock<::azul::ipc::sync::RobustMutex>& mutex)
        {
            std::unique_lock<::azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);
            const auto current_thread_id = std::this_thread::get_id();
            const auto semaphore = GetSemaphore(current_thread_id);

            _threadQueue.PushBack(current_thread_id);

            mutex.unlock();
            lock.unlock();

            const auto result = WaitForSingleObject(semaphore, INFINITE);

            lock.lock();
            mutex.lock();

            if (WAIT_FAILED == result)
            {
                _threadQueue.Remove(current_thread_id);
                throw std::runtime_error("WaitForSingleObject failed, error: " + std::to_string(GetLastError()));
            }
        }

        std::cv_status WaitFor(std::unique_lock<::azul::ipc::sync::RobustMutex>& mutex, std::chrono::milliseconds const& timeout)
        {
            std::unique_lock<::azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);
            const auto current_thread_id = std::this_thread::get_id();
            const auto semaphore = GetSemaphore(current_thread_id);

            _threadQueue.PushBack(current_thread_id);

            mutex.unlock();
            lock.unlock();

            const DWORD result = WaitForSingleObject(semaphore, static_cast<DWORD>(timeout.count()));

            lock.lock();
            mutex.lock();

            if (WAIT_FAILED == result)
            {
                _threadQueue.Remove(current_thread_id);
                throw std::runtime_error("WaitForSingleObject failed, error: " + std::to_string(GetLastError()));
            }

            if (result == WAIT_OBJECT_0)
            {
                return std::cv_status::no_timeout;
            }
            return std::cv_status::timeout;
        }
    };
}

// -----------------------------------------------------------------------------------------------------

azul::ipc::sync::ConditionVariable::ConditionVariable(std::string const& name, bool const isOwner)
    : _impl(std::make_unique<::ConditionVariable>(name, isOwner))
{
}

azul::ipc::sync::ConditionVariable::ConditionVariable() : _impl(nullptr)
{
}

azul::ipc::sync::ConditionVariable::~ConditionVariable()
{
}

void azul::ipc::sync::ConditionVariable::NotifyOne()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    instance->NotifyOne();
}

void azul::ipc::sync::ConditionVariable::NotifyAll()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    instance->NotifyAll();
}

void azul::ipc::sync::ConditionVariable::Wait(std::unique_lock<ipc::sync::RobustMutex>& mutex)
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    instance->Wait(mutex);
}

std::cv_status azul::ipc::sync::ConditionVariable::WaitFor(std::unique_lock<ipc::sync::RobustMutex>& mutex, std::chrono::milliseconds const& timeout)
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    return instance->WaitFor(mutex, timeout);
}
