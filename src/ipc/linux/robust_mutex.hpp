#pragma once

// http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html
// http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_init.html
// http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_getrobust.html
// http://pubs.opengroup.org/onlinepubs/009604499/functions/pthread_mutexattr_getpshared.html
// https://linux.die.net/man/3/pthread_mutex_trylock

#include <azul/ipc/shared_memory.hpp>
#include <azul/ipc/sync/condition_variable.hpp>
#include <azul/utils/disposer.hpp>
#include <pthread.h>
#include <string>

namespace
{
    class RobustMutex final
    {
    private:
        ::azul::ipc::SharedMemory _mutexMemory;
        ::azul::utils::Disposer _mutexDisposer;
        pthread_mutex_t *const _handle = nullptr;

        friend class ::azul::ipc::sync::ConditionVariable;
    public:
        explicit RobustMutex(std::string const& name, bool const isOwner)
            : _mutexMemory(std::string("ipc_mutex_") + name, sizeof(pthread_mutex_t), isOwner)
            , _handle(reinterpret_cast<pthread_mutex_t*>(_mutexMemory.Address()))
        {
            if (isOwner)
            {
                new (_handle) pthread_mutex_t();

                pthread_mutexattr_t attributes;
                pthread_mutexattr_init(&attributes);
                pthread_mutexattr_setrobust(&attributes, PTHREAD_MUTEX_ROBUST);
                pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);
                pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_ERRORCHECK);

                auto const result = pthread_mutex_init(_handle, &attributes);
                pthread_mutexattr_destroy(&attributes);

                if (result != 0)
                {
                    throw std::runtime_error("pthread_mutex_init failed, error: " + std::to_string(result));
                }

                _mutexDisposer.Set([isOwner, handle = _handle]() {
                    if (isOwner)
                    {
                        pthread_mutex_destroy(handle);
                    }
                });
            }
        }

        void lock()
        {
            const auto result = pthread_mutex_lock(_handle);

            if (result == EDEADLK)
            {
                throw std::runtime_error("pthread_mutex_lock called on already locked mutex");
            }

            if (result == EOWNERDEAD)
            {
                pthread_mutex_consistent(_handle);
            }

            if (result != 0 && result != EOWNERDEAD)
            {
                throw std::runtime_error("pthread_mutex_lock failed with error code: " + std::to_string(result));
            }
        }

        bool try_lock()
        {
            const auto result = pthread_mutex_trylock(_handle);

            if (result == EDEADLK)
            {
                throw std::runtime_error("pthread_mutex_trylock called on already locked mutex");
            }

            if (result != 0 && result != EBUSY)
            {
                throw std::runtime_error("pthread_mutex_trylock failed with error code: " + std::to_string(result));
            }

            return result == 0;
        }

        void unlock()
        {
            const auto result = pthread_mutex_unlock(_handle);

            if (result == EPERM)
            {
                throw std::runtime_error("pthread_mutex_unlock called from other thread");
            }
            if (result != 0)
            {
                throw std::runtime_error("pthread_mutex_unlock failed with error code: " + std::to_string(result));
            }
        }
    };
}
