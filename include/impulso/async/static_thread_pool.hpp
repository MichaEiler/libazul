#pragma once

#include <condition_variable>
#include <cstdint>
#include <impulso/async/detail/task.hpp>
#include <impulso/async/future.hpp>
#include <impulso/utils/disposer.hpp>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace impulso
{
    namespace async
    {
        class static_thread_pool
        {
        public:
            explicit static_thread_pool(const std::size_t number_of_threads)
            {
                std::unique_lock<std::mutex> lock(mutex_);

                for (std::uint32_t i = 0; i < number_of_threads; ++i)
                {
                    threads_.emplace_back([this](){
                        thread_loop();
                    });
                }
            }

            ~static_thread_pool()
            {
                shutdown_notify_threads();
                shutdown_join_threads();
                shutdown_process_remaining_tasks();
            }

            std::size_t thread_count() const
            {
                std::unique_lock<std::mutex> lock(mutex_);
                return threads_.size();
            }
        
            template<typename T, typename TResult=std::invoke_result_t<T>, typename... TFutures>
            impulso::async::future<TResult> execute(T&& callable, TFutures&&... dependencies)
            {
                std::unique_lock<std::mutex> lock(mutex_);

                const auto new_task = std::make_shared<detail::task_type<TResult>>(std::function<TResult()>(callable), impulso::async::detail::wrap_dependencies(dependencies...));
                tasks_.emplace_back(new_task);

                condition_.notify_one();

                return new_task->get_future();
            }


        private:            
            std::condition_variable condition_;
            mutable std::mutex mutex_;

            std::list<std::shared_ptr<impulso::async::detail::base_task_type>> tasks_;
            std::vector<std::thread> threads_;

            bool shutdown_initiated_ = false;

            std::shared_ptr<impulso::async::detail::base_task_type> next_task()
            {
                auto it = tasks_.begin();

                for (; it != tasks_.end(); ++it)
                {
                    if ((*it)->is_ready())
                    {
                        break;
                    }
                }

                if (it != tasks_.end())
                {
                    auto task = *it;
                    tasks_.erase(it);
                    return task;
                }

                return {};
            }

            void thread_loop()
            {
                std::unique_lock<std::mutex> lock(mutex_);

                while (!shutdown_initiated_)
                {
                    const auto task = next_task();
                    if (task)
                    {
                        lock.unlock();
                        task->operator()();
                        lock.lock();

                        // number of continuations equals the amount of tasks waiting
                        // for the just executed task to be completed
                        for (std::size_t i = 0; i < std::min(task->number_of_continuations(), threads_.size()); ++i)
                        {
                            condition_.notify_one();
                        }
                    }

                    if (!task && !shutdown_initiated_)
                    {
                        condition_.wait_for(lock, std::chrono::milliseconds(1000));
                    }
                }
            }

            void shutdown_notify_threads()
            {
                std::unique_lock<std::mutex> lock(mutex_);
                shutdown_initiated_ = true;
                condition_.notify_all();
            }

            void shutdown_join_threads()
            {
                std::for_each(threads_.begin(), threads_.end(), [](auto& t) { t.join(); });
            }

            void shutdown_process_remaining_tasks()
            {
                tasks_.clear();
            }
        };
    }
}