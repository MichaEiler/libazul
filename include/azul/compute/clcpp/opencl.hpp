#pragma once

#include <cstddef>
#include <cstdint>

namespace azul
{
    namespace compute
    {
        namespace clcpp
        {
            extern thread_local std::size_t global_index[3];

            static inline const std::size_t& get_global_id(const std::uint32_t& dimindx)
            {
                return global_index[dimindx];
            }

            static inline void set_global_id(const std::uint32_t& dimindx, const std::size_t& value)
            {
                global_index[dimindx] = value;
            }
        }
    }
}

