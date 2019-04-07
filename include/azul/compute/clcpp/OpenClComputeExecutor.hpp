#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <azul/async/StaticThreadPool.hpp>
#include <azul/compute/clcpp/OpenCl.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

namespace azul
{
    namespace compute
    {
        namespace clcpp
        {
            class OpenClComputeExecutor
            {
            public:
                explicit OpenClComputeExecutor(std::shared_ptr<async::StaticThreadPool> const& executor)
                    : _executor(executor)
                {
                    
                }

                azul::async::Future<void> Execute(std::function<void()> && kernel, std::tuple<std::size_t> const& globalWorkSize, std::tuple<std::size_t> const& globalWorkOffset = { 0u })
                {
                    std::vector<azul::async::Future<void>> taskResults;

                    const auto workItems = std::get<0>(globalWorkSize);
                    const auto workItemsPerTask = std::max<std::size_t>(workItems / _executor->ThreadCount(), 1ul);

                    for (std::size_t i = 0; i < workItems; i += workItemsPerTask)
                    {
                        const auto workItemsToProcess = std::min(workItemsPerTask, workItems - i);
                        const auto workItemOffset = std::get<0>(globalWorkOffset) + i;
                        
                        auto task = [kernel, workItemOffset, workItemsToProcess]() {
                            for (std::size_t j = workItemOffset; j < workItemOffset + workItemsToProcess; ++j)
                            {
                                set_global_id(0, j);
                                kernel();
                            }
                        };
                        taskResults.emplace_back(_executor->Execute(task));
                    }

                    return WaitFor(taskResults);
                }

                azul::async::Future<void> Execute(std::function<void()> && kernel, std::tuple<std::size_t, std::size_t> const& globalWorkSize, std::tuple<std::size_t, std::size_t> const& globalWorkOffset = { 0u, 0u })
                {
                    std::vector<azul::async::Future<void>> taskResults;

                    const auto workItems  = std::get<0>(globalWorkSize) * std::get<1>(globalWorkSize);
                    const auto workItemsPerTask = std::max<std::size_t>(workItems / _executor->ThreadCount(), 1ul);

                    for (std::size_t i = 0; i < workItems; i += workItemsPerTask)
                    {
                        const auto work_item_count = std::min(workItemsPerTask, workItems - i);
                        auto task = [kernel, globalWorkOffset, work_item_count, size_x = std::get<0>(globalWorkSize), work_item_start=i]() {
                            for (std::size_t j = work_item_start; j < work_item_start + work_item_count; ++j)
                            {
                                set_global_id(0, std::get<0>(globalWorkOffset) + j % size_x);
                                set_global_id(1, std::get<1>(globalWorkOffset) + j / size_x);
                                kernel();
                            }
                        };
                        taskResults.emplace_back(_executor->Execute(task));
                    }

                    return WaitFor(taskResults);
                }

                azul::async::Future<void> Execute(std::function<void()> && kernel, std::tuple<std::size_t, std::size_t, std::size_t> const& globalWorkSize, std::tuple<std::size_t, std::size_t, std::size_t> const& globalWorkOffset = { 0u, 0u, 0u })
                {
                    std::vector<azul::async::Future<void>> taskResults;

                    const auto workItems  = std::get<0>(globalWorkSize) * std::get<1>(globalWorkSize) * std::get<2>(globalWorkSize);
                    const auto workItemsPerTask = std::max<std::size_t>(workItems / _executor->ThreadCount(), 1ul);

                    for (std::size_t i = 0; i < workItems; i += workItemsPerTask)
                    {
                        const auto work_item_count = std::min(workItemsPerTask, workItems - i);
                        auto task = [kernel, globalWorkOffset, globalWorkSize, work_item_count, work_item_start = i]() {
                            for (std::size_t j = work_item_start; j < work_item_start + work_item_count; ++j)
                            {
                                set_global_id(2, std::get<2>(globalWorkOffset) + j / (std::get<0>(globalWorkSize) * std::get<1>(globalWorkSize)));
                                set_global_id(1, std::get<1>(globalWorkOffset) + (j % (std::get<0>(globalWorkSize) * std::get<1>(globalWorkSize))) / std::get<0>(globalWorkSize));
                                set_global_id(0, std::get<0>(globalWorkOffset) + j % std::get<0>(globalWorkSize));
                                kernel();
                            }
                        };
                        taskResults.emplace_back(_executor->Execute(task));
                    }

                    return WaitFor(taskResults);
                }

            private:
                azul::async::Future<void> WaitFor(std::vector<azul::async::Future<void>>& futures)
                {
                    auto sharedFutureState = std::make_shared<azul::async::detail::FutureState<void>>();
                    auto future = ::azul::async::Future<void>(sharedFutureState);

                    auto sharedPromiseActivator = std::make_shared<azul::utils::Disposer>([sharedFutureState]() {
                        sharedFutureState->SetValue();
                    });

                    auto func = [sharedPromiseActivator](auto) mutable {
                        sharedPromiseActivator.reset();
                    };

                    for (auto& f : futures)
                    {
                        f.Then(func);
                    }

                    return future;
                }

                std::shared_ptr<async::StaticThreadPool> _executor;
            };
        }
    }
}