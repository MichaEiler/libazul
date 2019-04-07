#pragma once

#include <chrono>
#include <condition_variable>
#include <azul/ipc/sync/robust_mutex.hpp>
#include <memory>
#include <mutex>
#include <string>

namespace azul
{
    namespace ipc
    {
        namespace sync
        {
            class ConditionVariable final
            {
            public:
                explicit ConditionVariable(std::string const& name, bool const isOwner);
                ConditionVariable();
                ~ConditionVariable();

                void NotifyOne();
                void NotifyAll();
                void Wait(std::unique_lock<azul::ipc::sync::RobustMutex>& mutex);
                std::cv_status WaitFor(std::unique_lock<azul::ipc::sync::RobustMutex>& mutex, std::chrono::milliseconds const& timeout);

                ConditionVariable(ConditionVariable const&) = delete;
                ConditionVariable operator=(ConditionVariable const&) = delete;
                ConditionVariable(ConditionVariable &&) = default;
                ConditionVariable& operator=(ConditionVariable &&) = default;

            private:
                std::shared_ptr<void> _impl;
            };
        }
    }
}
