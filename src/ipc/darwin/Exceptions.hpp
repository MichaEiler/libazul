#pragma once

#include <stdexcept>

namespace azul
{
    namespace ipc
    {
        namespace detail
        {
            class ResourceMissingError final : public std::runtime_error
            {
            public:
                explicit ResourceMissingError() : std::runtime_error("")
                { }
            };
        }
    }
}

