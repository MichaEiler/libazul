#pragma once

#include <functional>
#include <impulso/async/future.hpp>
#include <impulso/utils/disposer.hpp>
#include <memory>
#include <stdexcept>

namespace impulso
{
    namespace async
    {
        namespace detail
        {
            class base_task_type
            {
            public:
                explicit base_task_type(impulso::async::future<void> const& dependency)
                    : dependency_(dependency)
                {

                }

                explicit base_task_type()
                    : dependency_()
                {

                }

                virtual ~base_task_type() = default;
                virtual void operator()() noexcept = 0;

                virtual bool is_ready() const noexcept
                {
                    return !(dependency_.valid() && !dependency_.is_ready());
                }

                virtual std::size_t number_of_continuations() const = 0;
            
            private:
                impulso::async::future<void> dependency_;
            };

            template <typename TResult>
            class task_type : public base_task_type
            {
            public:
                explicit task_type(std::function<TResult()> && func, impulso::async::future<void> const& dependency = { })
                    : base_task_type(dependency)
                    , func_(std::make_shared<std::function<TResult()>>(func))
                {
                    
                }

                void operator()() noexcept override
                {
                    try
                    {
                        TResult result = func_->operator()();
                        func_.reset();
                        promise_.set_value(result);
                    }
                    catch(...)
                    {
                        promise_.set_exception(std::current_exception());
                    }
                }

                impulso::async::future<TResult> get_future() { return promise_.get_future(); }

                std::size_t number_of_continuations() const override
                {
                    return promise_.number_of_continuations();
                }

            private:
                impulso::async::promise<TResult> promise_;
                std::shared_ptr<std::function<TResult()>> func_;
            };

            template <>
            class task_type<void> : public base_task_type
            {
            public:
                explicit task_type(std::function<void()> && func, impulso::async::future<void> const& dependency = { })
                    : base_task_type(dependency)
                    , func_(std::make_shared<std::function<void()>>(func))
                {

                }

                void operator()() noexcept override
                {
                    try
                    {
                        func_->operator()();
                        func_.reset();
                        promise_.set_value();
                    }
                    catch(...)
                    {
                        promise_.set_exception(std::current_exception());
                    }
                }

                impulso::async::future<void> get_future() { return promise_.get_future(); }
                
                std::size_t number_of_continuations() const override
                {
                    return promise_.number_of_continuations();
                }

            private:
                impulso::async::promise<void> promise_;
                std::shared_ptr<std::function<void()>> func_;
            };

            template <typename... TFutures>
            future<void> wrap_dependencies(TFutures&&... futures)
            {
                auto shared_future_state = std::make_shared<detail::future_state<void>>();
                auto future = ::impulso::async::future<void>(shared_future_state);

                auto shared_promise_activator = std::make_shared<impulso::utils::disposer>([shared_future_state]() {
                    shared_future_state->set();
                });

                auto func = [shared_promise_activator](auto) mutable {
                    shared_promise_activator.reset();
                };

                (futures.then(func),...);

                return future;
            }
        }
    }
}
