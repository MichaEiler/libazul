#include "impulso/ipc/sync/condition_variable.hpp"

#include <queue.hpp>
#include <impulso/ipc/sync/robust_mutex.hpp>
#include <impulso/ipc/shared_memory.hpp>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <Windows.h>

namespace
{
    constexpr int THREAD_QUEUE_STORAGE_SIZE = 128 * 1024;

    class condition_variable final
    {
    private:
        std::unordered_map<std::thread::id, HANDLE> semaphores_;
        ::impulso::ipc::shared_memory thread_queue_memory_;
        impulso::ipc::detail::queue<std::thread::id> thread_queue_;
        ::impulso::ipc::sync::robust_mutex thread_queue_mutex_;
        std::string const name_;

        HANDLE get_semaphore(std::thread::id const& thread_id)
        {
            if (semaphores_.find(thread_id) == semaphores_.end())
            {
                std::stringstream stream;
                stream << thread_id;

                const std::string semaphore_name = name_ + stream.str();
                const auto semaphore_handle = CreateSemaphoreA(nullptr, 0, 1, semaphore_name.c_str());
                if (semaphore_handle == nullptr)
                {
                    throw std::runtime_error("CreateSemaphoreA failed, error: " + std::to_string(GetLastError()));
                }

                semaphores_[thread_id] = semaphore_handle;
                return semaphore_handle;
            }

            auto semaphore_handle = semaphores_[thread_id];
            WaitForSingleObject(semaphore_handle, 0); // reset
            return semaphore_handle;
        }

    public:
        explicit condition_variable(std::string const& name, bool const is_owner)
            : thread_queue_memory_(name + std::string("_thread_queue"), THREAD_QUEUE_STORAGE_SIZE, is_owner),
              thread_queue_(thread_queue_memory_.address(), THREAD_QUEUE_STORAGE_SIZE, is_owner),
              thread_queue_mutex_(name + std::string("_cond_mutex"), is_owner),
              name_(name)
        {
        }

        void notify_one()
        {
            std::unique_lock<::impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);

            auto id = thread_queue_.front();
            thread_queue_.pop();

            ReleaseSemaphore(get_semaphore(id), 1, nullptr);
        }

        void notify_all()
        {
            std::unique_lock<::impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);

            while (thread_queue_.count() > 0)
            {
                auto id = thread_queue_.front();
                thread_queue_.pop();

                ReleaseSemaphore(get_semaphore(id), 1, nullptr);
            }
        }

        void wait(std::unique_lock<::impulso::ipc::sync::robust_mutex>& mutex)
        {
            std::unique_lock<::impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);
            const auto current_thread_id = std::this_thread::get_id();
            const auto semaphore = get_semaphore(current_thread_id);

            thread_queue_.push_back(current_thread_id);

            mutex.unlock();
            lock.unlock();

            const auto result = WaitForSingleObject(semaphore, INFINITE);

            lock.lock();
            mutex.lock();

            if (WAIT_FAILED == result)
            {
                thread_queue_.remove(current_thread_id);
                throw std::runtime_error("WaitForSingleObject failed, error: " + std::to_string(GetLastError()));
            }
        }

        std::cv_status wait_for(std::unique_lock<::impulso::ipc::sync::robust_mutex>& mutex, std::chrono::milliseconds const& timeout)
        {
            std::unique_lock<::impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);
            const auto current_thread_id = std::this_thread::get_id();
            const auto semaphore = get_semaphore(current_thread_id);

            thread_queue_.push_back(current_thread_id);

            mutex.unlock();
            lock.unlock();

            const DWORD result = WaitForSingleObject(semaphore, static_cast<DWORD>(timeout.count()));

            lock.lock();
            mutex.lock();

            if (WAIT_FAILED == result)
            {
                thread_queue_.remove(current_thread_id);
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

impulso::ipc::sync::condition_variable::condition_variable(std::string const& name, bool const is_owner)
    : impl_(std::make_unique<::condition_variable>(name, is_owner))
{
}

impulso::ipc::sync::condition_variable::condition_variable() : impl_(nullptr)
{
}

impulso::ipc::sync::condition_variable::~condition_variable()
{
}

void impulso::ipc::sync::condition_variable::notify_one()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    instance->notify_one();
}

void impulso::ipc::sync::condition_variable::notify_all()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    instance->notify_all();
}

void impulso::ipc::sync::condition_variable::wait(std::unique_lock<ipc::sync::robust_mutex>& mutex)
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    instance->wait(mutex);
}

std::cv_status impulso::ipc::sync::condition_variable::wait_for(std::unique_lock<ipc::sync::robust_mutex>& mutex, std::chrono::milliseconds const& timeout)
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    return instance->wait_for(mutex, timeout);
}
