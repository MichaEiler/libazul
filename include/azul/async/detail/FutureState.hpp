#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace azul
{
    namespace async
    {
        enum class FutureErrorCode : std::uint32_t
        {
            BrokenPromise = 0,
        };

        class FutureError : public std::exception
        {
        public:
            explicit FutureError(FutureErrorCode const code)
                : _code(code)
            {
                
            }
            
            FutureErrorCode ErrorCode() const
            {
                return _code;
            }

        private:
            FutureErrorCode _code;
        };

        namespace detail
        {

            template <typename T>
            class FutureState
            {
            public:
                explicit FutureState()
                {

                }

                FutureState(FutureState const&) = delete;
                FutureState(FutureState&&) = delete;
                FutureState& operator=(FutureState const&) = delete;
                FutureState& operator=(FutureState&&) = delete;

                bool IsReady() const
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    return _state != State::Undefined;
                }

                const T& Get()
                {
                    std::unique_lock<std::mutex> lock(_mutex);

                    for(;;)
                    {
                        switch (_state)
                        {
                        case State::Exception:
                            std::rethrow_exception(_exception);
                        case State::BrokenPromise:
                            throw azul::async::FutureError(azul::async::FutureErrorCode::BrokenPromise);
                        case State::Ready:
                            return _value;
                        case State::Undefined:
                            _condition.wait(lock);
                            break;
                        }
                    }
                }

                void SetValue(T const& value)
                {
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        _value = value;
                        _state = State::Ready;
                        _condition.notify_all();
                    }

                    for (const auto& continuation : _continuations)
                    {
                        continuation();
                    }
                }

                void SetException(std::exception_ptr const& ex)
                {
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        _exception = ex;
                        _state = State::Exception;
                        _condition.notify_all();
                    }

                    for (const auto& continuation : _continuations)
                    {
                        continuation();
                    }
                }

                void Wait()
                {
                    std::unique_lock<std::mutex> lock(_mutex);

                    if (_state == State::Undefined)
                    {
                        _condition.wait(lock);
                    }
                }

                template<class Rep, class Period>
                bool WaitFor(std::chrono::duration<Rep,Period> const& timeoutDuration) const
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_state == State::Undefined)
                    {
                        return _condition.wait_for(lock, timeoutDuration) != std::cv_status::timeout;
                    }
                    return true;
                }

                void Then(std::function<void()> const& continuation)
                {
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        if (_state == State::BrokenPromise)
                        {
                            return;
                        }

                        if (_state == State::Undefined)
                        {
                            _continuations.emplace_back(continuation);
                            return;
                        }
                    }

                    continuation();                    
                }

                std::size_t NumberOfContinuations() const
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    return _continuations.size();
                }

                void AboutToDestroyPromise()
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    if (_state == State::Undefined)
                    {
                        _state = State::BrokenPromise;
                    }
                    _condition.notify_all();
                }

            private:
                mutable std::condition_variable _condition;
                mutable std::mutex _mutex;

                enum class State
                {
                    Undefined = 0,
                    Ready = 1,
                    Exception = 2,
                    BrokenPromise = 3,
                };

                State _state{ State::Undefined };
                T _value{ };
                std::exception_ptr _exception{ };

                std::vector<std::function<void()>> _continuations;
            };

            template <>
            class FutureState<void>
            {
            public:
                explicit FutureState()
                {

                }

                FutureState(FutureState const& other) = delete;
                FutureState(FutureState&& other) = delete;
                FutureState& operator=(FutureState const& other) = delete;
                FutureState& operator=(FutureState&& other) = delete;

                bool IsReady() const
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    return _state != State::Undefined;
                }

                void Get()
                {
                    std::unique_lock<std::mutex> lock(_mutex);

                    for(;;)
                    {
                        switch (_state)
                        {
                        case State::Exception:
                            std::rethrow_exception(_exception);
                        case State::BrokenPromise:
                            throw azul::async::FutureError(azul::async::FutureErrorCode::BrokenPromise);
                        case State::Ready:
                            return;
                        case State::Undefined:
                            _condition.wait(lock);
                            break;
                        }
                    }
                }

                void SetValue()
                {
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        _state = State::Ready;
                        _condition.notify_all();
                    }

                    for (const auto& continuation : _continuations)
                    {
                        continuation();
                    }
                }

                void SetException(std::exception_ptr const& ex)
                {
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        _exception = ex;
                        _state = State::Exception;
                        _condition.notify_all();
                    }

                    for (const auto& continuation : _continuations)
                    {
                        continuation();
                    }
                }
            

                void Wait()
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_state == State::Undefined)
                    {
                        _condition.wait(lock);
                    }
                }

                template<class Rep, class Period>
                bool wait_for(std::chrono::duration<Rep,Period> const& timeoutDuration) const
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_state == State::Undefined)
                    {
                        return _condition.wait_for(lock, timeoutDuration) != std::cv_status::timeout;
                    }
                    return true;
                }

                void Then(std::function<void()> const& continuation)
                {
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        if (_state == State::BrokenPromise)
                        {
                            return;
                        }

                        if (_state == State::Undefined)
                        {
                            _continuations.emplace_back(continuation);
                            return;
                        }
                    }

                    continuation();                    
                }

                std::size_t NumberOfContinuations() const
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    return _continuations.size();
                }

                void AboutToDestroyPromise()
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    if (_state == State::Undefined)
                    {
                        _state = State::BrokenPromise;
                    }
                    _condition.notify_all();
                }

            private:
                mutable std::condition_variable _condition;
                mutable std::mutex _mutex;

                enum class State
                {
                    Undefined = 0,
                    Ready = 1,
                    Exception = 2,
                    BrokenPromise = 3,
                };

                State _state{ State::Undefined };
                std::exception_ptr _exception{ };

                std::vector<std::function<void()>> _continuations;
            };
        }
    }
}