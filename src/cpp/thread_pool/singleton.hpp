#ifndef _THREAD_POOL_SINGLETON_H_
#define _THREAD_POOL_SINGLETON_H_

#include "./thread_pool.hpp"

class GlobalThreadPool {
private:
    static GlobalThreadPool* instance;
    ThreadPool* thread_pool;

public:
    static GlobalThreadPool* getInstance() {
        if (instance == nullptr) {
            instance = new GlobalThreadPool();
            instance->thread_pool = new ThreadPool();
        }
        return instance;
    }

    static void createInstance(unsigned int thread_nums) {
        if (instance != nullptr) {
            // Destroy the thread pool
            GlobalThreadPool::destroyInstance();
        }
        instance = new GlobalThreadPool();
        instance->thread_pool = new ThreadPool(thread_nums);
    }

    static void destroyInstance() {
        delete instance->thread_pool;
        instance->thread_pool = nullptr;
        delete instance;
        instance = nullptr;
    }

    template<typename FunctionType>
    std::future<typename std::invoke_result<FunctionType>::type> submit(FunctionType f_) {
        return thread_pool->submit(f_);
    }
};

GlobalThreadPool* GlobalThreadPool::instance = nullptr;

#endif