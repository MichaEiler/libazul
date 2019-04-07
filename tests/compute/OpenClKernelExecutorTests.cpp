#include <gmock/gmock.h>
#include <azul/async/StaticThreadPool.hpp>
#include <azul/compute/clcpp/OpenClComputeExecutor.hpp>
#include <azul/compute/clcpp/OpenCl.hpp>
#include <iostream>

class OpenClComputExecutorTestFixture : public testing::Test
{
};

TEST_F(OpenClComputExecutorTestFixture, KernelExecutor_WorkItemCountLarge_KernelCallsValid)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);

    const std::size_t width = 1920;
    const std::size_t height = 1080;
    std::vector<int> image(width * height, 0);

    const auto kernel = [&image, width]() {
        using namespace azul::compute::clcpp;
        image[get_global_id(1) * width + get_global_id(0)]++;
    };

    auto result = kernel_executor->Execute(kernel, { width, height });
    result.Get();

    for (std::size_t i = 0; i < image.size(); ++i)
    {
        if (image[i] != 1)
        {
            std::cerr << "Value of image at " << i << ", is " << image[i] << std::endl;
        }
        ASSERT_EQ(1, image[i]);
    }
}

TEST_F(OpenClComputExecutorTestFixture, Execute_WorkItemsWithOneDim_AllWorkItemsExecuted)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);

    std::vector<int> expected_result { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result]() {
        using namespace azul::compute::clcpp;
        result[get_global_id(0)]++;
    };

    auto future = kernel_executor->Execute(kernel, { 8u });
    future.Wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(OpenClComputExecutorTestFixture, Execute_WorkItemsWithOneDim_OffsetConsidered)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);

    std::vector<int> expected_result { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0};
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result]() {
        using namespace azul::compute::clcpp;
        result[get_global_id(0)]++;
    };

    auto future = kernel_executor->Execute(kernel, { 7u }, { 3u });
    future.Wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(OpenClComputExecutorTestFixture, Execute_WorkItemsWithTwoDims_AllWorkItemsExecuted)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);   

    std::vector<int> expected_result { 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result, width=4]() {
        using namespace azul::compute::clcpp;
        result[get_global_id(1) * width + get_global_id(0)]++;
    };

    auto future = kernel_executor->Execute(kernel, { 3u, 2u });
    future.Wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(OpenClComputExecutorTestFixture, Execute_WorkItemsWithTwoDims_OffsetConsidered)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);   

    std::vector<int> expected_result { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 };
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result, width=4]() {
        using namespace azul::compute::clcpp;
        result[get_global_id(1) * width + get_global_id(0)]++;
    };

    auto future = kernel_executor->Execute(kernel, { 2u, 1u }, { 1u, 2u });
    future.Wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(OpenClComputExecutorTestFixture, Execute_WorkItemsWithThreeDims_AllWorkItemsExecuted)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);   

    // prepare expected result, a cuboid in the lower left corner with width 4, depth 3 and height 2
    const int edge = 5;
    std::vector<int> expected_result{
            1, 1, 1, 1, 0,
            1, 1, 1, 1, 0,
            1, 1, 1, 1, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            1, 1, 1, 1, 0,
            1, 1, 1, 1, 0,
            1, 1, 1, 1, 0
        };
    expected_result.resize(edge * edge * edge, 0);

    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result, edge]() {
        using namespace azul::compute::clcpp;
        result[(get_global_id(2) * edge + get_global_id(1))* edge + get_global_id(0)]++;
    };

    auto future = kernel_executor->Execute(kernel, { 4u, 3u, 2u });
    future.Wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(OpenClComputExecutorTestFixture, Execute_WorkItemsWithThreeDims_OffsetConsidered)
{
    const auto executor = std::make_shared<azul::async::StaticThreadPool>(5);
    const auto kernel_executor = std::make_shared<azul::compute::clcpp::OpenClComputeExecutor>(executor);   

    // prepare expected result, nested cubes
    const int edge = 5;
    std::vector<int> expected_result{
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,

            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 1, 1, 0, 
            0, 0, 1, 1, 0,
            0, 0, 0, 0, 0,

            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 1, 1, 0,
            0, 0, 1, 1, 0,
            0, 0, 0, 0, 0
        };
    expected_result.resize(edge * edge * edge, 0);

    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result, edge]() {
        using namespace azul::compute::clcpp;
        result[(get_global_id(2) * edge + get_global_id(1))* edge + get_global_id(0)]++;
    };

    auto future = kernel_executor->Execute(kernel, { 2u, 2u, 2u }, { 2u, 2u, 1u });
    future.Wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

