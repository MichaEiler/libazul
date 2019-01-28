#include "impulso/ipc/sync/robust_mutex.hpp"

#include <impulso/utils/disposer.hpp>
#include <linux/robust_mutex.hpp>

impulso::ipc::sync::robust_mutex::robust_mutex(std::string const& name, bool const is_owner)
    : impl_(std::make_unique<::robust_mutex>(name, is_owner))
{
}

impulso::ipc::sync::robust_mutex::robust_mutex() : impl_(nullptr)
{
}

impulso::ipc::sync::robust_mutex::~robust_mutex()
{
}

void impulso::ipc::sync::robust_mutex::lock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    instance->lock();
}

bool impulso::ipc::sync::robust_mutex::try_lock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    return instance->try_lock();
}

void impulso::ipc::sync::robust_mutex::unlock()
{
    if (!impl_)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::robust_mutex *const instance = reinterpret_cast<::robust_mutex*>(impl_.get());
    instance->unlock();
}
