#include "impulso/ipc/sync/robust_mutex.hpp"

#include <cerrno>
#include <cstdio>
#include "exceptions.hpp"
#include <fstream>
#include <impulso/ipc/shared_memory.hpp>
#include <impulso/utils/disposer.hpp>
#include <memory>
#include <pthread.h>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

namespace
{
    class robust_mutex final
    {
    private:
        // synchronisation within a process
        impulso::ipc::shared_memory mutex_memory_;
        impulso::utils::disposer mutex_disposer_;
        pthread_mutex_t * mutex_;

        // synchronisation between multiple processes with recovery option
        impulso::utils::disposer file_disposer_;
        int file_;

        static pthread_mutex_t* create_local_mutex(impulso::ipc::shared_memory& mutex_memory, impulso::utils::disposer& mutex_disposer, bool is_owner, const std::string& name);
        static pthread_mutex_t* init_local_mutex(impulso::ipc::shared_memory& memory, impulso::utils::disposer& disposer);
        static int open_file(std::string const& name, bool const is_owner, impulso::utils::disposer& disposer);

    public:
        robust_mutex(std::string const& name, bool const is_owner)
        {
            const auto local_mutex_name = name + "_mutex_memory_" + std::to_string(getpid());
            mutex_ = create_local_mutex(mutex_memory_, mutex_disposer_, is_owner, local_mutex_name);
            file_ = open_file(name + "_mutex", is_owner, file_disposer_);
        }

        void lock()
        {
            if (pthread_mutex_lock(mutex_) == EDEADLK)
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
            const auto mutex_result = pthread_mutex_trylock(mutex_);
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

            if (pthread_mutex_unlock(mutex_) == EPERM)
            {
                throw std::runtime_error("pthread_mutex_unlock failed, lock not owned by this thread");
            }
        }
    };
}

pthread_mutex_t* robust_mutex::create_local_mutex(impulso::ipc::shared_memory& mutex_memory, impulso::utils::disposer& mutex_disposer,
    bool is_owner, const std::string& name)
{
    try
    {
        mutex_memory = impulso::ipc::shared_memory(name + "_mutex_memory_" + std::to_string(getpid()), sizeof(pthread_mutex_t), is_owner);
    } catch(const impulso::ipc::detail::resource_missing_error&) {
        // in this case the flock mutex has already been created within another process
        // but the posix mutex we use to synchronize between threads within another process has not
        // therefore we internally need to behave here for the local mutex as if we would one this robust_mutex
        is_owner = true;
        mutex_memory = impulso::ipc::shared_memory(name + "_mutex_memory_" + std::to_string(getpid()), sizeof(pthread_mutex_t), is_owner);
    }

    if (is_owner)
    {
        return init_local_mutex(mutex_memory, mutex_disposer);
    }
    else
    {
        return reinterpret_cast<pthread_mutex_t*>(mutex_memory.address());
    }
}

pthread_mutex_t* robust_mutex::init_local_mutex(impulso::ipc::shared_memory& memory, impulso::utils::disposer& disposer)
{
    auto *mutex = reinterpret_cast<pthread_mutex_t*>(memory.address());

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

    disposer.set([mutex]() mutable {
        pthread_mutex_destroy(mutex);
    });

    return mutex;
}

int robust_mutex::open_file(std::string const& name, bool const is_owner, impulso::utils::disposer& disposer)
{
    const std::string file_path("/tmp/mutex_" + name + ".lock");
    
    struct stat buffer;
    if (is_owner && stat(file_path.c_str(), &buffer) != 0)
    {
        std::ofstream output_stream(file_path, std::ios::binary);
        output_stream.close();
    }

    const auto file_handle = std::fopen(file_path.c_str(), "rb");
    if (file_handle == nullptr)
    {
        throw std::runtime_error("fopen failed with error " + std::to_string(errno));
    }

    impulso::utils::disposer file_handle_disposer([file_handle](){
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
