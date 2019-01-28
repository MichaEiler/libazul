#include "impulso/ipc/sync/condition_variable.hpp"

#include <errno.h>
#include <fcntl.h>
#include <impulso/ipc/sync/robust_mutex.hpp>
#include <impulso/ipc/shared_memory.hpp>
#include <impulso/utils/disposer.hpp>
#include <memory>
#include <mutex>
#include <queue.hpp>
#include <semaphore.h>
#include <sstream>
#include <string>
#include <thread>

#include "fifo.hpp"

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mkfifo.2.html
// https://apple.stackexchange.com/questions/261288/why-max-inode-count-became-232-1-on-my-hdd-after-update-to-os-x-10-12-sierra
// https://wilsonmar.github.io/maximum-limits/
// http://cgi.di.uoa.gr/~ad/k22/named-pipes.pdf

namespace {
    constexpr int THREAD_QUEUE_STORAGE_SIZE = 128 * 1024;
    constexpr int FIFO_SYNC_MESSAGE = 0x12345678;


    class condition_variable
    {
    private:
        impulso::ipc::shared_memory thread_queue_memory_;
        impulso::ipc::detail::queue<std::thread::id> thread_queue_;
        impulso::ipc::sync::robust_mutex thread_queue_mutex_;
        std::string const name_;

        static std::shared_ptr<::impulso::ipc::detail::fifo> get_fifo(std::string const& name, std::thread::id const& thread_id, bool const is_owner)
        {
            std::stringstream memory_stream;
            memory_stream << thread_id;
            const auto full_name = name + "_" + memory_stream.str();
            const auto fifo = std::make_shared<::impulso::ipc::detail::fifo>(full_name, is_owner);
            return fifo;
        }

    public:
        explicit condition_variable(std::string const& name, bool const is_owner)
            : thread_queue_memory_(name + "_threadqueue", THREAD_QUEUE_STORAGE_SIZE, is_owner)
            , thread_queue_(thread_queue_memory_.address(), THREAD_QUEUE_STORAGE_SIZE, is_owner)
            , thread_queue_mutex_(name + "_threadqueu", is_owner)
            , name_(name)
        {

        }

        void notify_one()
        {
            std::unique_lock<impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);

            if (thread_queue_.count() > 0)
            {
                auto const waiting_thread_id = thread_queue_.front();
                thread_queue_.pop();

                auto fifo = get_fifo(name_, waiting_thread_id, false);
                fifo->write(reinterpret_cast<const char*>(&FIFO_SYNC_MESSAGE), sizeof(FIFO_SYNC_MESSAGE));
            }
        }

        void notify_all()
        {
            std::unique_lock<impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);
            while (thread_queue_.count() > 0)
            {
                auto const waiting_thread_id = thread_queue_.front();
                thread_queue_.pop();
                auto fifo = get_fifo(name_, waiting_thread_id, false);
                fifo->write(reinterpret_cast<const char*>(&FIFO_SYNC_MESSAGE), sizeof(FIFO_SYNC_MESSAGE));
            }
        }

        void wait(std::unique_lock<::impulso::ipc::sync::robust_mutex>& mutex)
        {
            std::unique_lock<::impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);
            const auto current_thread_id = std::this_thread::get_id();
            const auto fifo = get_fifo(name_, current_thread_id, true);

            thread_queue_.push_back(current_thread_id);

            mutex.unlock();
            lock.unlock();

            char buffer[sizeof(FIFO_SYNC_MESSAGE)];
            int toRead = static_cast<int>(sizeof(FIFO_SYNC_MESSAGE));
            while (toRead > 0)
            {
                toRead -= static_cast<int>(fifo->read(buffer, static_cast<std::size_t>(toRead)));
            }

            lock.lock();
            mutex.lock();
        }

        std::cv_status wait_for(std::unique_lock<::impulso::ipc::sync::robust_mutex>& mutex, std::chrono::milliseconds const& timeout)
        {
            std::unique_lock<::impulso::ipc::sync::robust_mutex> lock(thread_queue_mutex_);
            const auto current_thread_id = std::this_thread::get_id();
            const auto fifo = get_fifo(name_, current_thread_id, true);

            thread_queue_.push_back(current_thread_id);

            char buffer[sizeof(FIFO_SYNC_MESSAGE)];
            int toRead = static_cast<int>(sizeof(FIFO_SYNC_MESSAGE));
            while (toRead > 0)
            {
                mutex.unlock();
                lock.unlock();

                const auto result = fifo->timed_read(buffer, static_cast<std::size_t>(toRead), static_cast<std::uint32_t>(timeout.count()));

                lock.lock();
                mutex.lock();

                if (result < 0)
                {
                    return std::cv_status::timeout;
                }
                toRead -= static_cast<int>(result);
            }

            return std::cv_status::no_timeout;

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
