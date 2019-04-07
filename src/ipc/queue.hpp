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
            class queue
            {
            private:
                TItem* items_ = nullptr;
                std::uint32_t* read_pos_ = nullptr;
                std::uint32_t* write_pos_ = nullptr;
                std::uint32_t* count_ = nullptr;
                std::uint32_t size_ = 0;

            public:
                explicit queue(void *const memory, std::uint32_t const size, bool initialize = false)
                    : items_(reinterpret_cast<TItem* const>(reinterpret_cast<char *const>(memory) + sizeof(std::uint32_t) * 3)),
                    read_pos_(reinterpret_cast<std::uint32_t* const>(memory)),
                    write_pos_(read_pos_ + 1),
                    count_(write_pos_ + 1),
                    size_((size - static_cast<std::uint32_t>(sizeof(std::uint32_t)) * 3) / static_cast<std::uint32_t>(sizeof(TItem)))
                {
                    if (initialize)
                    {
                        *read_pos_ = 0;
                        *write_pos_ = 0;
                        *count_ = 0;
                    }
                }

                queue() = default;

                queue(queue const&) = default;
                queue(queue &&) = default;
                queue& operator=(queue const&) = default;
                queue& operator=(queue &&) = default;

                bool space_available()
                {
                    return *count_ < size_;
                }

                std::uint32_t count()
                {
                    return *count_;
                }

                std::uint32_t size()
                {
                    return size_;
                }

                bool contains(const TItem& item)
                {
                    for (std::uint32_t i = 0; i < *count_; ++i)
                    {
                        std::uint32_t const position = (*read_pos_ + i + size_) % size_;
                        if (items_[position] == item)
                        {
                            return true;
                        }
                    }

                    return false;
                }

                bool remove(TItem const& item)
                {
                    if (count() == 0)
                    {
                        return false;
                    }

                    for (std::uint32_t i = 0; i < *count_; ++i)
                    {
                        std::uint32_t const position = (*read_pos_ + i + size_) % size_;
                        if (items_[position] == item)
                        {
                            items_[position] = back();
                            pop_back();
                            return true;
                        }
                    }

                    return false;
                }

                void pop()
                {
                    if (count() > 0)
                    {
                        *read_pos_ = (*read_pos_ + 1 + size_) % size_;
                        (*count_)--;
                    }
                    else
                    {
                        throw std::runtime_error("no item available");
                    }
                }

                TItem const& front()
                {
                    if (count() > 0)
                    {
                        return items_[*read_pos_];
                    }

                    throw std::runtime_error("no item available");
                }

                void pop_back()
                {
                    if (count() > 0)
                    {
                        *write_pos_ = (*write_pos_ - 1 + size_) % size_;
                        (*count_)--;
                    }
                    else
                    {
                        throw std::runtime_error("no item available");
                    }
                }

                TItem const& back()
                {
                    if (count() > 0)
                    {
                        const auto postitionOfLastItem = (*write_pos_ + size_ - 1) % size_;
                        return items_[postitionOfLastItem];
                    }

                    throw std::runtime_error("no item available");
                }

                void push_back(TItem const& item)
                {
                    if (space_available())
                    {
                        items_[*write_pos_] = item;
                        *write_pos_ = (*write_pos_ + 1 + size_) % size_;
                        (*count_)++;
                        return;
                    }

                    throw std::runtime_error("no space available");
                }
            };
        } // namespace detail
    }
}
