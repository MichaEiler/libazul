#include <gmock/gmock.h>
#include <azul/compute/clcpp/opencl_vector.hpp>

using int2 = azul::compute::clcpp::Vec2<std::int32_t>;

class OpenClVec2TestFixture : public testing::Test
{
};

TEST_F(OpenClVec2TestFixture, Operator_SubtractionAssignment_ResultValid)
{
    int2 valueA( 1,  5);
    int2 valueB( 3, 10);

    valueA -= valueB;

    const int2 expectedResult( -2, -5);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_AdditionAssignment_ResultValid)
{
    int2 valueA( 1,  5);
    int2 valueB( 3, 10);

    valueA += valueB;

    const int2 expectedResult( 4, 15);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_MultiplicationAssignment_ResultValid)
{
    int2 valueA( 1,  5);
    std::int32_t valueB(4);

    valueA *= valueB;

    const int2 expectedResult( 4, 20);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_DivisionAssignment_ResultValid)
{
    int2 valueA( 16,  24);
    std::int32_t valueB(4);

    valueA /= valueB;

    const int2 expectedResult( 4, 6);
    ASSERT_EQ(valueA.x, expectedResult.x);
    ASSERT_EQ(valueA.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_Addition_ResultValid)
{
    int2 valueA( 16,  24);
    int2 valueB(  1,   2);

    const auto result = valueA + valueB;

    const int2 expectedResult( 17, 26);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_Subtraction_ResultValid)
{
    int2 valueA( 16,  24);
    int2 valueB(  1,   2);

    const auto result = valueA - valueB;

    const int2 expectedResult( 15, 22);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_Multiplication_ResultValid)
{
    int2 valueA( 1,  5);
    std::int32_t valueB(4);

    const auto result = valueA * valueB;

    const int2 expectedResult( 4, 20);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_Division_ResultValid)
{
    int2 valueA( 16,  24);
    std::int32_t valueB(4);

    const auto result = valueA / valueB;

    const int2 expectedResult( 4, 6);
    ASSERT_EQ(result.x, expectedResult.x);
    ASSERT_EQ(result.y, expectedResult.y);
}

TEST_F(OpenClVec2TestFixture, Operator_Equal_CorrectResult)
{
    const int2 valueA( 1, 2);
    const int2 valueB( 1, 2);
    const int2 valueC( 2, 3);

    ASSERT_TRUE(valueA == valueB);
    ASSERT_FALSE(valueA == valueC);
}

TEST_F(OpenClVec2TestFixture, Operator_NotEqual_CorrectResult)
{
    const int2 valueA( 1, 2);
    const int2 valueB( 1, 2);
    const int2 valueC( 2, 3);

    ASSERT_FALSE(valueA != valueB);
    ASSERT_TRUE(valueA != valueC);
}

