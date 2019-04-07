#include "azul/ipc/sync/condition_variable.hpp"

#include <azul/utils/disposer.hpp>
#include <azul/ipc/sync/robust_mutex.hpp>
#include <azul/ipc/shared_memory.hpp>
#include <linux/robust_mutex.hpp>
#include <pthread.h>
#include <time.h>

namespace
{
    class condition_variable final
    {
    private:
        static timespec get_absolute_time(std::chrono::milliseconds timeout)
        {
            struct timespec time;
            timespec_get(&time, TIME_UTC);

            auto additional_seconds = timeout.count() / 1000;
            timeout -= std::chrono::milliseconds(additional_seconds * 1000);
            time.tv_sec += additional_seconds;
            time.tv_nsec += timeout.count() * 1000000;

            if (time.tv_nsec > 1000000000)
            {
                time.tv_nsec -= 1000000000;
                time.tv_sec += 1;
            }

            return time;
        }

        ::azul::ipc::shared_memory cond_memory_;
        ::azul::utils::disposer cond_disposer_;
        pthread_cond_t* const handle_;

    public:
        explicit condition_variable(std::string const& name, bool const is_owner)
            : cond_memory_(std::string("ipc_cond_") + name, sizeof(pthread_cond_t), is_owner),
            handle_(reinterpret_cast<pthread_cond_t*>(cond_memory_.address()))
        {
            if (is_owner)
            {
                new (handle_) pthread_cond_t();

                pthread_condattr_t attributes;
                pthread_condattr_init(&attributes);
                pthread_condattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);
                pthread_condattr_setclock(&attributes, CLOCK_REALTIME);

                const auto result = pthread_cond_init(handle_, &attributes);
                pthread_condattr_destroy(&attributes);

                if (result != 0)
                {
                    throw std::runtime_error("pthread_cond init failed, error: " + std::to_string(result));
                }

                cond_disposer_.set([=]() { pthread_cond_destroy(handle_); });
            }
        }

        void notify_one()
        {
            pthread_cond_signal(handle_);
        }

        void notify_all()
        {
            pthread_cond_broadcast(handle_);
        }

        void wait(pthread_mutex_t *const mutex)
        {
            const auto result = pthread_cond_wait(handle_, mutex);
            if (result == EINVAL)
            {
                throw std::runtime_error("pthread_cond_wait failed, error: " + std::to_string(result));
            }
        }

        std::cv_status wait_for(pthread_mutex_t *const mutex, std::chrono::milliseconds const& timeout)
        {
            timespec deadline = get_absolute_time(timeout);
            const auto result = pthread_cond_timedwait(handle_, mutex, &deadline);
            if (result == 0)
            {
                return std::cv_status::no_timeout;
            }

            if (result == ETIMEDOUT)
            {
                return std::cv_status::timeout;
            }

            throw std::runtime_error("pthread_cond_timedwait failed, error: " + std::to_string(result));
        }
    };
}

// -----------------------------------------------------------------------------------------------------

azul::ipc::sync::condition_variable::condition_variable(std::string const& name, bool const is_owner)
    : impl_(std::make_unique<::condition_variable>(name, is_owner))
{
}

azul::ipc::sync::condition_variable::condition_variable() : impl_(nullptr)
{
}

azul::ipc::sync::condition_variable::~condition_variable()
{
}

void azul::ipc::sync::condition_variable::notify_one()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    instance->notify_one();
}

void azul::ipc::sync::condition_variable::notify_all()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    instance->notify_all();
}

void azul::ipc::sync::condition_variable::wait(std::unique_lock<ipc::sync::robust_mutex>& mutex)
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    ::robust_mutex *const mutex_instance = reinterpret_cast<::robust_mutex*>(mutex.mutex()->impl_.get());
    instance->wait(mutex_instance->handle_);
}

std::cv_status azul::ipc::sync::condition_variable::wait_for(std::unique_lock<ipc::sync::robust_mutex>& mutex, std::chrono::milliseconds const& timeout)
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::condition_variable *const instance = reinterpret_cast<::condition_variable*>(impl_.get());
    ::robust_mutex *const mutex_instance = reinterpret_cast<::robust_mutex*>(mutex.mutex()->impl_.get());
    return instance->wait_for(mutex_instance->handle_, timeout);
}
