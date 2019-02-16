#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <impulso/async/fair_executor.hpp>
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
                explicit opencl_kernel_executor(std::shared_ptr<async::fair_executor> const& executor)
                    : executor_(executor)
                {
                    
                }

                std::future<void> execute(std::function<void()> && kernel, std::tuple<std::size_t> const& global_work_size, std::tuple<std::size_t> const& global_work_offset = { 0u })
                {
                    auto task_results = std::make_shared<std::vector<std::future<void>>>();

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
                        task_results->emplace_back(std::move(executor_->execute(task)));
                    }

                    return wait_for(std::move(task_results));
                }

                std::future<void> execute(std::function<void()> && kernel, std::tuple<std::size_t, std::size_t> const& global_work_size, std::tuple<std::size_t, std::size_t> const& global_work_offset = { 0u, 0u })
                {
                    auto task_results = std::make_shared<std::vector<std::future<void>>>();

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
                        task_results->emplace_back(std::move(executor_->execute(task)));
                    }

                    return wait_for(std::move(task_results));
                }

                std::future<void> execute(std::function<void()> && kernel, std::tuple<std::size_t, std::size_t, std::size_t> const& global_work_size, std::tuple<std::size_t, std::size_t, std::size_t> const& global_work_offset = { 0u, 0u, 0u })
                {
                    auto task_results = std::make_shared<std::vector<std::future<void>>>();

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
                        task_results->emplace_back(std::move(executor_->execute(task)));
                    }

                    return wait_for(std::move(task_results));
                }

            private:
                std::future<void> wait_for(std::shared_ptr<std::vector<std::future<void>>> futures)
                {
                    std::function<void()> completion_task = [futures = std::move(futures)]() {
                        // in case multiple tasks have thrown an exception, this function would only
                        // forward the first, since an opencl kernel will not throw an exception
                        // this won't be an issue
                        for (auto& future : *futures)
                        {
                            future.get();
                        }
                    };
                    
                    return std::move(executor_->execute(std::move(completion_task)));
                }

                std::shared_ptr<async::fair_executor> executor_;
            };
        }
    }
}