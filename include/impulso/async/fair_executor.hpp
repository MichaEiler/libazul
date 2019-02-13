#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <future>
#include <impulso/async/task.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace impulso
{
    namespace async
    {
        class fair_executor final
        {
        public:
            explicit fair_executor()
            {
            }

            ~fair_executor()
            {
                shutdown_notify_threads();
                shutdown_join_threads();
                shutdown_process_remaining_tasks();
            }

            void add_threads(std::uint32_t const& number_of_new_threads = 1)
            {
                std::unique_lock<std::mutex> lock(executor_mutex_);

                for (std::uint32_t i = 0; i < number_of_new_threads; ++i)
                {
                    threads_.emplace_back([this](){
                        thread_loop();
                    });
                }
            }

            std::uint32_t thread_count() const
            {
                std::unique_lock<std::mutex> lock(executor_mutex_);
                return static_cast<std::uint32_t>(threads_.size());
            }

            template<typename TResult>
            std::future<TResult> execute(std::function<TResult()> && func)
            {
                std::unique_lock<std::mutex> lock(executor_mutex_);

                const auto new_task = std::make_shared<task_type<TResult>>(std::move(func));

                tasks_.emplace(new_task);
                tasks_condition_.notify_one();

                return new_task->get_future();
            }

            std::future<void> execute(std::function<void()> && func)
            {
                std::unique_lock<std::mutex> lock(executor_mutex_);

                const auto new_task = std::make_shared<task_type<void>>(std::move(func));

                tasks_.emplace(new_task);
                tasks_condition_.notify_one();

                return new_task->get_future();
            }

        private:
            mutable std::mutex executor_mutex_;
            std::vector<std::thread> threads_;
    

            std::queue<std::shared_ptr<base_task_type>> tasks_;
            std::condition_variable tasks_condition_;
            bool shutdown_initiated_ = false;

            void thread_loop()
            {
                std::unique_lock<std::mutex> lock(executor_mutex_);

                while (!shutdown_initiated_)
                {
                    while (!shutdown_initiated_ && tasks_.size() == 0)
                    {
                        tasks_condition_.wait_for(lock, std::chrono::milliseconds(100));
                    }

                    if (shutdown_initiated_)
                    {
                        return;
                    }

                    const auto next_task = tasks_.front();
                    tasks_.pop();

                    lock.unlock();
                    next_task->operator()();
                    lock.lock();
                }
            }

            void shutdown_notify_threads()
            {
                std::unique_lock<std::mutex> lock(executor_mutex_);
                shutdown_initiated_ = true;
                tasks_condition_.notify_all();
            }

            void shutdown_join_threads()
            {
                std::for_each(threads_.begin(), threads_.end(), [](auto& t) { t.join(); });
            }

            void shutdown_process_remaining_tasks()
            {
                while (tasks_.size() > 0)
                {
                    tasks_.pop();
                }
            }
        };
    }
}
