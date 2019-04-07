#include <../../src/ipc/queue.hpp>
#include <gmock/gmock.h>
#include <memory>
#include <vector>

class queue_fixture : public testing::Test
{
protected:
    std::vector<int> buffer_;
    azul::ipc::detail::queue<int> queue_;

public:
    void SetUp()
    {
        buffer_.resize(32);
        const bool initialize = true;
        queue_ = azul::ipc::detail::queue<int>(&buffer_[0], static_cast<uint32_t>(buffer_.size() * sizeof(int)), initialize);
    }
    void TearDown()
    {
    }
};

TEST_F(queue_fixture, spaceAvailable_empty_returnsTrue)
{
    ASSERT_TRUE(queue_.space_available());
}

TEST_F(queue_fixture, spaceAvailable_fewElements_returnsTrue)
{
    queue_.push_back(1);
    ASSERT_TRUE(queue_.space_available());
}

TEST_F(queue_fixture, spaceAvailable_full_returnsFalse)
{
    for (std::uint32_t i = 0; i < queue_.size(); ++i)
    {
        queue_.push_back(static_cast<std::uint32_t>(i));
    }

    ASSERT_TRUE(!queue_.space_available());
}

TEST_F(queue_fixture, count_empty_returnsZero)
{
    const std::uint32_t expected_count = 0;
    ASSERT_EQ(expected_count, queue_.count());
}

TEST_F(queue_fixture, count_notEmpty_returnsNumber)
{
    queue_.push_back(1);
    queue_.push_back(2);
    queue_.push_back(3);

    const std::uint32_t expected_count = 3;
    ASSERT_EQ(expected_count, queue_.count());
}

TEST_F(queue_fixture, size_localBuffer_bufferSizeMinus3Int)
{
    const std::uint32_t expectedBufferSize = 29;
    ASSERT_EQ(expectedBufferSize, queue_.size());
}

TEST_F(queue_fixture, pop_empty_throwsRuntimeError)
{
    EXPECT_THROW(queue_.pop(), std::runtime_error);
}

TEST_F(queue_fixture, pop_fewItems_returnsInOrder)
{
    for (std::uint32_t i = 0; i < queue_.size(); ++i)
    {
        queue_.push_back(static_cast<std::uint32_t>(i));
    }
    
    const std::uint32_t expected_count = queue_.size();
    ASSERT_EQ(expected_count, queue_.count());

    for (std::uint32_t i = 0; i < queue_.size(); ++i)
    {
        const int front_item = queue_.front();
        ASSERT_EQ(int(i), front_item);
        queue_.pop();
    }
}

TEST_F(queue_fixture, popBack_fewItems_returnsInInvertedOrder)
{
    for (std::uint32_t i = 0; i < queue_.size(); ++i)
    {
        queue_.push_back(static_cast<std::uint32_t>(i));
    }

    const std::uint32_t expected_count = queue_.size();
    ASSERT_EQ(expected_count, queue_.count());

    for (std::uint32_t i = 0; i < queue_.size(); ++i)
    {
        const int front_item = queue_.back();
        ASSERT_EQ(int(expected_count - i - 1), front_item);
        queue_.pop_back();
    }
}

TEST_F(queue_fixture, pushBack_partiallyEmptiedBuffer_usesAvailableSpace)
{
    for (std::uint32_t i = 0; i < queue_.size(); ++i)
    {
        queue_.push_back(static_cast<std::uint32_t>(i));
    }

    const std::uint32_t expected_count = queue_.size();
    ASSERT_EQ(expected_count, queue_.count());

    queue_.pop();
    queue_.pop();
    queue_.pop();

    queue_.push_back(100);
    queue_.push_back(200);
    queue_.push_back(300);

    EXPECT_THROW(queue_.push_back(400), std::runtime_error);

    queue_.pop_back();
    queue_.pop_back();
    queue_.pop_back();

    for (std::uint32_t i = 3; i < queue_.size(); ++i)
    {
        const int front_item = queue_.front();
        ASSERT_EQ(int(i), front_item);
        queue_.pop();
    }
}

TEST_F(queue_fixture, remove_itemExists_isRemoved)
{
    queue_.push_back(1);
    queue_.push_back(2);
    queue_.push_back(3);

    queue_.remove(2);

    ASSERT_EQ(1, queue_.front());
    queue_.pop();
    ASSERT_EQ(3, queue_.front());
    queue_.pop();
    ASSERT_EQ(0u, queue_.count());
}

TEST_F(queue_fixture, remove_itemExistsTwice_firstIsRemoved)
{
    queue_.push_back(1);
    queue_.push_back(3);
    queue_.push_back(3);
    queue_.push_back(4);

    queue_.remove(3);

    ASSERT_TRUE(queue_.contains(3));
    ASSERT_EQ(3, queue_.count());
}

TEST_F(queue_fixture, remove_itemDoesNotExist_noChange)
{
    queue_.push_back(1);
    queue_.push_back(2);
    queue_.push_back(3);

    ASSERT_FALSE(queue_.remove(4));
}

TEST_F(queue_fixture, contains_itemExists_returnsTrue)
{
    queue_.push_back(1);
    queue_.push_back(2);
    queue_.push_back(3);

    ASSERT_TRUE(queue_.contains(3));
}

TEST_F(queue_fixture, contains_itemDoesNotExist_returnsFalse)
{
    queue_.push_back(1);
    queue_.push_back(2);
    queue_.push_back(3);

    ASSERT_FALSE(queue_.contains(4));
}
