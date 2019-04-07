#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace azul
{
    namespace ipc
    {
        class SharedMemory final
        {
        public:
            explicit SharedMemory(std::string const& name, std::uint64_t const size, bool const is_owner);
            SharedMemory();
            ~SharedMemory();

            void* Address() const;
            std::uint64_t Size() const;

            SharedMemory& operator=(SharedMemory const&) = delete;
            SharedMemory(SharedMemory const&) = delete;
            SharedMemory& operator=(SharedMemory&&) = default;
            SharedMemory(SharedMemory&&) = default;

        private:
            std::shared_ptr<void> _impl;
        };
    }
}

