#include "azul/utils/disposer.hpp"
#include <gmock/gmock.h>

class disposer_fixture : public testing::Test
{
};

TEST_F(disposer_fixture, destructor_initializedDisposer_calledOnce)
{
    int count = 0;

    {
        azul::utils::disposer disposer([&]() { count++; });
    }

    const int expectedCount = 1;
    ASSERT_EQ(expectedCount, count);
}

TEST_F(disposer_fixture, moveAssignment_calledOnce)
{
    int count = 0;

    {
        azul::utils::disposer disposer2;

        {
            azul::utils::disposer disposer1([&]() { count++; });

            disposer2 = std::move(disposer1);
        }

        ASSERT_EQ(0, count);
    }

    const int expectedCount = 1;
    ASSERT_EQ(expectedCount, count);
}

TEST_F(disposer_fixture, moveConstruction_calledOnce)
{
    int count = 0;

    const int expectedCount = 1;

    {
        azul::utils::disposer disposer1([&]() { count++; });

        {
            azul::utils::disposer disposer2 = std::move(disposer1);
        }

        // disposer2 should have increased count
        ASSERT_EQ(expectedCount, count);
    }

    // disposer1 was not initialized anymore, no further increase of count
    ASSERT_EQ(expectedCount, count);
}
