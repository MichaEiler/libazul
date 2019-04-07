#pragma once

#include <condition_variable>
#include <cstdint>
#include <azul/async/future.hpp>
#include <azul/async/task.hpp>
#include <azul/utils/disposer.hpp>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace azul
{
    namespace async
    {
        class StaticThreadPool
        {
        public:
            explicit StaticThreadPool(const std::size_t numberOfThreads)
            {
                std::unique_lock<std::mutex> lock(_mutex);

                for (std::uint32_t i = 0; i < numberOfThreads; ++i)
                {
                    _threads.emplace_back([this](){
                        ThreadLoop();
                    });
                }
            }

            ~StaticThreadPool()
            {
                ShutdownNotifyThreads();
                ShutdownJoinThreads();
                ShutdownDestroyRemainingTasks();
            }

            std::size_t ThreadCount() const
            {
                std::unique_lock<std::mutex> lock(_mutex);
                return _threads.size();
            }
        
            template<typename T, typename TResult=std::invoke_result_t<T>, typename... TFutures>
            Future<TResult> Execute(T&& callable, TFutures&&... dependencies)
            {
                std::unique_lock<std::mutex> lock(_mutex);

                const auto newTask = std::make_shared<Task<TResult>>(std::function<TResult()>(callable), azul::async::WrapDependencies(dependencies...));
                _tasks.emplace_back(newTask);

                _condition.notify_one();

                return newTask->GetFuture();
            }


        private:            
            std::condition_variable _condition;
            mutable std::mutex _mutex;

            std::list<std::shared_ptr<azul::async::TaskBase>> _tasks;
            std::vector<std::thread> _threads;

            bool _shutdownInitiated = false;

            std::shared_ptr<azul::async::TaskBase> NextTask()
            {
                auto it = _tasks.begin();

                for (; it != _tasks.end(); ++it)
                {
                    if ((*it)->IsReady())
                    {
                        break;
                    }
                }

                if (it != _tasks.end())
                {
                    auto task = *it;
                    _tasks.erase(it);
                    return task;
                }

                return {};
            }

            void ThreadLoop()
            {
                std::unique_lock<std::mutex> lock(_mutex);

                while (!_shutdownInitiated)
                {
                    const auto task = NextTask();
                    if (task)
                    {
                        lock.unlock();
                        task->operator()();
                        lock.lock();

                        // number of continuations equals the amount of tasks waiting
                        // for the just executed task to be completed
                        for (std::size_t i = 0; i < std::min(task->NumberOfContinuations(), _threads.size()); ++i)
                        {
                            _condition.notify_one();
                        }
                    }

                    if (!task && !_shutdownInitiated)
                    {
                        _condition.wait_for(lock, std::chrono::milliseconds(1000));
                    }
                }
            }

            void ShutdownNotifyThreads()
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _shutdownInitiated = true;
                _condition.notify_all();
            }

            void ShutdownJoinThreads()
            {
                std::for_each(_threads.begin(), _threads.end(), [](auto& t) { t.join(); });
            }

            void ShutdownDestroyRemainingTasks()
            {
                _tasks.clear();
            }
        };
    }
}