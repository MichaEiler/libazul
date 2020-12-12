#pragma once

#include <azul/ipc/export.hpp>
#include <memory>
#include <string>

namespace azul
{
    namespace ipc
    {
        namespace sync
        {
            class ConditionVariable;

            class AZUL_IPC_EXPORT RobustMutex final
            {
            public:
                explicit RobustMutex(std::string const& name, bool const isOwner);
                RobustMutex();
                ~RobustMutex();

                // lock, try_lock, unlock functions must use these names
                // so that one can use the mutex with std::unique_lock and std::lock_guard

                void lock();
                bool try_lock();
                void unlock();

                RobustMutex(RobustMutex const&) = delete;
                RobustMutex& operator=(RobustMutex const&) = delete;
                RobustMutex(RobustMutex &&) = default;
                RobustMutex& operator=(RobustMutex &&) = default;

            private:
                friend class ConditionVariable;

                std::shared_ptr<void> _impl;
            };
        }
    }
}
