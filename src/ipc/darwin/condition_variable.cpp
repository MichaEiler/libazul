#include "azul/ipc/sync/condition_variable.hpp"

#include <errno.h>
#include <fcntl.h>
#include <azul/ipc/sync/robust_mutex.hpp>
#include <azul/ipc/shared_memory.hpp>
#include <azul/utils/disposer.hpp>
#include <memory>
#include <mutex>
#include <queue.hpp>
#include <semaphore.h>
#include <sstream>
#include <string>
#include <thread>

#include "fifo.hpp"

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mkfifo.2.html
// https://apple.stackexchange.com/questions/261288/why-max-inode-count-became-232-1-on-my-hdd-after-update-to-os-x-10-12-sierra
// https://wilsonmar.github.io/maximum-limits/
// http://cgi.di.uoa.gr/~ad/k22/named-pipes.pdf

namespace {
    constexpr int _threadQueueSTORAGE_SIZE = 128 * 1024;
    constexpr int FIFO_SYNC_MESSAGE = 0x12345678;


    class ConditionVariable
    {
    private:
        azul::ipc::SharedMemory _threadQueueMemory;
        azul::ipc::detail::Queue<std::thread::id> _threadQueue;
        azul::ipc::sync::RobustMutex _threadQueueMutex;
        std::string const name_;

        static std::shared_ptr<::azul::ipc::detail::FiFo> GetFiFo(std::string const& name, std::thread::id const& thread_id, bool const isOwner)
        {
            std::stringstream memory_stream;
            memory_stream << thread_id;
            const auto full_name = name + "_" + memory_stream.str();
            const auto fifo = std::make_shared<::azul::ipc::detail::FiFo>(full_name, isOwner);
            return fifo;
        }

    public:
        explicit ConditionVariable(std::string const& name, bool const isOwner)
            : _threadQueueMemory(name + "_threadqueue", _threadQueueSTORAGE_SIZE, isOwner)
            , _threadQueue(_threadQueueMemory.Address(), _threadQueueSTORAGE_SIZE, isOwner)
            , _threadQueueMutex(name + "_threadqueu", isOwner)
            , name_(name)
        {

        }

        void NotifyOne()
        {
            std::unique_lock<azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);

            if (_threadQueue.Count() > 0)
            {
                auto const waiting_thread_id = _threadQueue.Front();
                _threadQueue.Pop();

                auto fifo = GetFiFo(name_, waiting_thread_id, false);
                fifo->Write(reinterpret_cast<const char*>(&FIFO_SYNC_MESSAGE), sizeof(FIFO_SYNC_MESSAGE));
            }
        }

        void NotifyAll()
        {
            std::unique_lock<azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);
            while (_threadQueue.Count() > 0)
            {
                auto const waitingThreadId = _threadQueue.Front();
                _threadQueue.Pop();
                auto fifo = GetFiFo(name_, waitingThreadId, false);
                fifo->Write(reinterpret_cast<const char*>(&FIFO_SYNC_MESSAGE), sizeof(FIFO_SYNC_MESSAGE));
            }
        }

        void Wait(std::unique_lock<::azul::ipc::sync::RobustMutex>& mutex)
        {
            std::unique_lock<::azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);
            const auto current_thread_id = std::this_thread::get_id();
            const auto fifo = GetFiFo(name_, current_thread_id, true);

            _threadQueue.PushBack(current_thread_id);

            mutex.unlock();
            lock.unlock();

            char buffer[sizeof(FIFO_SYNC_MESSAGE)];
            int toRead = static_cast<int>(sizeof(FIFO_SYNC_MESSAGE));
            while (toRead > 0)
            {
                toRead -= static_cast<int>(fifo->Read(buffer, static_cast<std::size_t>(toRead)));
            }

            lock.lock();
            mutex.lock();
        }

        std::cv_status WaitFor(std::unique_lock<::azul::ipc::sync::RobustMutex>& mutex, std::chrono::milliseconds const& timeout)
        {
            std::unique_lock<::azul::ipc::sync::RobustMutex> lock(_threadQueueMutex);
            const auto current_thread_id = std::this_thread::get_id();
            const auto fifo = GetFiFo(name_, current_thread_id, true);

            _threadQueue.PushBack(current_thread_id);

            char buffer[sizeof(FIFO_SYNC_MESSAGE)];
            int toRead = static_cast<int>(sizeof(FIFO_SYNC_MESSAGE));
            while (toRead > 0)
            {
                mutex.unlock();
                lock.unlock();

                const auto result = fifo->TimedRead(buffer, static_cast<std::size_t>(toRead), static_cast<std::uint32_t>(timeout.count()));

                lock.lock();
                mutex.lock();

                if (result < 0)
                {
                    return std::cv_status::timeout;
                }
                toRead -= static_cast<int>(result);
            }

            return std::cv_status::no_timeout;

        }

    };
}


// -----------------------------------------------------------------------------------------------------

azul::ipc::sync::ConditionVariable::ConditionVariable(std::string const& name, bool const isOwner)
    : _impl(std::make_unique<::ConditionVariable>(name, isOwner))
{
}

azul::ipc::sync::ConditionVariable::ConditionVariable() : _impl(nullptr)
{
}

azul::ipc::sync::ConditionVariable::~ConditionVariable()
{
}

void azul::ipc::sync::ConditionVariable::NotifyOne()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    instance->NotifyOne();
}

void azul::ipc::sync::ConditionVariable::NotifyAll()
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    instance->NotifyAll();
}

void azul::ipc::sync::ConditionVariable::Wait(std::unique_lock<ipc::sync::RobustMutex>& mutex)
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    instance->Wait(mutex);
}

std::cv_status azul::ipc::sync::ConditionVariable::WaitFor(std::unique_lock<ipc::sync::RobustMutex>& mutex, std::chrono::milliseconds const& timeout)
{
    if (!_impl)
    {
        throw std::runtime_error("Not initialized.");
    }

    ::ConditionVariable *const instance = reinterpret_cast<::ConditionVariable*>(_impl.get());
    return instance->WaitFor(mutex, timeout);
}
