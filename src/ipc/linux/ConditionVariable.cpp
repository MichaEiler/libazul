#include "azul/ipc/sync/ConditionVariable.hpp"

#include <azul/utils/Disposer.hpp>
#include <azul/ipc/sync/RobustMutex.hpp>
#include <azul/ipc/SharedMemory.hpp>
#include <linux/RobustMutex.hpp>
#include <pthread.h>
#include <time.h>

namespace
{
    class ConditionVariable final
    {
    private:
        static timespec getAbsoluteTime(std::chrono::milliseconds timeout)
        {
            struct timespec time;
            timespec_get(&time, TIME_UTC);

            auto additionalSeconds = timeout.count() / 1000;
            timeout -= std::chrono::milliseconds(additionalSeconds * 1000);
            time.tv_sec += additionalSeconds;
            time.tv_nsec += timeout.count() * 1000000;

            if (time.tv_nsec > 1000000000)
            {
                time.tv_nsec -= 1000000000;
                time.tv_sec += 1;
            }

            return time;
        }

        ::azul::ipc::SharedMemory _conditionMemory;
        ::azul::utils::Disposer _conditionDisposer;
        pthread_cond_t* const _handle;

    public:
        explicit ConditionVariable(std::string const& name, bool const isOwner)
            : _conditionMemory(std::string("ipc_cond_") + name, sizeof(pthread_cond_t), isOwner),
            _handle(reinterpret_cast<pthread_cond_t*>(_conditionMemory.Address()))
        {
            if (isOwner)
            {
                new (_handle) pthread_cond_t();

                pthread_condattr_t attributes;
                pthread_condattr_init(&attributes);
                pthread_condattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);
                pthread_condattr_setclock(&attributes, CLOCK_REALTIME);

                const auto result = pthread_cond_init(_handle, &attributes);
                pthread_condattr_destroy(&attributes);

                if (result != 0)
                {
                    throw std::runtime_error("pthread_cond init failed, error: " + std::to_string(result));
                }

                _conditionDisposer.Set([=]() { pthread_cond_destroy(_handle); });
            }
        }

        void NotifyOne()
        {
            pthread_cond_signal(_handle);
        }

        void NotifyAll()
        {
            pthread_cond_broadcast(_handle);
        }

        void Wait(pthread_mutex_t *const mutex)
        {
            const auto result = pthread_cond_wait(_handle, mutex);
            if (result == EINVAL)
            {
                throw std::runtime_error("pthread_cond_wait failed, error: " + std::to_string(result));
            }
        }

        std::cv_status WaitFor(pthread_mutex_t *const mutex, std::chrono::milliseconds const& timeout)
        {
            timespec deadline = getAbsoluteTime(timeout);
            const auto result = pthread_cond_timedwait(_handle, mutex, &deadline);
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
    ::RobustMutex *const mutex_instance = reinterpret_cast<::RobustMutex*>(mutex.mutex()->_impl.get());
    instance->Wait(mutex_instance->_handle);
}

std::cv_status azul::ipc::sync::ConditionVariable::WaitFor(std::unique_lock<ipc::sync::RobustMutex>& mutex, std::chrono::milliseconds const& timeout)
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    ::RobustMutex *const mutex_instance = reinterpret_cast<::RobustMutex*>(mutex.mutex()->_impl.get());
    return instance->WaitFor(mutex_instance->_handle, timeout);
}
