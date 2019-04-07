#pragma once

#include <functional>

namespace azul
{
    namespace utils
    {
        class Disposer final
        {
        private:
            std::function<void()> _func;

        public:
            explicit Disposer(std::function<void()> const& func) : _func(func)
            {
            }

            Disposer()
            {
            }

            ~Disposer()
            {
                if (_func)
                {
                    _func();
                }
            }

            void Set(std::function<void()> const& func)
            {
                _func = func;
            }

            Disposer(Disposer const&) = delete;
            Disposer& operator=(Disposer const&) = delete;

            Disposer(Disposer&& other)
            {
                _func = other._func;
                other._func = nullptr;
            }

            Disposer& operator=(Disposer&& other)
            {
                _func = other._func;
                other._func = nullptr;
                return *this;
            }
        };
    }
} // namespace detail
