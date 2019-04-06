#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <impulso/async/static_thread_pool.hpp>
#include <impulso/compute/clcpp/opencl.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

namespace impulso
{
    namespace compute
    {
        namespace clcpp
        {
            class opencl_kernel_executor
            {
            public:
                explicit opencl_kernel_executor(std::shared_ptr<async::static_thread_pool> const& executor)
                    : executor_(executor)
                {
                    
                }

                impulso::async::future<void> execute(std::function<void()> && kernel, std::tuple<std::size_t> const& global_work_size, std::tuple<std::size_t> const& global_work_offset = { 0u })
                {
                    std::vector<impulso::async::future<void>> task_results;

                    const auto work_items = std::get<0>(global_work_size);
                    const auto work_items_per_task = std::max<std::size_t>(work_items / executor_->thread_count(), 1ul);

                    for (std::size_t i = 0; i < work_items; i += work_items_per_task)
                    {
                        const auto work_items_to_process = std::min(work_items_per_task, work_items - i);
                        const auto work_items_offset = std::get<0>(global_work_offset) + i;
                        
                        auto task = [kernel, work_items_offset, work_items_to_process]() {
                            for (std::size_t j = work_items_offset; j < work_items_offset + work_items_to_process; ++j)
                            {
                                set_global_id(0, j);
                                kernel();
                            }
                        };
                        task_results.emplace_back(executor_->execute(task));
                    }

                    return wait_for(task_results);
                }

                impulso::async::future<void> execute(std::function<void()> && kernel, std::tuple<std::size_t, std::size_t> const& global_work_size, std::tuple<std::size_t, std::size_t> const& global_work_offset = { 0u, 0u })
                {
                    std::vector<impulso::async::future<void>> task_results;

                    const auto work_items  = std::get<0>(global_work_size) * std::get<1>(global_work_size);
                    const auto work_items_per_task = std::max<std::size_t>(work_items / executor_->thread_count(), 1ul);

                    for (std::size_t i = 0; i < work_items; i += work_items_per_task)
                    {
                        const auto work_item_count = std::min(work_items_per_task, work_items - i);
                        auto task = [kernel, global_work_offset, work_item_count, size_x = std::get<0>(global_work_size), work_item_start=i]() {
                            for (std::size_t j = work_item_start; j < work_item_start + work_item_count; ++j)
                            {
                                set_global_id(0, std::get<0>(global_work_offset) + j % size_x);
                                set_global_id(1, std::get<1>(global_work_offset) + j / size_x);
                                kernel();
                            }
                        };
                        task_results.emplace_back(executor_->execute(task));
                    }

                    return wait_for(task_results);
                }

                impulso::async::future<void> execute(std::function<void()> && kernel, std::tuple<std::size_t, std::size_t, std::size_t> const& global_work_size, std::tuple<std::size_t, std::size_t, std::size_t> const& global_work_offset = { 0u, 0u, 0u })
                {
                    std::vector<impulso::async::future<void>> task_results;

                    const auto work_items  = std::get<0>(global_work_size) * std::get<1>(global_work_size) * std::get<2>(global_work_size);
                    const auto work_items_per_task = std::max<std::size_t>(work_items / executor_->thread_count(), 1ul);

                    for (std::size_t i = 0; i < work_items; i += work_items_per_task)
                    {
                        const auto work_item_count = std::min(work_items_per_task, work_items - i);
                        auto task = [kernel, global_work_offset, global_work_size, work_item_count, work_item_start = i]() {
                            for (std::size_t j = work_item_start; j < work_item_start + work_item_count; ++j)
                            {
                                set_global_id(2, std::get<2>(global_work_offset) + j / (std::get<0>(global_work_size) * std::get<1>(global_work_size)));
                                set_global_id(1, std::get<1>(global_work_offset) + (j % (std::get<0>(global_work_size) * std::get<1>(global_work_size))) / std::get<0>(global_work_size));
                                set_global_id(0, std::get<0>(global_work_offset) + j % std::get<0>(global_work_size));
                                kernel();
                            }
                        };
                        task_results.emplace_back(executor_->execute(task));
                    }

                    return wait_for(task_results);
                }

            private:
                impulso::async::future<void> wait_for(std::vector<impulso::async::future<void>>& futures)
                {
                    auto shared_future_state = std::make_shared<impulso::async::detail::future_state<void>>();
                    auto future = ::impulso::async::future<void>(shared_future_state);

                    auto shared_promise_activator = std::make_shared<impulso::utils::disposer>([shared_future_state]() {
                        shared_future_state->set();
                    });

                    auto func = [shared_promise_activator](auto) mutable {
                        shared_promise_activator.reset();
                    };

                    for (auto& f : futures)
                    {
                        f.then(func);
                    }

                    return future;
                }

                std::shared_ptr<async::static_thread_pool> executor_;
            };
        }
    }
}