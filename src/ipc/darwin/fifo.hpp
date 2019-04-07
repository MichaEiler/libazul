#pragma once

#include <cstdint>
#include <fcntl.h>
#include <azul/utils/disposer.hpp>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace azul
{
    namespace ipc
    {
        namespace detail
        {
            class FiFo
            {
            private:
                azul::utils::Disposer _fifoDisposer;
                int _fifoDescriptor;

                static int CreateOrOpenFiFo(std::string const& path, bool const isOwner, azul::utils::Disposer& disposer);
            public:
                explicit FiFo(std::string const& name, bool const isOwner)
                    : _fifoDisposer()
                    , _fifoDescriptor(CreateOrOpenFiFo(std::string("/tmp/azul_fifo_") + name, isOwner, _fifoDisposer))
                {

                }

                void Write(char const *const buffer, std::size_t buffer_size)
                {
                    ::write(_fifoDescriptor, buffer, buffer_size);
                }

                std::size_t Read(char *const buffer, std::size_t buffer_size)
                {
                    struct pollfd poll_details{};
                    poll_details.fd = _fifoDescriptor;
                    poll_details.events = POLLIN;

		            int result = 0;
                    while ((result = poll(&poll_details, 1, static_cast<int>(10000))) == 0);
                    if (result < 0)
                    {
                        throw std::runtime_error(std::string("Unexpected error while polling: ") + std::to_string(errno));
                    }

                    result = ::read(_fifoDescriptor, buffer, buffer_size);
                    if (result < 0)
                    {
                        throw std::runtime_error(std::string("Unexpected error while reading from fifo: ") + std::to_string(errno));
                    }
                    return static_cast<std::size_t>(result);
                }

                ssize_t TimedRead(char *const buffer, std::size_t buffer_size, std::uint32_t timeout_ms)
                {
                    struct pollfd poll_details{};
                    poll_details.fd = _fifoDescriptor;
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

                    return Read(buffer, buffer_size);
                }
            };
        }
    }
}

int azul::ipc::detail::FiFo::CreateOrOpenFiFo(std::string const& path, bool const isOwner, azul::utils::Disposer& disposer)
{
    if (isOwner)
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

    disposer.Set([fd, path, isOwner](){
        close(fd);
        if (isOwner)
        {
            remove(path.c_str());
        }
    });

    return fd;
}
