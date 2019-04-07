#include "azul/ipc/sync/RobustMutex.hpp"

#include <linux/RobustMutex.hpp>

azul::ipc::sync::RobustMutex::RobustMutex(std::string const& name, bool const isOwner)
    : _impl(std::make_unique<::RobustMutex>(name, isOwner))
{
}

azul::ipc::sync::RobustMutex::RobustMutex() : _impl(nullptr)
{
}

azul::ipc::sync::RobustMutex::~RobustMutex()
{
}

void azul::ipc::sync::RobustMutex::lock()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::RobustMutex *const instance = reinterpret_cast<::RobustMutex*>(_impl.get());
    instance->lock();
}

bool azul::ipc::sync::RobustMutex::try_lock()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::RobustMutex *const instance = reinterpret_cast<::RobustMutex*>(_impl.get());
    return instance->try_lock();
}

void azul::ipc::sync::RobustMutex::unlock()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::RobustMutex *const instance = reinterpret_cast<::RobustMutex*>(_impl.get());
    instance->unlock();
}
