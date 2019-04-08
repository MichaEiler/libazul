#pragma once

#include <chrono>
#include <condition_variable>
#include <azul/async/detail/FutureState.hpp>
#include <azul/utils/Disposer.hpp>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace azul
{
    namespace async
    {   
        template <typename T>
        class Future final
        {
        public:
            Future()
                : _state(nullptr)
            {

            }

            explicit Future(std::shared_ptr<detail::FutureState<T>> const& state)
                : _state(state)
            {

            }

            ~Future() noexcept
            {
                try {
                    if (_state)
                    {
                        _state.reset();
                    }
                } catch(...)
                { }
            }

            Future(Future const&) = default;
            Future(Future&&) = default;
            Future& operator=(Future const&) = default;
            Future& operator=(Future&&) = default;

            bool Valid() const noexcept
            {
                return static_cast<bool>(_state);
            }

            bool IsReady() const
            {
                Check();
                return _state->IsReady();
            }

            T Get() const
            {
                Check();
                return _state->Get();
            }

            void Wait() const
            {
                Check();
                _state->Wait();
            }

            template <class Rep, class Period>
            bool WaitFor(std::chrono::duration<Rep,Period> const& timeoutDuration) const
            {
                Check();
                return _state->WaitFor(timeoutDuration);
            }

            template <typename F>
            typename ::azul::async::Future<std::invoke_result_t<F, Future<T>>> Then(F&& callable)
            {
                Check();

                using TResult = std::invoke_result_t<F, Future<T>>;

                auto stateOfResultFuture = std::shared_ptr<detail::FutureState<TResult>>(new detail::FutureState<TResult>(), [](auto* state){
                    state->AboutToDestroyPromise();
                    delete state;
                });

                auto resultFuture = azul::async::Future<TResult>(stateOfResultFuture);

                auto continuation = [callable = std::function<TResult(azul::async::Future<T>)>(callable), stateOfResultFuture, copyOfThis = azul::async::Future<T>(*this)]()
                {
                    try
                    {
                        if constexpr (std::is_void_v<TResult>)
                        {
                            callable(copyOfThis);
                            stateOfResultFuture->SetValue();
                        }
                        else
                        {
                            auto result = callable(copyOfThis);
                            stateOfResultFuture->SetValue(result);
                        }
                    }
                    catch(...)
                    {
                        stateOfResultFuture->SetException(std::current_exception());
                    }
                };

                _state->Then(continuation);
               
                return resultFuture;
            }

            std::size_t NumberOfContinuations() const
            {
                return _state->NumberOfContinuations();
            }

        private:
            void Check() const
            {
                if (!_state)
                {
                    throw std::logic_error("Calling operations on an uninitialized object.");
                }
            }

            std::shared_ptr<detail::FutureState<T>> _state;
        };

        template<typename T>
        class Promise final
        {
        public:
            explicit Promise()
                : _state(std::make_shared<detail::FutureState<T>>())
            {

            }

            ~Promise() noexcept
            {
                try {
                    if (_state)
                    {
                        _state->AboutToDestroyPromise();
                        _state.reset();
                    }
                } catch(...)
                { }
            }

            Promise(Promise const&) = delete;
            Promise(Promise&&) = default;
            Promise& operator=(Promise const&) = delete;
            Promise& operator=(Promise&&) = default;

            bool Valid() const noexcept
            {
                return static_cast<bool>(_state);
            }

            template <typename F, typename std::enable_if<std::is_same_v<F, T> && !std::is_void_v<F>>::type* = nullptr>
            void SetValue(F const& value)
            {
                Check();
                _state->SetValue(value);
            }

            template <typename F = void, typename std::enable_if<std::is_void_v<F>>::type* = nullptr>
            void SetValue()
            {
                Check();
                _state->SetValue();
            }

            void SetException(std::exception_ptr const& ex)
            {
                Check();
                _state->SetException(ex);
            }
            
            Future<T> GetFuture() const
            {
                Check();
                return Future<T>(_state);
            }

            std::size_t NumberOfContinuations() const
            {
                return _state->NumberOfContinuations();
            }

        private:
            void Check() const
            {
                if (!_state)
                {
                    throw std::logic_error("Calling operations on an uninitialized object.");
                }
            }

            std::shared_ptr<detail::FutureState<T>> _state;
        };

        template <typename... TFutures>
        Future<void> WhenAll(TFutures&&... futures)
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

        template <typename... TFutures>
        Future<void> WhenAny(TFutures&&... futures)
        {
            auto sharedFutureState = std::make_shared<detail::FutureState<void>>();
            auto future = ::azul::async::Future<void>(sharedFutureState);
            auto func = [sharedFutureState](auto) mutable {
                sharedFutureState->SetValue();
            };

            (futures.Then(func),...);

            return future;
        }

        template <typename F1, typename F2>
        static Future<void> operator&&(F1&& future1, F2&& future2)
        {
            return WhenAll(future1, future2);
        }

        template <typename F1, typename F2>
        static Future<void> operator||(F1&& future1, F2&& future2)
        {
            return WhenAny(future1, future2);
        }

    }
}