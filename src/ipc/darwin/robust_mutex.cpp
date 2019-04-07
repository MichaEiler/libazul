#include "azul/ipc/sync/robust_mutex.hpp"

#include <cerrno>
#include <cstdio>
#include "exceptions.hpp"
#include <fstream>
#include <azul/ipc/shared_memory.hpp>
#include <azul/utils/disposer.hpp>
#include <memory>
#include <pthread.h>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

namespace
{
    class RobustMutex final
    {
    private:
        // synchronisation within a process
        azul::ipc::SharedMemory _mutexMemory;
        azul::utils::Disposer _mutexDisposer;
        pthread_mutex_t * _mutex;

        // synchronisation between multiple processes with recovery option
        azul::utils::Disposer file_disposer_;
        int file_;

        static pthread_mutex_t* CreateLocalMutex(azul::ipc::SharedMemory& mutex_memory, azul::utils::Disposer& mutex_disposer, bool isOwner, const std::string& name);
        static pthread_mutex_t* InitLocalMutex(azul::ipc::SharedMemory& memory, azul::utils::Disposer& disposer);
        static int OpenFile(std::string const& name, bool const isOwner, azul::utils::Disposer& disposer);

    public:
        RobustMutex(std::string const& name, bool const isOwner)
        {
            const auto local_mutex_name = name + "__mutexMemory" + std::to_string(getpid());
            _mutex = CreateLocalMutex(_mutexMemory, _mutexDisposer, isOwner, local_mutex_name);
            file_ = OpenFile(name + "_mutex", isOwner, file_disposer_);
        }

        void lock()
        {
            if (pthread_mutex_lock(_mutex) == EDEADLK)
            {
                throw std::runtime_error("pthread_mutex_lock failed (EDEADLK), check your implementation");
            }

            const auto result = flock(file_, LOCK_EX);
            if (result != 0)
            {
                throw std::runtime_error("flock failed with error " + std::to_string(result));
            }

            if (result == EBADF)
            {
                throw std::runtime_error("fd is not an open file descriptor");
            }
        }

        bool try_lock()
        {
            const auto mutex_result = pthread_mutex_trylock(_mutex);
            if (mutex_result == EBUSY)
            {
                return false;
            }

            const auto result = flock(file_, LOCK_EX | LOCK_NB);
            if (result == EWOULDBLOCK)
            {
                return false;
            }

            if (result != 0)
            {
                throw std::runtime_error("flock failed with error " + std::to_string(result));
            }

            return true;
        }

        void unlock()
        {
            const auto result = flock(file_, LOCK_UN);
            if (result != 0)
            {
                throw std::runtime_error("flock failed with error " + std::to_string(result));
            }

            if (pthread_mutex_unlock(_mutex) == EPERM)
            {
                throw std::runtime_error("pthread_mutex_unlock failed, lock not owned by this thread");
            }
        }
    };
}

pthread_mutex_t* RobustMutex::CreateLocalMutex(azul::ipc::SharedMemory& mutex_memory, azul::utils::Disposer& mutex_disposer,
    bool isOwner, const std::string& name)
{
    try
    {
        mutex_memory = azul::ipc::SharedMemory(name + "__mutexMemory" + std::to_string(getpid()), sizeof(pthread_mutex_t), isOwner);
    } catch(const azul::ipc::detail::ResourceMissingError&) {
        // in this case the flock mutex has already been created within another process
        // but the posix mutex we use to synchronize between threads within another process has not
        // therefore we internally need to behave here for the local mutex as if we would one this RobustMutex
        isOwner = true;
        mutex_memory = azul::ipc::SharedMemory(name + "__mutexMemory" + std::to_string(getpid()), sizeof(pthread_mutex_t), isOwner);
    }

    if (isOwner)
    {
        return InitLocalMutex(mutex_memory, mutex_disposer);
    }
    else
    {
        return reinterpret_cast<pthread_mutex_t*>(mutex_memory.Address());
    }
}

pthread_mutex_t* RobustMutex::InitLocalMutex(azul::ipc::SharedMemory& memory, azul::utils::Disposer& disposer)
{
    auto *mutex = reinterpret_cast<pthread_mutex_t*>(memory.Address());

    pthread_mutexattr_t attributes;
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);

    auto const result = pthread_mutex_init(mutex, &attributes);
    pthread_mutexattr_destroy(&attributes);

    if (result != 0)
    {
        throw std::runtime_error("pthread_mutex_init failed, error: " + std::to_string(result));
    }

    disposer.Set([mutex]() mutable {
        pthread_mutex_destroy(mutex);
    });

    return mutex;
}

int RobustMutex::OpenFile(std::string const& name, bool const isOwner, azul::utils::Disposer& disposer)
{
    const std::string file_path("/tmp/mutex_" + name + ".lock");
    
    struct stat buffer;
    if (isOwner && stat(file_path.c_str(), &buffer) != 0)
    {
        std::ofstream output_stream(file_path, std::ios::binary);
        output_stream.close();
    }

    const auto file_handle = std::fopen(file_path.c_str(), "rb");
    if (file_handle == nullptr)
    {
        throw std::runtime_error("fopen failed with error " + std::to_string(errno));
    }

    azul::utils::Disposer file_handle_disposer([file_handle](){
        const auto result = std::fclose(file_handle);
        if (result != 0)
        {
            throw std::runtime_error("fclose failed with error " + std::to_string(result));
        }
    });

    const auto fd = fileno(file_handle);
    disposer = std::move(file_handle_disposer);
    return fd;
}

// -----------------------------------------------------------------------------------------------------

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
