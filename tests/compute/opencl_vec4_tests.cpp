#include <gmock/gmock.h>
#include <impulso/compute/clcpp/opencl_vector.hpp>

using int4 = impulso::compute::clcpp::vec4<std::int32_t>;

class compute_clcpp_vec4_fixture : public testing::Test
{
};

TEST_F(compute_clcpp_vec4_fixture, operator_subtractionAssignment_resultValid)
{
    int4 valueA( 1,  5, 10, 50);
    int4 valueB( 3, 10, 47, 99);

    valueA -= valueB;

    const int4 expectedResult( -2, -5, -37, -49);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
    ASSERT_EQ(valueA.z, expectedResult.z);
    ASSERT_EQ(valueA.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_additionAssignment_resultValid)
{
    int4 valueA( 1,  5, 10, 50);
    int4 valueB( 3, 10, 47, 99);

    valueA += valueB;

    const int4 expectedResult( 4, 15, 57, 149);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
    ASSERT_EQ(valueA.z, expectedResult.z);
    ASSERT_EQ(valueA.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_multiplicationAssignment_resultValid)
{
    int4 valueA( 1,  5, 10, 50);
    std::int32_t valueB(4);

    valueA *= valueB;

    const int4 expectedResult( 4, 20, 40, 200);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
    ASSERT_EQ(valueA.z, expectedResult.z);
    ASSERT_EQ(valueA.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_divisionAssignment_resultValid)
{
    int4 valueA( 16,  24, 36, 56);
    std::int32_t valueB(4);

    valueA /= valueB;

    const int4 expectedResult( 4, 6, 9, 14);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
    ASSERT_EQ(valueA.z, expectedResult.z);
    ASSERT_EQ(valueA.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_addition_resultValid)
{
    int4 valueA( 16,  24, 36, 56);
    int4 valueB(  1,   2,  3,  4);

    const auto result = valueA + valueB;

    const int4 expectedResult( 17, 26, 39, 60);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
    ASSERT_EQ(result.z, expectedResult.z);
    ASSERT_EQ(result.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_subtraction_resultValid)
{
    int4 valueA( 16,  24, 36, 56);
    int4 valueB(  1,   2,  3,  4);

    const auto result = valueA - valueB;

    const int4 expectedResult( 15, 22, 33, 52);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
    ASSERT_EQ(result.z, expectedResult.z);
    ASSERT_EQ(result.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_multiplication_resultValid)
{
    int4 valueA( 1,  5, 10, 50);
    std::int32_t valueB(4);

    const auto result = valueA * valueB;

    const int4 expectedResult( 4, 20, 40, 200);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
    ASSERT_EQ(result.z, expectedResult.z);
    ASSERT_EQ(result.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_division_resultValid)
{
    int4 valueA( 16,  24, 36, 56);
    std::int32_t valueB(4);

    const auto result = valueA / valueB;

    const int4 expectedResult( 4, 6, 9, 14);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
    ASSERT_EQ(result.z, expectedResult.z);
    ASSERT_EQ(result.w, expectedResult.w);
}

TEST_F(compute_clcpp_vec4_fixture, operator_equal_correctResult)
{
    const int4 valueA( 1, 2, 3, 4);
    const int4 valueB( 1, 2, 3, 4);
    const int4 valueC( 2, 3, 4, 5);

    ASSERT_TRUE(valueA == valueB);
    ASSERT_FALSE(valueA == valueC);
}

TEST_F(compute_clcpp_vec4_fixture, operator_notEqual_correctResult)
{
    const int4 valueA( 1, 2, 3, 4);
    const int4 valueB( 1, 2, 3, 4);
    const int4 valueC( 2, 3, 4, 5);

    ASSERT_FALSE(valueA != valueB);
    ASSERT_TRUE(valueA != valueC);
}
