#pragma once

#include <azul/compute/export.hpp>
#include <cstddef>
#include <cstdint>

namespace azul
{
    namespace compute
    {
        namespace clcpp
        {
            extern thread_local std::size_t AZUL_COMPUTE_EXPORT global_index[3];

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

