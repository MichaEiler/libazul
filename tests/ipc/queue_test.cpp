#include <../../src/ipc/queue.hpp>
#include <gmock/gmock.h>
#include <memory>
#include <vector>

class QueueTestFixture : public testing::Test
{
protected:
    std::vector<int> _buffer;
    azul::ipc::detail::Queue<int> _queue;

public:
    void SetUp()
    {
        _buffer.resize(32);
        const bool initialize = true;
        _queue = azul::ipc::detail::Queue<int>(&_buffer[0], static_cast<uint32_t>(_buffer.size() * sizeof(int)), initialize);
    }
    void TearDown()
    {
    }
};

TEST_F(QueueTestFixture, SpaceAvailable_Empty_ReturnsTrue)
{
    ASSERT_TRUE(_queue.SpaceAvailable());
}

TEST_F(QueueTestFixture, SpaceAvailable_FewElements_ReturnsTrue)
{
    _queue.PushBack(1);
    ASSERT_TRUE(_queue.SpaceAvailable());
}

TEST_F(QueueTestFixture, SpaceAvailable_Full_ReturnsFalse)
{
    for (std::uint32_t i = 0; i < _queue.Size(); ++i)
    {
        _queue.PushBack(static_cast<std::uint32_t>(i));
    }

    ASSERT_TRUE(!_queue.SpaceAvailable());
}

TEST_F(QueueTestFixture, Count_Empty_ReturnsZero)
{
    const std::uint32_t expected_count = 0;
    ASSERT_EQ(expected_count, _queue.Count());
}

TEST_F(QueueTestFixture, Count_NotEmpty_ReturnsNumber)
{
    _queue.PushBack(1);
    _queue.PushBack(2);
    _queue.PushBack(3);

    const std::uint32_t expected_count = 3;
    ASSERT_EQ(expected_count, _queue.Count());
}

TEST_F(QueueTestFixture, Size_Local_BufferbufferSizeMinus3Int)
{
    const std::uint32_t expectedBufferSize = 29;
    ASSERT_EQ(expectedBufferSize, _queue.Size());
}

TEST_F(QueueTestFixture, Pop_Empty_ThrowsRuntimeError)
{
    EXPECT_THROW(_queue.Pop(), std::runtime_error);
}

TEST_F(QueueTestFixture, Pop_FewItems_ReturnsInOrder)
{
    for (std::uint32_t i = 0; i < _queue.Size(); ++i)
    {
        _queue.PushBack(static_cast<std::uint32_t>(i));
    }
    
    const std::uint32_t expected_count = _queue.Size();
    ASSERT_EQ(expected_count, _queue.Count());

    for (std::uint32_t i = 0; i < _queue.Size(); ++i)
    {
        const int front_item = _queue.Front();
        ASSERT_EQ(int(i), front_item);
        _queue.Pop();
    }
}

TEST_F(QueueTestFixture, PopBack_FewItems_ReturnsInInvertedOrder)
{
    for (std::uint32_t i = 0; i < _queue.Size(); ++i)
    {
        _queue.PushBack(static_cast<std::uint32_t>(i));
    }

    const std::uint32_t expected_count = _queue.Size();
    ASSERT_EQ(expected_count, _queue.Count());

    for (std::uint32_t i = 0; i < _queue.Size(); ++i)
    {
        const int front_item = _queue.Back();
        ASSERT_EQ(int(expected_count - i - 1), front_item);
        _queue.PopBack();
    }
}

TEST_F(QueueTestFixture, PushBack_PartiallyEmptied_BufferusesAvailableSpace)
{
    for (std::uint32_t i = 0; i < _queue.Size(); ++i)
    {
        _queue.PushBack(static_cast<std::uint32_t>(i));
    }

    const std::uint32_t expected_count = _queue.Size();
    ASSERT_EQ(expected_count, _queue.Count());

    _queue.Pop();
    _queue.Pop();
    _queue.Pop();

    _queue.PushBack(100);
    _queue.PushBack(200);
    _queue.PushBack(300);

    EXPECT_THROW(_queue.PushBack(400), std::runtime_error);

    _queue.PopBack();
    _queue.PopBack();
    _queue.PopBack();

    for (std::uint32_t i = 3; i < _queue.Size(); ++i)
    {
        const int front_item = _queue.Front();
        ASSERT_EQ(int(i), front_item);
        _queue.Pop();
    }
}

TEST_F(QueueTestFixture, Remove_ItemExists_IsRemoved)
{
    _queue.PushBack(1);
    _queue.PushBack(2);
    _queue.PushBack(3);

    _queue.Remove(2);

    ASSERT_EQ(1, _queue.Front());
    _queue.Pop();
    ASSERT_EQ(3, _queue.Front());
    _queue.Pop();
    ASSERT_EQ(0u, _queue.Count());
}

TEST_F(QueueTestFixture, Remove_ItemExistsTwice_FirstIsRemoved)
{
    _queue.PushBack(1);
    _queue.PushBack(3);
    _queue.PushBack(3);
    _queue.PushBack(4);

    _queue.Remove(3);

    ASSERT_TRUE(_queue.Contains(3));
    ASSERT_EQ(3, _queue.Count());
}

TEST_F(QueueTestFixture, Remove_ItemDoesNotExist_NoChange)
{
    _queue.PushBack(1);
    _queue.PushBack(2);
    _queue.PushBack(3);

    ASSERT_FALSE(_queue.Remove(4));
}

TEST_F(QueueTestFixture, Contains_ItemExists_ReturnsTrue)
{
    _queue.PushBack(1);
    _queue.PushBack(2);
    _queue.PushBack(3);

    ASSERT_TRUE(_queue.Contains(3));
}

TEST_F(QueueTestFixture, Contains_ItemDoesNotExist_ReturnsFalse)
{
    _queue.PushBack(1);
    _queue.PushBack(2);
    _queue.PushBack(3);

    ASSERT_FALSE(_queue.Contains(4));
}
