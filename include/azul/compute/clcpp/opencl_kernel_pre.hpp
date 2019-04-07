#ifndef __OPENCL_HEADER_INCLUDES
#define __OPENCL_HEADER_INCLUDES

#include "opencl.hpp"
#include "opencl_vector.hpp"

#include <cmath>
#include <limits>
#endif // __OPENCL_HEADER_INCLUDES



#ifndef __OPENCL_COMPAT_DEFINES
#define __OPENCL_COMPAT_DEFINES

#define __kernel static inline
#define __global
#define __local
#define __private
#define __constant const

#define get_global_id azul::compute::clcpp::get_global_id

#define uchar       std::uint8_t
#define float2      azul::compute::clcpp::Vec2<float>
#define float4      azul::compute::clcpp::Vec4<float>
#define double2     azul::compute::clcpp::Vec2<double>
#define double4     azul::compute::clcpp::Vec4<double>
#define int2        azul::compute::clcpp::Vec2<std::int32_t>
#define int4        azul::compute::clcpp::Vec4<std::int32_t>
#define uint2       azul::compute::clcpp::Vec2<std::uint32_t>
#define uint4       azul::compute::clcpp::Vec4<std::uint32_t>

#define make_int2(a,b)          azul::compute::clcpp::Vec2<std::int32_t>(a, b)
#define make_int4(a,b,c,d)      azul::compute::clcpp::Vec4<std::int32_t>(a, b, c, d)
#define make_uint2(a,b)         azul::compute::clcpp::Vec2<std::uint32_t>(a, b)
#define make_uint4(a,b,c,d)     azul::compute::clcpp::Vec4<std::uint32_t>(a, b, c, d)
#define make_float2(a,b)        azul::compute::clcpp::Vec2<float>(a, b)
#define make_float4(a,b,c,d)    azul::compute::clcpp::Vec4<float>(a, b, c, d)

//#define FLT_MAX     std::numeric_limits<float>::max()
#define native_exp  std::exp

#ifndef OPENCL_KERNEL_NAMESPACE
#define OPENCL_KERNEL_NAMESPACE clcpp
#endif

namespace OPENCL_KERNEL_NAMESPACE {

using namespace std;


#endif // __OPENCL_COMPAT_DEFINES
