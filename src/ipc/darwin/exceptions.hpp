#pragma once

#include <stdexcept>

namespace impulso
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

