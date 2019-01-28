#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace impulso
{
    namespace ipc
    {
        class shared_memory final
        {
        public:
            explicit shared_memory(std::string const& name, std::uint64_t const size, bool const is_owner);
            shared_memory();
            ~shared_memory();

            void* address() const;
            std::uint64_t size() const;

            shared_memory& operator=(shared_memory const&) = delete;
            shared_memory(shared_memory const&) = delete;
            shared_memory& operator=(shared_memory&&) = default;
            shared_memory(shared_memory&&) = default;

        private:
            std::shared_ptr<void> impl_;
        };
    }
}

