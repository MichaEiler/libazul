#include "azul/ipc/sync/robust_mutex.hpp"

#include <azul/utils/disposer.hpp>
#include <linux/robust_mutex.hpp>

azul::ipc::sync::robust_mutex::robust_mutex(std::string const& name, bool const is_owner)
    : impl_(std::make_unique<::robust_mutex>(name, is_owner))
{
}

azul::ipc::sync::robust_mutex::robust_mutex() : impl_(nullptr)
{
}

azul::ipc::sync::robust_mutex::~robust_mutex()
{
}

void azul::ipc::sync::robust_mutex::lock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    instance->lock();
}

bool azul::ipc::sync::robust_mutex::try_lock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    return instance->try_lock();
}

void azul::ipc::sync::robust_mutex::unlock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    instance->unlock();
}
