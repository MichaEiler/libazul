#include "azul/ipc/shared_memory.hpp"

#include <azul/utils/disposer.hpp>
#include <Windows.h>
#include <stdexcept>

namespace
{
    class SharedMemory final
    {
    private:
        static void* CreateSharedMemory(std::string const& name, std::uint64_t const size, bool const isOwner, ::azul::utils::Disposer& disposer)
        {
            void* handle = nullptr;
            if (isOwner)
            {
                handle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, (size >> 32),
                                            size & 0xffffffff, name.c_str());
            }
            else
            {
                handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, name.c_str());
            }

            if (handle == nullptr)
            {
                throw std::runtime_error("CreateFileMappingA failed, Error: " + std::to_string(GetLastError()));
            }

            disposer.Set([=]() { CloseHandle(handle); });

            auto address = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<_sizeT>(size));

            if (address == nullptr)
            {
                throw std::runtime_error("MapViewOfFile failed, Error: " + std::to_string(GetLastError()));
            }

            return address;
        }

        ::azul::utils::Disposer _disposer;
        void *const _address = nullptr;
        const std::uint64_t _size = 0;

    public:
        explicit SharedMemory(std::string const& name, std::uint64_t const size, bool const isOwner)
            : _size(size), _address(CreateSharedMemory(name, size, isOwner, _disposer))
        {
        }

        void* address() const
        {
            return _address;
        }

        std::uint64_t size() const
        {
            return _size;
        }
    };
}

// -----------------------------------------------------------------------------------------------------

::azul::ipc::SharedMemory::SharedMemory(std::string const& name, std::uint64_t const size, bool const isOwner)
    : _impl(std::make_unique<::SharedMemory>(name, size, isOwner))
{
}

::azul::ipc::SharedMemory::SharedMemory() : _impl(nullptr)
{
}

::azul::ipc::SharedMemory::~SharedMemory()
{
}

void* ::azul::ipc::SharedMemory::address() const
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::SharedMemory *const instance = reinterpret_cast<::SharedMemory*>(_impl.get());
    return instance->address();
}

std::uint64_t azul::ipc::SharedMemory::size() const
{
	if (!_impl)
	{
		throw std::runtime_error("Not initialized.");
	}

    ::SharedMemory *const instance = reinterpret_cast<::SharedMemory*>(_impl.get());
	return instance->size();
}

