#pragma once

#include <memory>
#include <string>

namespace impulso
{
    namespace ipc
    {
        namespace sync
        {
            class condition_variable;

            class robust_mutex final
            {
            public:
                explicit robust_mutex(std::string const& name, bool const is_owner);
                robust_mutex();
                ~robust_mutex();

                void lock();
                bool try_lock();
                void unlock();

                robust_mutex(robust_mutex const&) = delete;
                robust_mutex& operator=(robust_mutex const&) = delete;
                robust_mutex(robust_mutex &&) = default;
                robust_mutex& operator=(robust_mutex &&) = default;

            private:
                friend class condition_variable;

                std::shared_ptr<void> impl_;
            };
        }
    }
}
