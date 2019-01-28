#pragma once

#include <functional>

namespace impulso
{
    namespace utils
    {
        class disposer final
        {
        private:
            std::function<void()> func_;

        public:
            explicit disposer(std::function<void()> const& func) : func_(func)
            {
            }

            disposer()
            {
            }

            ~disposer()
            {
                if (func_)
                {
                    func_();
                }
            }

            void set(std::function<void()> const& func)
            {
                func_ = func;
            }

            disposer(disposer const&) = delete;
            disposer& operator=(disposer const&) = delete;

            disposer(disposer&& other)
            {
                func_ = other.func_;
                other.func_ = nullptr;
            }

            disposer& operator=(disposer&& other)
            {
                func_ = other.func_;
                other.func_ = nullptr;
                return *this;
            }
        };
    }
} // namespace detail
