#pragma once

#include <stdexcept>

namespace azul
{
    namespace ipc
    {
        namespace detail
        {
            class resource_missing_error final : public std::runtime_error
            {
            public:
                explicit resource_missing_error() : std::runtime_error("")
                { }
            };
        }
    }
}

