#include <gmock/gmock.h>
#include <impulso/async/static_thread_pool.hpp>
#include <impulso/compute/clcpp/opencl_kernel_executor.hpp>
#include <impulso/compute/clcpp/opencl.hpp>
#include <iostream>

class kernel_executor_fixture : public testing::Test
{
};

TEST_F(kernel_executor_fixture, kernelExecutor_workItemCountLarge_kernelCallsValid)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);

    const std::size_t width = 1920;
    const std::size_t height = 1080;
    std::vector<int> image(width * height, 0);

    const auto kernel = [&image, width]() {
        using namespace impulso::compute::clcpp;
        image[get_global_id(1) * width + get_global_id(0)]++;
    };

    auto result = kernel_executor->execute(kernel, { width, height });
    result.get();

    for (std::size_t i = 0; i < image.size(); ++i)
    {
        if (image[i] != 1)
        {
            std::cerr << "Value of image at " << i << ", is " << image[i] << std::endl;
        }
        ASSERT_EQ(1, image[i]);
    }
}

TEST_F(kernel_executor_fixture, execute_workItemsWithOneDim_allWorkItemsExecuted)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);

    std::vector<int> expected_result { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result]() {
        using namespace impulso::compute::clcpp;
        result[get_global_id(0)]++;
    };

    auto future = kernel_executor->execute(kernel, { 8u });
    future.wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(kernel_executor_fixture, execute_workItemsWithOneDim_offsetConsidered)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);

    std::vector<int> expected_result { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0};
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result]() {
        using namespace impulso::compute::clcpp;
        result[get_global_id(0)]++;
    };

    auto future = kernel_executor->execute(kernel, { 7u }, { 3u });
    future.wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(kernel_executor_fixture, execute_workItemsWithTwoDims_allWorkItemsExecuted)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);   

    std::vector<int> expected_result { 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result, width=4]() {
        using namespace impulso::compute::clcpp;
        result[get_global_id(1) * width + get_global_id(0)]++;
    };

    auto future = kernel_executor->execute(kernel, { 3u, 2u });
    future.wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(kernel_executor_fixture, execute_workItemsWithTwoDims_offsetConsidered)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);   

    std::vector<int> expected_result { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 };
    std::vector<int> result(expected_result.size(), 0);

    const auto kernel = [&result, width=4]() {
        using namespace impulso::compute::clcpp;
        result[get_global_id(1) * width + get_global_id(0)]++;
    };

    auto future = kernel_executor->execute(kernel, { 2u, 1u }, { 1u, 2u });
    future.wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(kernel_executor_fixture, execute_workItemsWithThreeDims_allWorkItemsExecuted)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);   

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
        using namespace impulso::compute::clcpp;
        result[(get_global_id(2) * edge + get_global_id(1))* edge + get_global_id(0)]++;
    };

    auto future = kernel_executor->execute(kernel, { 4u, 3u, 2u });
    future.wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

TEST_F(kernel_executor_fixture, execute_workItemsWithThreeDims_offsetConsidered)
{
    const auto executor = std::make_shared<impulso::async::static_thread_pool>(5);
    const auto kernel_executor = std::make_shared<impulso::compute::clcpp::opencl_kernel_executor>(executor);   

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
        using namespace impulso::compute::clcpp;
        result[(get_global_id(2) * edge + get_global_id(1))* edge + get_global_id(0)]++;
    };

    auto future = kernel_executor->execute(kernel, { 2u, 2u, 2u }, { 2u, 2u, 1u });
    future.wait();

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i], expected_result[i]);
    }
}

