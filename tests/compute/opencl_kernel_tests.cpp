#include <gmock/gmock.h>

#include <azul/compute/clcpp/opencl_kernel_pre.hpp>
#include "kernels/math_builtins.cl"
#include "kernels/vec_add.cl"
#include "kernels/vec_types.cl"
#include <azul/compute/clcpp/opencl_kernel_post.hpp>



class OpenClKernelTestFixture : public testing::Test
{
};

TEST_F(OpenClKernelTestFixture, KernelVecAdd_Executed_ResultValid)
{
    float input[3]{ 1.0f, 3.0f, 5.0f };
    const float expectedResult[3]{ 2.0f, 6.0f, 10.0f };
    float result[3]{ 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < 3; ++i)
    {
        azul::compute::clcpp::set_global_id(0, static_cast<std::size_t>(i));
        clcpp::kernel_vec_add(input, input, result);
    }
    
    for (int i = 0; i < 3; ++i)
    {
        ASSERT_THAT(result[i], testing::FloatNear(expectedResult[i], 0.000001f));
    }    
}

TEST_F(OpenClKernelTestFixture, KernelMathBuiltIns_CompilationCheck_NoWarningsShown)
{
    float input = 1.0f;
    azul::compute::clcpp::set_global_id(0, 0);
    clcpp::kernel_math_builtins(&input);
}

TEST_F(OpenClKernelTestFixture, KernelVecTypes_CompilationCheck_Float4Supported)
{
    float input = 1.0f;
    azul::compute::clcpp::set_global_id(0, 0);
    clcpp::kernel_vec_types(&input);
}
