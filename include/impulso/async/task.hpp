#pragma once

#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

namespace impulso
{
    namespace async
    {
        class base_task_type
        {
        public:
            virtual ~base_task_type() = default;
            virtual void operator()() noexcept = 0;
        };



        template <typename TResult>
        class task_type : public base_task_type
        {
        public:
            explicit task_type(std::function<TResult()> && func)
                : func_(std::make_shared<std::function<TResult()>>(func))
            {
                
            }

            void operator()() noexcept override
            {
                try
                {
                    TResult result = func_->operator()();
                    func_.reset();
                    promise_.set_value(result);
                }
                catch(...)
                {
                    promise_.set_exception(std::current_exception());
                }
            }

            std::future<TResult> get_future() { return promise_.get_future(); }

        private:
            std::promise<TResult> promise_;
            std::shared_ptr<std::function<TResult()>> func_;
        };



        template <>
        class task_type<void> : public base_task_type
        {
        public:
            explicit task_type(std::function<void()> && func)
                : func_(std::make_shared<std::function<void()>>(func))
            {

            }

            void operator()() noexcept override
            {
                try
                {
                    func_->operator()();
                    func_.reset();
                    promise_.set_value();
                }
                catch(...)
                {
                    promise_.set_exception(std::current_exception());
                }
            }

            std::future<void> get_future() { return promise_.get_future(); }
            
        private:
            std::promise<void> promise_;
            std::shared_ptr<std::function<void()>> func_;
        };
    }
}
