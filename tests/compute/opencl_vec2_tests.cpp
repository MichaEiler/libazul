#include <gmock/gmock.h>
#include <azul/compute/clcpp/opencl_vector.hpp>

using int2 = azul::compute::clcpp::vec2<std::int32_t>;

class compute_clcpp_vec2_fixture : public testing::Test
{
};

TEST_F(compute_clcpp_vec2_fixture, operator_subtractionAssignment_resultValid)
{
    int2 valueA( 1,  5);
    int2 valueB( 3, 10);

    valueA -= valueB;

    const int2 expectedResult( -2, -5);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_additionAssignment_resultValid)
{
    int2 valueA( 1,  5);
    int2 valueB( 3, 10);

    valueA += valueB;

    const int2 expectedResult( 4, 15);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_multiplicationAssignment_resultValid)
{
    int2 valueA( 1,  5);
    std::int32_t valueB(4);

    valueA *= valueB;

    const int2 expectedResult( 4, 20);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_divisionAssignment_resultValid)
{
    int2 valueA( 16,  24);
    std::int32_t valueB(4);

    valueA /= valueB;

    const int2 expectedResult( 4, 6);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_addition_resultValid)
{
    int2 valueA( 16,  24);
    int2 valueB(  1,   2);

    const auto result = valueA + valueB;

    const int2 expectedResult( 17, 26);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_subtraction_resultValid)
{
    int2 valueA( 16,  24);
    int2 valueB(  1,   2);

    const auto result = valueA - valueB;

    const int2 expectedResult( 15, 22);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_multiplication_resultValid)
{
    int2 valueA( 1,  5);
    std::int32_t valueB(4);

    const auto result = valueA * valueB;

    const int2 expectedResult( 4, 20);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_division_resultValid)
{
    int2 valueA( 16,  24);
    std::int32_t valueB(4);

    const auto result = valueA / valueB;

    const int2 expectedResult( 4, 6);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(compute_clcpp_vec2_fixture, operator_equal_correctResult)
{
    const int2 valueA( 1, 2);
    const int2 valueB( 1, 2);
    const int2 valueC( 2, 3);

    ASSERT_TRUE(valueA == valueB);
    ASSERT_FALSE(valueA == valueC);
}

TEST_F(compute_clcpp_vec2_fixture, operator_notEqual_correctResult)
{
    const int2 valueA( 1, 2);
    const int2 valueB( 1, 2);
    const int2 valueC( 2, 3);

    ASSERT_FALSE(valueA != valueB);
    ASSERT_TRUE(valueA != valueC);
}

