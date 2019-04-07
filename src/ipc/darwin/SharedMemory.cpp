#include "azul/ipc/SharedMemory.hpp"

#include <cerrno>
#include <fcntl.h>
#include <azul/utils/Disposer.hpp>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Exceptions.hpp"
#include "Hash.hpp"
// http://man7.org/linux/man-pages/man7/shm_overview.7.html

namespace
{
    class SharedMemory final
    {
    private:
        static int CreateSharedMemory(bool const isOwner, std::string const& full_name, azul::utils::Disposer& disposer)
        {
            // maximum name length on darwin is 31 bytes, base64 encoded sha1 hash is unique enough and stays within these limits
            const auto hashed_name = azul::ipc::detail::sha1_hash(full_name);
            const auto encoded_name = azul::ipc::detail::base64_encode(hashed_name);
            const auto name = "/" + encoded_name;

            if (isOwner)
            {
                // https://stackoverflow.com/questions/25502229/ftruncate-not-working-on-posix-shared-memory-in-mac-os-x
                shm_unlink(name.c_str());
            }

            int const flags(isOwner ? (O_RDWR | O_CREAT /*| O_EXCL*/) : (O_RDWR));
            
            int fd = shm_open(name.c_str(), flags, 0644/*S_IRUSR | S_IWUSR*/);
            if (fd < 0 && errno == ENOENT)
            {
                throw azul::ipc::detail::resource_missing_error();
            }

            if (fd < 0 && errno == EEXIST)
            {
                if (shm_unlink(name.c_str()) != 0)
                {
                    throw std::runtime_error("shm_unlink of previously generated shared memory failed, error: " + std::to_string(errno));
                }
                fd = shm_open(name.c_str(), flags, 0644/*S_IRUSR | S_IWUSR*/);
            }

            if (fd < 0)
            {
                throw std::runtime_error("shm_open failed, error: " + std::to_string(errno));
            }

            disposer.Set([=]() {
                close(fd);

                if (isOwner)
                {
                    shm_unlink(name.c_str());
                }
            });

            return fd;
        }

        static void* AllocateSharedMemory(int const fd, std::uint64_t const size, bool const isOwner, azul::utils::Disposer& disposer)
        {
            struct stat mapstat;
            if (isOwner && fstat(fd, &mapstat) != -1 && mapstat.st_size == 0 && ftruncate(fd, size) != 0)
            {
                throw std::runtime_error("ftruncate failed, error: " + std::to_string(errno));
            }

            const auto address = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd, 0);
            if (address == MAP_FAILED)
            {
                throw std::runtime_error("mmap failed, error: " + std::to_string(errno));
            }

            disposer.Set([=]() {
                munmap(address, size);
            });

            return address;
        }

        azul::utils::Disposer _fdDisposer;
        const int _fd = 0;

        azul::utils::Disposer _allocationDisposer;
        void* const _address = nullptr;

        const std::uint64_t _size = 0;

    public:
        explicit SharedMemory(std::string const& name, std::uint64_t const size, bool const isOwner)
            : _fd(CreateSharedMemory(isOwner, name, _fdDisposer)),
              _address(AllocateSharedMemory(_fd, size, isOwner, _allocationDisposer)),
              _size(size)
        {
        }

        void* Address() const
        {
            return _address;
        }

        std::uint64_t Size() const
        {
            return _size;
        }
    };
}

// -----------------------------------------------------------------------------------------------------

azul::ipc::SharedMemory::SharedMemory(std::string const& name, std::uint64_t const size, bool const isOwner)
    : _impl(std::make_unique<::SharedMemory>(name, size, isOwner))
{
}

azul::ipc::SharedMemory::SharedMemory() : _impl(nullptr)
{
}

azul::ipc::SharedMemory::~SharedMemory()
{
}

void* azul::ipc::SharedMemory::Address() const
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::SharedMemory *const instance = reinterpret_cast<::SharedMemory*>(_impl.get());
    return instance->Address();
}

std::uint64_t azul::ipc::SharedMemory::Size() const
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::SharedMemory *const instance = reinterpret_cast<::SharedMemory*>(_impl.get());
    return instance->Size();
}
