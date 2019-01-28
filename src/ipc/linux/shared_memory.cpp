#include "impulso/ipc/shared_memory.hpp"

#include <cerrno>
#include <fcntl.h>
#include <impulso/utils/disposer.hpp>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// http://man7.org/linux/man-pages/man7/shm_overview.7.html

namespace
{
    class shared_memory final
    {
    private:
        static int create_shared_memory(bool const is_owner, std::string const& name, impulso::utils::disposer& disposer)
        {
            int const flags(is_owner ? (O_RDWR | O_CREAT | O_EXCL) : (O_RDWR));

            int fd = shm_open(name.c_str(), flags, 0640);
            if (fd < 0 && errno == EEXIST)
            {
                if (shm_unlink(name.c_str()) != 0)
                {
                    throw std::runtime_error("shm_unlink of previously generated shared memory failed, error: " +
                                                std::to_string(errno));
                }
                fd = shm_open(name.c_str(), flags, 0640);
            }

            if (fd < 0)
            {
                throw std::runtime_error("shm_open failed, error: " + std::to_string(errno));
            }

            disposer.set([=]() {
                close(fd);

                if (is_owner)
                {
                    shm_unlink(name.c_str());
                }
            });

            return fd;
        }

        static void* allocate_shared_memory(int const fd, std::uint64_t const size, bool const is_owner, impulso::utils::disposer& disposer)
        {
            if (is_owner && ftruncate(fd, size) != 0)
            {
                throw std::runtime_error("ftruncate failed, error: " + std::to_string(errno));
            }

            const auto address = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd, 0);
            if (address == MAP_FAILED)
            {
                throw std::runtime_error("mmap failed, error: " + std::to_string(errno));
            }

            disposer.set([=]() {
                munmap(address, size);
            });

            return address;
        }

        impulso::utils::disposer fd_disposer_;
        const int fd_ = 0;

        impulso::utils::disposer allocation_disposer_;
        void* const address_ = nullptr;

        const std::uint64_t size_ = 0;

    public:
        explicit shared_memory(std::string const& name, std::uint64_t const size, bool const is_owner)
            : fd_(create_shared_memory(is_owner, name, fd_disposer_)),
              address_(allocate_shared_memory(fd_, size, is_owner, allocation_disposer_)),
              size_(size)
        {
        }

        void* address() const
        {
            return address_;
        }

        std::uint64_t size() const
        {
            return size_;
        }
    };
}

// -----------------------------------------------------------------------------------------------------

impulso::ipc::shared_memory::shared_memory(std::string const& name, std::uint64_t const size, bool const is_owner)
    : impl_(std::make_unique<::shared_memory>(name, size, is_owner))
{
}

impulso::ipc::shared_memory::shared_memory() : impl_(nullptr)
{
}

impulso::ipc::shared_memory::~shared_memory()
{
}

void* impulso::ipc::shared_memory::address() const
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::shared_memory *const instance = reinterpret_cast<::shared_memory*>(impl_.get());
    return instance->address();
}

std::uint64_t impulso::ipc::shared_memory::size() const
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::shared_memory *const instance = reinterpret_cast<::shared_memory*>(impl_.get());
    return instance->size();
}
