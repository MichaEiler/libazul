#pragma once

#include <type_traits>
#include <tuple>

namespace azul
{
    namespace compute
    {
        namespace clcpp
        {
            template<typename TValue, typename std::enable_if<std::is_floating_point<TValue>::value || std::is_integral<TValue>::value>::type* = nullptr>
            struct vec2
            {
                TValue x, y;

                constexpr vec2()
                    : x(static_cast<TValue>(0)), y(static_cast<TValue>(0))
                {
                }

                constexpr vec2(TValue const& _x, TValue const& _y)
                    : x(_x), y(_y)
                {
                }

                constexpr vec2<TValue>& operator +=(vec2<TValue> const& other)
                {
                    x += other.x;
                    y += other.y;

                    return *this;
                }

                constexpr vec2<TValue> operator +(vec2<TValue> const& other) const
                {
                    return vec2<TValue>(x + other.x, y + other.y);
                }

                constexpr vec2<TValue>& operator -=(vec2<TValue> const& other)
                {
                    x -= other.x;
                    y -= other.y;
                    
                    return *this;
                }

                constexpr vec2<TValue> operator -(vec2<TValue> const& other) const
                {
                    return vec2<TValue>(x - other.x, y - other.y);
                }

                constexpr vec2<TValue>& operator *=(TValue const& factor)
                {
                    x *= factor;
                    y *= factor;
                    
                    return *this;
                }

                constexpr vec2<TValue> operator *(TValue const& factor) const
                {
                    return vec2<TValue>(x * factor, y * factor);
                }

                constexpr vec2<TValue>& operator /=(TValue const& factor)
                {
                    x /= factor;
                    y /= factor;
                    
                    return *this;
                }

                constexpr vec2<TValue> operator /(TValue const& factor) const
                {
                    return vec2<TValue>(x / factor, y / factor);
                }

                constexpr bool operator==(vec2<TValue> const& other) const
                {
                    return x == other.x && y == other.y;
                }

                constexpr bool operator!=(vec2<TValue> const& other) const
                {
                    return !(*this == other);
                }
            };

            template<typename TValue, typename std::enable_if<std::is_floating_point<TValue>::value || std::is_integral<TValue>::value>::type* = nullptr>
            struct vec4
            {
                TValue x, y, z, w;

                constexpr vec4()
                    : x(static_cast<TValue>(0))
                    , y(static_cast<TValue>(0))
                    , z(static_cast<TValue>(0))
                    , w(static_cast<TValue>(0))
                {
                }

                constexpr vec4(TValue const& _x, TValue const& _y, TValue const& _z, TValue const& _w)
                    : x(_x), y(_y), z(_z), w(_w)
                {
                }

                constexpr vec4<TValue>& operator +=(vec4<TValue> const& other)
                {
                    x += other.x;
                    y += other.y;
                    z += other.z;
                    w += other.w;

                    return *this;
                }

                constexpr vec4<TValue> operator +(vec4<TValue> const& other) const
                {
                    return vec4<TValue>(x + other.x, y + other.y, z + other.z, w + other.w);
                }

                constexpr vec4<TValue>& operator -=(vec4<TValue> const& other)
                {
                    x -= other.x;
                    y -= other.y;
                    z -= other.z;
                    w -= other.w;
                    
                    return *this;
                }

                constexpr vec4<TValue> operator -(vec4<TValue> const& other) const
                {
                    return vec4<TValue>(x - other.x, y - other.y, z - other.z, w - other.w);
                }

                constexpr vec4<TValue>& operator *=(TValue const& factor)
                {
                    x *= factor;
                    y *= factor;
                    z *= factor;
                    w *= factor;
                    
                    return *this;
                }

                constexpr vec4<TValue> operator *(TValue const& other) const
                {
                    return vec4<TValue>(x * other, y * other, z * other, w * other);
                }

                constexpr vec4<TValue>& operator /=(TValue const& factor)
                {
                    x /= factor;
                    y /= factor;
                    z /= factor;
                    w /= factor;
                    
                    return *this;
                }

                constexpr vec4<TValue> operator /(TValue const& other) const
                {
                    return vec4<TValue>(x / other, y / other, z / other, w / other);
                }

                constexpr bool operator==(vec4<TValue> const& other) const
                {
                    return x == other.x && y == other.y && z == other.z && w == other.w;
                }

                constexpr bool operator!=(vec4<TValue> const& other) const
                {
                    return !(*this == other);
                }
            };
        }
    }
}