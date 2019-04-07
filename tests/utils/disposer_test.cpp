#include "azul/utils/disposer.hpp"
#include <gmock/gmock.h>

class DisposerTestFixture : public testing::Test
{
};

TEST_F(DisposerTestFixture, Destructor_InitializedDisposer_CalledOnce)
{
    int count = 0;

    {
        azul::utils::Disposer disposer([&]() { count++; });
    }

    const int expectedCount = 1;
    ASSERT_EQ(expectedCount, count);
}

TEST_F(DisposerTestFixture, MoveAssignment_CalledOnce)
{
    int count = 0;

    {
        azul::utils::Disposer disposer2;

        {
            azul::utils::Disposer disposer1([&]() { count++; });

            disposer2 = std::move(disposer1);
        }

        ASSERT_EQ(0, count);
    }

    const int expectedCount = 1;
    ASSERT_EQ(expectedCount, count);
}

TEST_F(DisposerTestFixture, MoveConstruction_CalledOnce)
{
    int count = 0;

    const int expectedCount = 1;

    {
        azul::utils::Disposer disposer1([&]() { count++; });

        {
            azul::utils::Disposer disposer2 = std::move(disposer1);
        }

        // disposer2 should have increased the counter
        ASSERT_EQ(expectedCount, count);
    }

    // disposer1 was not initialized anymore, no further increase of the counter
    ASSERT_EQ(expectedCount, count);
}
