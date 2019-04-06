#include "impulso/ipc/sync/robust_mutex.hpp"

#include <impulso/utils/disposer.hpp>
#include <thread>
#include <Windows.h>
#include <stdexcept>

namespace
{
    class robust_mutex final
    {
    private:
        ::impulso::utils::disposer disposer_;
        void* mutex_ = nullptr;
        std::thread::id current_locking_thread_;

    public:
        explicit robust_mutex(std::string const& name, bool const)
        {
            mutex_ = CreateMutexA(nullptr, false, name.c_str());

            if (mutex_ == nullptr)
            {
                throw std::runtime_error("CreateMutexA failed, error: " + std::to_string(GetLastError()));
            }

            disposer_.set([=]() { CloseHandle(mutex_); });
        }

        void lock()
        {
            const auto thread_id = std::this_thread::get_id();
            if (current_locking_thread_ == thread_id)
            {
                throw std::runtime_error("Attempted to recursively lock a mutex.");
            }

            const auto result = WaitForSingleObject(mutex_, INFINITE);
            if (result == WAIT_ABANDONED || result == WAIT_OBJECT_0)
            {
                current_locking_thread_ = thread_id;
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
            if (current_locking_thread_ == thread_id)
            {
                throw std::runtime_error("Attempted to recursively lock a mutex.");
            }

            const auto result = WaitForSingleObject(mutex_, 0);
            if (result == WAIT_FAILED)
            {
                throw std::runtime_error("WaitForSingleObject failed, error: " + std::to_string(GetLastError()));
            }

            if ((result == WAIT_OBJECT_0) || (result == WAIT_ABANDONED))
            {
                current_locking_thread_ = thread_id;
                return true;
            }

            return false;
        }

        void unlock()
        {
            if (!ReleaseMutex(mutex_))
            {
                throw std::runtime_error("ReleaseMutex failed, error: " + std::to_string(GetLastError()));
            }

            current_locking_thread_ = std::thread::id();
        }
    };
}

// -----------------------------------------------------------------------------------------------------

impulso::ipc::sync::robust_mutex::robust_mutex(std::string const& name, bool const is_owner)
    : impl_(std::make_unique<::robust_mutex>(name, is_owner))
{
}

impulso::ipc::sync::robust_mutex::robust_mutex() : impl_(nullptr)
{
}

impulso::ipc::sync::robust_mutex::~robust_mutex()
{
}

void impulso::ipc::sync::robust_mutex::lock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    instance->lock();
}

bool impulso::ipc::sync::robust_mutex::try_lock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    return instance->try_lock();
}

void impulso::ipc::sync::robust_mutex::unlock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    instance->unlock();
}
