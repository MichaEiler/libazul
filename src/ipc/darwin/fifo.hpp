#pragma once

#include <cstdint>
#include <fcntl.h>
#include <impulso/utils/disposer.hpp>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace impulso
{
    namespace ipc
    {
        namespace detail
        {
            class fifo
            {
            private:
                impulso::utils::disposer fifo_disposer_;
                int fifo_descriptor_;

                static int create_or_open_fifo(std::string const& path, bool const is_owner, impulso::utils::disposer& disposer);
            public:
                explicit fifo(std::string const& name, bool const is_owner)
                    : fifo_disposer_()
                    , fifo_descriptor_(create_or_open_fifo(std::string("/tmp/impulso_fifo_") + name, is_owner, fifo_disposer_))
                {

                }

                void write(char const *const buffer, std::size_t buffer_size)
                {
                    ::write(fifo_descriptor_, buffer, buffer_size);
                }

                std::size_t read(char *const buffer, std::size_t buffer_size)
                {
                    struct pollfd poll_details{};
                    poll_details.fd = fifo_descriptor_;
                    poll_details.events = POLLIN;

		            int result = 0;
                    while ((result = poll(&poll_details, 1, static_cast<int>(10000))) == 0);
                    if (result < 0)
                    {
                        throw std::runtime_error(std::string("Unexpected error while polling: ") + std::to_string(errno));
                    }

                    result = ::read(fifo_descriptor_, buffer, buffer_size);
                    if (result < 0)
                    {
                        throw std::runtime_error(std::string("Unexpected error while reading from fifo: ") + std::to_string(errno));
                    }
                    return static_cast<std::size_t>(result);
                }

                ssize_t timed_read(char *const buffer, std::size_t buffer_size, std::uint32_t timeout_ms)
                {
                    struct pollfd poll_details{};
                    poll_details.fd = fifo_descriptor_;
                    poll_details.events = POLLIN;
                    
                    const auto result = poll(&poll_details, 1, static_cast<int>(timeout_ms));
                    if (result < 0)
                    {
                        throw std::runtime_error(std::string("Unexpected error while polling: ") + std::to_string(errno));
                    }

                    if (result == 0)
                    {
                        return -1;
                    }

                    return read(buffer, buffer_size);
                }
            };
        }
    }
}

int impulso::ipc::detail::fifo::create_or_open_fifo(std::string const& path, bool const is_owner, impulso::utils::disposer& disposer)
{
    if (is_owner)
    {
        const auto result = mkfifo(path.c_str(), S_IRUSR | S_IWUSR);
        if (result != 0 && errno == EDQUOT)
        {
            throw std::runtime_error("Too many inodes opened, try to increase the max. number if you are on macOS.");
        }
        if (result != 0)
        {
            throw std::runtime_error(std::string("Failed to create fifo: ") + std::to_string(errno));
        }

    }

    const auto fd = open(path.c_str(), O_RDWR|O_NONBLOCK);
    if (fd < 0)
       throw std::runtime_error("open of fifo failed: " + std::to_string(errno));

    disposer.set([fd, path, is_owner](){
        close(fd);
        if (is_owner)
        {
            remove(path.c_str());
        }
    });

    return fd;
}
