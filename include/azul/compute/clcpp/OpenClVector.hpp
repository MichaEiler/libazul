#pragma once

#include <azul/compute/export.hpp>
#include <type_traits>
#include <tuple>

namespace azul
{
    namespace compute
    {
        namespace clcpp
        {
            template<typename TValue, typename std::enable_if<std::is_floating_point<TValue>::value || std::is_integral<TValue>::value>::type* = nullptr>
            struct AZUL_COMPUTE_EXPORT Vec2
            {
                TValue x, y;

                constexpr Vec2()
                    : x(static_cast<TValue>(0)), y(static_cast<TValue>(0))
                {
                }

                constexpr Vec2(TValue const& _x, TValue const& _y)
                    : x(_x), y(_y)
                {
                }

                constexpr Vec2<TValue>& operator +=(Vec2<TValue> const& other)
                {
                    x += other.x;
                    y += other.y;

                    return *this;
                }

                constexpr Vec2<TValue> operator +(Vec2<TValue> const& other) const
                {
                    return Vec2<TValue>(x + other.x, y + other.y);
                }

                constexpr Vec2<TValue>& operator -=(Vec2<TValue> const& other)
                {
                    x -= other.x;
                    y -= other.y;
                    
                    return *this;
                }

                constexpr Vec2<TValue> operator -(Vec2<TValue> const& other) const
                {
                    return Vec2<TValue>(x - other.x, y - other.y);
                }

                constexpr Vec2<TValue>& operator *=(TValue const& factor)
                {
                    x *= factor;
                    y *= factor;
                    
                    return *this;
                }

                constexpr Vec2<TValue> operator *(TValue const& factor) const
                {
                    return Vec2<TValue>(x * factor, y * factor);
                }

                constexpr Vec2<TValue>& operator /=(TValue const& factor)
                {
                    x /= factor;
                    y /= factor;
                    
                    return *this;
                }

                constexpr Vec2<TValue> operator /(TValue const& factor) const
                {
                    return Vec2<TValue>(x / factor, y / factor);
                }

                constexpr bool operator==(Vec2<TValue> const& other) const
                {
                    return x == other.x && y == other.y;
                }

                constexpr bool operator!=(Vec2<TValue> const& other) const
                {
                    return !(*this == other);
                }
            };

            template<typename TValue, typename std::enable_if<std::is_floating_point<TValue>::value || std::is_integral<TValue>::value>::type* = nullptr>
            struct AZUL_COMPUTE_EXPORT Vec4
            {
                TValue x, y, z, w;

                constexpr Vec4()
                    : x(static_cast<TValue>(0))
                    , y(static_cast<TValue>(0))
                    , z(static_cast<TValue>(0))
                    , w(static_cast<TValue>(0))
                {
                }

                constexpr Vec4(TValue const& _x, TValue const& _y, TValue const& _z, TValue const& _w)
                    : x(_x), y(_y), z(_z), w(_w)
                {
                }

                constexpr Vec4<TValue>& operator +=(Vec4<TValue> const& other)
                {
                    x += other.x;
                    y += other.y;
                    z += other.z;
                    w += other.w;

                    return *this;
                }

                constexpr Vec4<TValue> operator +(Vec4<TValue> const& other) const
                {
                    return Vec4<TValue>(x + other.x, y + other.y, z + other.z, w + other.w);
                }

                constexpr Vec4<TValue>& operator -=(Vec4<TValue> const& other)
                {
                    x -= other.x;
                    y -= other.y;
                    z -= other.z;
                    w -= other.w;
                    
                    return *this;
                }

                constexpr Vec4<TValue> operator -(Vec4<TValue> const& other) const
                {
                    return Vec4<TValue>(x - other.x, y - other.y, z - other.z, w - other.w);
                }

                constexpr Vec4<TValue>& operator *=(TValue const& factor)
                {
                    x *= factor;
                    y *= factor;
                    z *= factor;
                    w *= factor;
                    
                    return *this;
                }

                constexpr Vec4<TValue> operator *(TValue const& other) const
                {
                    return Vec4<TValue>(x * other, y * other, z * other, w * other);
                }

                constexpr Vec4<TValue>& operator /=(TValue const& factor)
                {
                    x /= factor;
                    y /= factor;
                    z /= factor;
                    w /= factor;
                    
                    return *this;
                }

                constexpr Vec4<TValue> operator /(TValue const& other) const
                {
                    return Vec4<TValue>(x / other, y / other, z / other, w / other);
                }

                constexpr bool operator==(Vec4<TValue> const& other) const
                {
                    return x == other.x && y == other.y && z == other.z && w == other.w;
                }

                constexpr bool operator!=(Vec4<TValue> const& other) const
                {
                    return !(*this == other);
                }
            };
        }
    }
}
