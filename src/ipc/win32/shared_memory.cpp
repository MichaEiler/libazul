#include "azul/ipc/shared_memory.hpp"

#include <azul/utils/disposer.hpp>
#include <Windows.h>
#include <stdexcept>

namespace
{
    class shared_memory final
    {
    private:
        static void* create_shared_memory(std::string const& name, std::uint64_t const size, bool const is_owner, ::azul::utils::disposer& disposer)
        {
            void* handle = nullptr;
            if (is_owner)
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

            disposer.set([=]() { CloseHandle(handle); });

            auto address = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<SIZE_T>(size));

            if (address == nullptr)
            {
                throw std::runtime_error("MapViewOfFile failed, Error: " + std::to_string(GetLastError()));
            }

            return address;
        }

        ::azul::utils::disposer disposer_;
        void *const address_ = nullptr;
        const std::uint64_t size_ = 0;

    public:
        explicit shared_memory(std::string const& name, std::uint64_t const size, bool const is_owner)
            : size_(size), address_(create_shared_memory(name, size, is_owner, disposer_))
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

::azul::ipc::shared_memory::shared_memory(std::string const& name, std::uint64_t const size, bool const is_owner)
    : impl_(std::make_unique<::shared_memory>(name, size, is_owner))
{
}

::azul::ipc::shared_memory::shared_memory() : impl_(nullptr)
{
}

::azul::ipc::shared_memory::~shared_memory()
{
}

void* ::azul::ipc::shared_memory::address() const
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::shared_memory *const instance = reinterpret_cast<::shared_memory*>(impl_.get());
    return instance->address();
}

std::uint64_t azul::ipc::shared_memory::size() const
{
	if (!impl_)
	{
		throw std::runtime_error("Not initialized.");
	}

    ::shared_memory *const instance = reinterpret_cast<::shared_memory*>(impl_.get());
	return instance->size();
}

