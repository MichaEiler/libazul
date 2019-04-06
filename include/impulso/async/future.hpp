#pragma once

#include <chrono>
#include <condition_variable>
#include <impulso/async/detail/future_state.hpp>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace impulso
{
    namespace async
    {   
        template <typename T>
        class future final
        {
        public:
            future()
                : state_(nullptr)
            {

            }

            explicit future(std::shared_ptr<detail::future_state<T>> const& state)
                : state_(state)
            {

            }

            ~future() noexcept
            {
                try {
                    if (state_)
                    {
                        state_.reset();
                    }
                } catch(...)
                { }
            }

            future(future const&) = default;
            future(future&&) = default;
            future& operator=(future const&) = default;
            future& operator=(future&&) = default;

            bool valid() const noexcept
            {
                return static_cast<bool>(state_);
            }

            bool is_ready() const
            {
                check();
                return state_->is_ready();
            }

            T get() const
            {
                check();
                return state_->get();
            }

            void wait() const
            {
                check();
                state_->wait();
            }

            template <class Rep, class Period>
            bool wait_for(std::chrono::duration<Rep,Period> const& timeout_duration) const
            {
                check();
                return state_->wait_for(timeout_duration);
            }

            template <typename F>
            typename impulso::async::future<std::invoke_result_t<F, future<T>>> then(F&& callable)
            {
                check();

                using TResult = std::invoke_result_t<F, future<T>>;

                auto state_of_result_future = std::shared_ptr<detail::future_state<TResult>>(new detail::future_state<TResult>(), [](auto* state){
                    state->about_to_destroy_promise();
                    delete state;
                });

                auto result_future = impulso::async::future<TResult>(state_of_result_future);

                auto continuation = [callable = std::function<TResult(impulso::async::future<T>)>(callable), state_of_result_future, copyOfThis = impulso::async::future<T>(*this)]()
                {
                    try
                    {
                        if constexpr (std::is_void_v<TResult>)
                        {
                            callable(copyOfThis);
                            state_of_result_future->set();
                        }
                        else
                        {
                            auto result = callable(copyOfThis);
                            state_of_result_future->set(result);
                        }
                    }
                    catch(...)
                    {
                        state_of_result_future->set(std::current_exception());
                    }
                };

                state_->then(continuation);
               
                return result_future;
            }

            std::size_t number_of_continuations() const
            {
                return state_->number_of_continuations();
            }

        private:
            void check() const
            {
                if (!state_)
                {
                    throw std::logic_error("Calling operations on an uninitialized object.");
                }
            }

            std::shared_ptr<detail::future_state<T>> state_;
        };

        template<typename T>
        class promise final
        {
        public:
            explicit promise()
                : state_(std::make_shared<detail::future_state<T>>())
            {

            }

            ~promise() noexcept
            {
                try {
                    if (state_)
                    {
                        state_->about_to_destroy_promise();
                        state_.reset();
                    }
                } catch(...)
                { }
            }

            promise(promise const&) = delete;
            promise(promise&&) = default;
            promise& operator=(promise const&) = delete;
            promise& operator=(promise&&) = default;

            bool valid() const noexcept
            {
                return static_cast<bool>(state_);
            }

            template <typename F, typename std::enable_if<std::is_same_v<F, T> && !std::is_void_v<F>>::type* = nullptr>
            void set_value(F const& value)
            {
                check();
                state_->set(value);
            }

            template <typename F = void, typename std::enable_if<std::is_void_v<F>>::type* = nullptr>
            void set_value()
            {
                check();
                state_->set();
            }

            void set_exception(std::exception_ptr const& ex)
            {
                check();
                state_->set(ex);
            }
            
            future<T> get_future() const
            {
                check();
                return future<T>(state_);
            }

            std::size_t number_of_continuations() const
            {
                return state_->number_of_continuations();
            }

        private:
            void check() const
            {
                if (!state_)
                {
                    throw std::logic_error("Calling operations on an uninitialized object.");
                }
            }

            std::shared_ptr<detail::future_state<T>> state_;
        };
    }
}