#pragma once

#include <functional>
#include <azul/async/future.hpp>
#include <azul/utils/disposer.hpp>
#include <memory>
#include <stdexcept>

namespace azul
{
    namespace async
    {
        class TaskBase
        {
        public:
            explicit TaskBase(azul::async::Future<void> const& dependency)
                : _dependency(dependency)
            {

            }

            explicit TaskBase()
                : _dependency()
            {

            }

            virtual ~TaskBase() = default;
            virtual void operator()() noexcept = 0;

            virtual bool IsReady() const noexcept
            {
                return !(_dependency.Valid() && !_dependency.IsReady());
            }

            virtual std::size_t NumberOfContinuations() const = 0;
        
        private:
            azul::async::Future<void> _dependency;
        };

        template <typename TResult>
        class Task : public TaskBase
        {
        public:
            explicit Task(std::function<TResult()> && func, azul::async::Future<void> const& dependency = { })
                : TaskBase(dependency)
                , _func(std::make_shared<std::function<TResult()>>(func))
            {
                
            }

            void operator()() noexcept override
            {
                try
                {
                    TResult result = _func->operator()();
                    _func.reset();
                    _promise.SetValue(result);
                }
                catch(...)
                {
                    _promise.SetException(std::current_exception());
                }
            }

            azul::async::Future<TResult> GetFuture() { return _promise.GetFuture(); }

            std::size_t NumberOfContinuations() const override
            {
                return _promise.NumberOfContinuations();
            }

        private:
            azul::async::Promise<TResult> _promise;
            std::shared_ptr<std::function<TResult()>> _func;
        };

        template <>
        class Task<void> : public TaskBase
        {
        public:
            explicit Task(std::function<void()> && func, azul::async::Future<void> const& dependency = { })
                : TaskBase(dependency)
                , _func(std::make_shared<std::function<void()>>(func))
            {

            }

            void operator()() noexcept override
            {
                try
                {
                    _func->operator()();
                    _func.reset();
                    _promise.SetValue();
                }
                catch(...)
                {
                    _promise.SetException(std::current_exception());
                }
            }

            azul::async::Future<void> GetFuture() { return _promise.GetFuture(); }
            
            std::size_t NumberOfContinuations() const override
            {
                return _promise.NumberOfContinuations();
            }

        private:
            azul::async::Promise<void> _promise;
            std::shared_ptr<std::function<void()>> _func;
        };

        template <typename... TFutures>
        Future<void> WrapDependencies(TFutures&&... futures)
        {
            auto sharedFutureState = std::make_shared<detail::FutureState<void>>();
            auto future = ::azul::async::Future<void>(sharedFutureState);

            auto sharedPromiseActivator = std::make_shared<azul::utils::Disposer>([sharedFutureState]() {
                sharedFutureState->SetValue();
            });

            auto func = [sharedPromiseActivator](auto) mutable {
                sharedPromiseActivator.reset();
            };

            (futures.Then(func),...);

            return future;
        }
    }
}
