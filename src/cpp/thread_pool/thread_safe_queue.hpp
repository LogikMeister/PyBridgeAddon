#ifndef _THREAD_SAFE_QUEUE_
#define _THREAD_SAFE_QUEUE_

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue
{   
    public:
        ThreadSafeQueue(): destruct_flag(false)
        {}

        void push(T value)
        {
            std::shared_ptr<T> data = std::make_shared<T>(std::move(value));
            std::lock_guard<std::mutex> lg(queue_mutex);
            safe_queue.push(data);
            cond_var.notify_one();
        }

        bool try_to_pop(T &value)
        {
            std::lock_guard<std::mutex> lg(queue_mutex);
            if (safe_queue.empty())
                return false;
            value = std::move(*safe_queue.front());
            safe_queue.pop();
            return true;
        }

        std::shared_ptr<T> try_to_pop()
        {
            std::lock_guard<std::mutex> lg(queue_mutex);
            if (safe_queue.empty())
                return std::make_shared<T>();
            std::shared_ptr<T> res = safe_queue.front();
            safe_queue.pop();
            return res;
        }

        bool wait_and_pop(T &value)
        {
            std::unique_lock<std::mutex> ul(queue_mutex);
            cond_var.wait(ul, [this](){ return !safe_queue.empty() || destruct_flag; });
            if (destruct_flag) {
                return false;
            }
            value = std::move(*safe_queue.front());
            safe_queue.pop();
            return true;
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> ul(queue_mutex);
            cond_var.wait(ul, [this](){ return !safe_queue.empty() || destruct_flag; });
            if (destruct_flag) {
                return std::make_shared<T>();
            }
            std::shared_ptr<T> res = safe_queue.front();
            safe_queue.pop();
            return res;
        }

        void notify_all_before_destruct()
        {
            destruct_flag = true;
            cond_var.notify_all();
        }

    private:
        std::atomic<bool> destruct_flag;
        std::queue<std::shared_ptr<T>> safe_queue;
        std::mutex queue_mutex;
        std::condition_variable cond_var;
};

#endif