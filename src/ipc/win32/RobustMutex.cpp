#include "azul/ipc/sync/RobustMutex.hpp"

#include <azul/utils/Disposer.hpp>
#include <thread>
#include <Windows.h>
#include <stdexcept>

namespace
{
    class RobustMutex final
    {
    private:
        ::azul::utils::Disposer _disposer;
        void* _mutex = nullptr;
        std::thread::id _currentLockingThread;

    public:
        explicit RobustMutex(std::string const& name, bool const)
        {
            _mutex = CreateMutexA(nullptr, false, name.c_str());

            if (_mutex == nullptr)
            {
                throw std::runtime_error("CreateMutexA failed, error: " + std::to_string(GetLastError()));
            }

            _disposer.Set([=]() { CloseHandle(_mutex); });
        }

        void lock()
        {
            const auto threadId = std::this_thread::get_id();
            if (_currentLockingThread == threadId)
            {
                throw std::runtime_error("Attempted to recursively lock a mutex.");
            }

            const auto result = WaitForSingleObject(_mutex, INFINITE);
            if (result == WAIT_ABANDONED || result == WAIT_OBJECT_0)
            {
                _currentLockingThread = threadId;
                return;
            }

            if (result == WAIT_FAILED)
            {
                throw std::runtime_error("WaitForSingleObject failed, error: " + std::to_string(GetLastError()));
            }
        }

        bool try_lock()
        {
            const auto thread_id = std::this_thread::get_id();
            if (_currentLockingThread == thread_id)
            {
                throw std::runtime_error("Attempted to recursively lock a mutex.");
            }

            const auto result = WaitForSingleObject(_mutex, 0);
            if (result == WAIT_FAILED)
            {
                throw std::runtime_error("WaitForSingleObject failed, error: " + std::to_string(GetLastError()));
            }

            if ((result == WAIT_OBJECT_0) || (result == WAIT_ABANDONED))
            {
                _currentLockingThread = thread_id;
                return true;
            }

            return false;
        }

        void unlock()
        {
            if (!ReleaseMutex(_mutex))
            {
                throw std::runtime_error("ReleaseMutex failed, error: " + std::to_string(GetLastError()));
            }

            _currentLockingThread = std::thread::id();
        }
    };
}

// -----------------------------------------------------------------------------------------------------

azul::ipc::sync::RobustMutex::RobustMutex(std::string const& name, bool const _isOwner)
    : _impl(std::make_unique<::RobustMutex>(name, _isOwner))
{
}

azul::ipc::sync::RobustMutex::RobustMutex() : _impl(nullptr)
{
}

azul::ipc::sync::RobustMutex::~RobustMutex()
{
}

void azul::ipc::sync::RobustMutex::lock()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::RobustMutex *const instance = reinterpret_cast<::RobustMutex*>(_impl.get());
    instance->lock();
}

bool azul::ipc::sync::RobustMutex::try_lock()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::RobustMutex *const instance = reinterpret_cast<::RobustMutex*>(_impl.get());
    return instance->try_lock();
}

void azul::ipc::sync::RobustMutex::unlock()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::RobustMutex *const instance = reinterpret_cast<::RobustMutex*>(_impl.get());
    instance->unlock();
}
