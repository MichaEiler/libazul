#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace azul
{
    namespace async
    {
        enum class future_errc : std::uint32_t
        {
            broken_promise = 0,
        };

        class future_error : public std::exception
        {
        public:
            explicit future_error(future_errc const errc)
                : errc_(errc)
            {
                
            }
            
            future_errc errc() const
            {
                return errc_;
            }

        private:
            future_errc errc_;
        };

        namespace detail
        {

            template <typename T>
            class future_state
            {
            public:
                explicit future_state()
                {

                }

                future_state(future_state const&) = delete;
                future_state(future_state&&) = delete;
                future_state& operator=(future_state const&) = delete;
                future_state& operator=(future_state&&) = delete;

                bool is_ready() const
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    return state_ != state_type::undefined;
                }

                const T& get()
                {
                    std::unique_lock<std::mutex> lock(mutex_);

                    for(;;)
                    {
                        switch (state_)
                        {
                        case state_type::exception:
                            std::rethrow_exception(exception_);
                        case state_type::broken_promise:
                            throw azul::async::future_error(azul::async::future_errc::broken_promise);
                        case state_type::ready:
                            return value_;
                        case state_type::undefined:
                            condition_.wait(lock);
                            break;
                        }
                    }
                }

                void set(T const& value)
                {
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        value_ = value;
                        state_ = state_type::ready;
                        condition_.notify_all();
                    }

                    for (const auto& continuation : continuations_)
                    {
                        continuation();
                    }
                }

                void set(std::exception_ptr const& ex)
                {
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        exception_ = ex;
                        state_ = state_type::exception;
                        condition_.notify_all();
                    }

                    for (const auto& continuation : continuations_)
                    {
                        continuation();
                    }
                }

                void wait()
                {
                    std::unique_lock<std::mutex> lock(mutex_);

                    if (state_ == future_state::state_type::undefined)
                    {
                        condition_.wait(lock);
                    }
                }

                template<class Rep, class Period>
                bool wait_for(std::chrono::duration<Rep,Period> const& timeout_duration) const
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    if (state_ == future_state::state_type::undefined)
                    {
                        return condition_.wait_for(lock, timeout_duration) != std::cv_status::timeout;
                    }
                    return true;
                }

                void then(std::function<void()> const& continuation)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        if (state_ == state_type::broken_promise)
                        {
                            return;
                        }

                        if (state_ == state_type::undefined)
                        {
                            continuations_.emplace_back(continuation);
                            return;
                        }
                    }

                    continuation();                    
                }

                std::size_t number_of_continuations() const
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    return continuations_.size();
                }

                void about_to_destroy_promise()
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (state_ == state_type::undefined)
                    {
                        state_ = state_type::broken_promise;
                    }
                    condition_.notify_all();
                }

            private:
                mutable std::condition_variable condition_;
                mutable std::mutex mutex_;

                enum class state_type
                {
                    undefined = 0,
                    ready = 1,
                    exception = 2,
                    broken_promise = 3,
                };

                state_type state_{ state_type::undefined };
                T value_{ };
                std::exception_ptr exception_{ };

                std::vector<std::function<void()>> continuations_;
            };

            template <>
            class future_state<void>
            {
            public:
                explicit future_state()
                {

                }

                future_state(future_state const& other) = delete;
                future_state(future_state&& other) = delete;
                future_state& operator=(future_state const& other) = delete;
                future_state& operator=(future_state&& other) = delete;

                bool is_ready() const
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    return state_ != state_type::undefined;
                }

                void get()
                {
                    std::unique_lock<std::mutex> lock(mutex_);

                    for(;;)
                    {
                        switch (state_)
                        {
                        case state_type::exception:
                            std::rethrow_exception(exception_);
                        case state_type::broken_promise:
                            throw azul::async::future_error(azul::async::future_errc::broken_promise);
                        case state_type::ready:
                            return;
                        case state_type::undefined:
                            condition_.wait(lock);
                            break;
                        }
                    }
                }

                void set()
                {
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        state_ = state_type::ready;
                        condition_.notify_all();
                    }

                    for (const auto& continuation : continuations_)
                    {
                        continuation();
                    }
                }

                void set(std::exception_ptr const& ex)
                {
                    {
                        std::lock_guard<std::mutex> lock(mutex_);
                        exception_ = ex;
                        state_ = state_type::exception;
                        condition_.notify_all();
                    }

                    for (const auto& continuation : continuations_)
                    {
                        continuation();
                    }
                }
            

                void wait()
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    if (state_ == future_state::state_type::undefined)
                    {
                        condition_.wait(lock);
                    }
                }

                template<class Rep, class Period>
                bool wait_for(std::chrono::duration<Rep,Period> const& timeout_duration) const
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    if (state_ == future_state::state_type::undefined)
                    {
                        return condition_.wait_for(lock, timeout_duration) != std::cv_status::timeout;
                    }
                    return true;
                }

                void then(std::function<void()> const& continuation)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        if (state_ == state_type::broken_promise)
                        {
                            return;
                        }

                        if (state_ == state_type::undefined)
                        {
                            continuations_.emplace_back(continuation);
                            return;
                        }
                    }

                    continuation();                    
                }

                std::size_t number_of_continuations() const
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    return continuations_.size();
                }

                void about_to_destroy_promise()
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (state_ == state_type::undefined)
                    {
                        state_ = state_type::broken_promise;
                    }
                    condition_.notify_all();
                }

            private:
                mutable std::condition_variable condition_;
                mutable std::mutex mutex_;

                enum class state_type
                {
                    undefined = 0,
                    ready = 1,
                    exception = 2,
                    broken_promise = 3,
                };

                state_type state_{ state_type::undefined };
                std::exception_ptr exception_{ };

                std::vector<std::function<void()>> continuations_;
            };
        }
    }
}