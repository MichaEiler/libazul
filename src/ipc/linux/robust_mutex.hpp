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
    class robust_mutex final
    {
    private:
        ::azul::ipc::shared_memory mutex_memory_;
        ::azul::utils::disposer mutex_disposer_;
        pthread_mutex_t *const handle_ = nullptr;

        friend class ::azul::ipc::sync::condition_variable;
    public:
        explicit robust_mutex(std::string const& name, bool const is_owner)
            : mutex_memory_(std::string("ipc_mutex_") + name, sizeof(pthread_mutex_t), is_owner),
                handle_(reinterpret_cast<pthread_mutex_t*>(mutex_memory_.address()))
        {
            if (is_owner)
            {
                new (handle_) pthread_mutex_t();

                pthread_mutexattr_t attributes;
                pthread_mutexattr_init(&attributes);
                pthread_mutexattr_setrobust(&attributes, PTHREAD_MUTEX_ROBUST);
                pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);
                pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_ERRORCHECK);

                auto const result = pthread_mutex_init(handle_, &attributes);
                pthread_mutexattr_destroy(&attributes);

                if (result != 0)
                {
                    throw std::runtime_error("pthread_mutex_init failed, error: " + std::to_string(result));
                }

                mutex_disposer_.set([is_owner, handle = handle_]() {
                    if (is_owner)
                    {
                        pthread_mutex_destroy(handle);
                    }
                });
            }
        }

        void lock()
        {
            const auto result = pthread_mutex_lock(handle_);

            if (result == EDEADLK)
            {
                throw std::runtime_error("pthread_mutex_lock called on already locked mutex");
            }

            if (result == EOWNERDEAD)
            {
                pthread_mutex_consistent(handle_);
            }

            if (result != 0 && result != EOWNERDEAD)
            {
                throw std::runtime_error("pthread_mutex_lock failed with error code: " + std::to_string(result));
            }
        }

        bool try_lock()
        {
            const auto result = pthread_mutex_trylock(handle_);

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
            const auto result = pthread_mutex_unlock(handle_);

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
