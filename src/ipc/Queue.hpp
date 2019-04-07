#pragma once

#include <cstdint>
#include <stdexcept>

namespace azul
{
    namespace ipc
    {
        namespace detail
        {
            template <typename TItem>
            class Queue
            {
            private:
                TItem* _items = nullptr;
                std::uint32_t* _readPosition = nullptr;
                std::uint32_t* _writePosition = nullptr;
                std::uint32_t* _count = nullptr;
                std::uint32_t _size = 0;

            public:
                explicit Queue(void *const memory, std::uint32_t const size, bool initialize = false)
                    : _items(reinterpret_cast<TItem* const>(reinterpret_cast<char *const>(memory) + sizeof(std::uint32_t) * 3)),
                    _readPosition(reinterpret_cast<std::uint32_t* const>(memory)),
                    _writePosition(_readPosition + 1),
                    _count(_writePosition + 1),
                    _size((size - static_cast<std::uint32_t>(sizeof(std::uint32_t)) * 3) / static_cast<std::uint32_t>(sizeof(TItem)))
                {
                    if (initialize)
                    {
                        *_readPosition = 0;
                        *_writePosition = 0;
                        *_count = 0;
                    }
                }

                Queue() = default;

                Queue(Queue const&) = default;
                Queue(Queue &&) = default;
                Queue& operator=(Queue const&) = default;
                Queue& operator=(Queue &&) = default;

                bool SpaceAvailable() const
                {
                    return *_count < _size;
                }

                std::uint32_t Count() const
                {
                    return *_count;
                }

                std::uint32_t Size() const
                {
                    return _size;
                }

                bool Contains(const TItem& item) const
                {
                    for (std::uint32_t i = 0; i < *_count; ++i)
                    {
                        std::uint32_t const position = (*_readPosition + i + _size) % _size;
                        if (_items[position] == item)
                        {
                            return true;
                        }
                    }

                    return false;
                }

                bool Remove(TItem const& item)
                {
                    if (Count() == 0)
                    {
                        return false;
                    }

                    for (std::uint32_t i = 0; i < *_count; ++i)
                    {
                        std::uint32_t const position = (*_readPosition + i + _size) % _size;
                        if (_items[position] == item)
                        {
                            _items[position] = Back();
                            PopBack();
                            return true;
                        }
                    }

                    return false;
                }

                void Pop()
                {
                    if (Count() > 0)
                    {
                        *_readPosition = (*_readPosition + 1 + _size) % _size;
                        (*_count)--;
                    }
                    else
                    {
                        throw std::runtime_error("no item available");
                    }
                }

                TItem const& Front() const
                {
                    if (Count() > 0)
                    {
                        return _items[*_readPosition];
                    }

                    throw std::runtime_error("no item available");
                }

                void PopBack()
                {
                    if (Count() > 0)
                    {
                        *_writePosition = (*_writePosition - 1 + _size) % _size;
                        (*_count)--;
                    }
                    else
                    {
                        throw std::runtime_error("no item available");
                    }
                }

                TItem const& Back() const
                {
                    if (Count() > 0)
                    {
                        const auto postitionOfLastItem = (*_writePosition + _size - 1) % _size;
                        return _items[postitionOfLastItem];
                    }

                    throw std::runtime_error("no item available");
                }

                void PushBack(TItem const& item)
                {
                    if (SpaceAvailable())
                    {
                        _items[*_writePosition] = item;
                        *_writePosition = (*_writePosition + 1 + _size) % _size;
                        (*_count)++;
                        return;
                    }

                    throw std::runtime_error("no space available");
                }
            };
        }
    }
}
