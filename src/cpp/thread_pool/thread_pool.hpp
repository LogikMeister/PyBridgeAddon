#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <future>
#include <type_traits>
#include "./joiner.hpp"
#include "./thread_safe_queue.hpp"

   
class FunctionWrapper
{
    struct Base 
    {
        virtual void call() = 0;
        virtual ~Base() = default;
    };

    template <typename F>
    struct Special: Base
    {
        F f;
        
        explicit Special(F &&f_): f(std::move(f_))
        {}

        void call()
        {
            f();
        }
    };
    
    public:
        FunctionWrapper() = default;

        template <typename F>
        FunctionWrapper(F &&f_): impl(std::make_unique<Special<F>>(std::forward<F>(f_)))
        {}

        FunctionWrapper(FunctionWrapper &&other): impl(std::move(other.impl))
        {}

        FunctionWrapper &operator=(FunctionWrapper &&other)
        {
            impl = std::move(other.impl);
            return *this;
        }

        FunctionWrapper(FunctionWrapper &) = delete;

        FunctionWrapper &operator=(FunctionWrapper &) = delete;

        void operator()()
        {
            impl -> call();
        }


    private:
        std::unique_ptr<Base> impl;
};

class ThreadPool
{
    public:
        ThreadPool(unsigned int thread_nums = std::thread::hardware_concurrency())
        : destruct_flag(false),
        thread_safe_free_joiner(thread_list)
        {
            try
            {
                for (unsigned int i = 0; i < thread_nums; ++i)
                {
                    thread_list.emplace_back(&ThreadPool::worker_thread, this);
                }
            } catch (...)
            {
                destruct_flag = true;
                throw std::runtime_error("thread create error");
            }
        }

        ~ThreadPool()
        {
            destruct_flag = true;
            safe_queue.notify_all_before_destruct();
        }

        template<typename FunctionType>
        std::future<typename std::invoke_result<FunctionType>::type> submit(FunctionType f_) {
            using returnType = typename std::invoke_result<FunctionType>::type;
            std::packaged_task<returnType()> task(std::move(f_));
            std::future<returnType> res(task.get_future());
            safe_queue.push(std::move(task));
            return res;
        }

    private:
        void worker_thread()
        {   
            while (!destruct_flag)
            {
                FunctionWrapper task;
                if (safe_queue.try_to_pop(task))
                {
                    task();
                } else
                {   
                    /**
                     * 
                     *  std::this_thread::yield();
                     *  资源消耗大, 采用任务队列为空时, 线程进入休眠待唤醒的方式: ThreadPool::ThreadSafeQueue::wait_and_pop().
                     * 
                     *  std::unique_lock 加锁 直到ThreadPool::sumbit()调用std::condition_varialbe::notify_one后唤醒.
                     *  注意由于ThreadPool::Joiner为了保证各个线程安全退出, 在调用析构函数时执行std::thread::join(), 如果各个线程处于休眠时加的锁未解锁, 导致程序进入死锁状态.
                     *  因此需要解锁操作 ThreadSafeQueue::notify_all_before_destruct()
                     *  
                     */
                    if (safe_queue.wait_and_pop(task))
                        task();
                }
            }
        }

    private:
        ThreadSafeQueue<FunctionWrapper> safe_queue;
        std::atomic<bool> destruct_flag;
        std::vector<std::thread> thread_list;
        util::Joiner thread_safe_free_joiner;
};

#endif