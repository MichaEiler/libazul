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
            class condition_variable final
            {
            public:
                explicit condition_variable(std::string const& name, bool const is_owner);
                condition_variable();
                ~condition_variable();

                void notify_one();
                void notify_all();
                void wait(std::unique_lock<azul::ipc::sync::robust_mutex>& mutex);
                std::cv_status wait_for(std::unique_lock<azul::ipc::sync::robust_mutex>& mutex, std::chrono::milliseconds const& timeout);

                condition_variable(condition_variable const&) = delete;
                condition_variable operator=(condition_variable const&) = delete;
                condition_variable(condition_variable &&) = default;
                condition_variable& operator=(condition_variable &&) = default;

            private:
                std::shared_ptr<void> impl_;
            };
        }
    }
}
