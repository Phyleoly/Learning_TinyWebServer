/*
 * @Author: Phyleoly Phyleoly@gmail.com
 * @Date: 2024-06-13 19:59:48
 * @LastEditTime: 2024-06-13 20:30:05
 * @Description: 基于POSIX实现的简易线程池
 * 
 * Copyright (c) 2024 by Phyeloly, All Rights Reserved. 
 */
#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <pthread.h>
#include <queue>
#include <vector>

typedef struct Task {
    void (*function)(void*);
    void *arg;
} Task; 

class ThreadPool {
public:
    ThreadPool(int threadNum);
    ~ThreadPool();

    void add_task(const Task& task);
private:
    int m_threadsNum;
    int m_taskNum;
    bool m_stop;

    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;

    std::vector<pthread_t> m_threads;
    std::queue<Task> m_tasks;

    static void *thread_function(void *arg);
};

/**
 * @description: 线程池构造函数，实现线程序列的创建与互斥锁和信号量的初始化
 * @param {int} threadNum - 线程池最大线程数
 * @return {*}
 */
ThreadPool::ThreadPool(int threadNum) {
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
    m_stop = false;

    m_threads = std::vector<pthread_t>(threadNum);
    for(int i = 0; i < threadNum; ++i) {
        pthread_create(&m_threads[i], NULL, thread_function, this);
    }
}

/**
 * @description: 线程池析构函数
 * @return {*}
 */
ThreadPool::~ThreadPool()
{
    m_stop = true;
    pthread_mutex_lock(&m_mutex);
    pthread_cond_broadcast(&m_cond);
    pthread_mutex_unlock(&m_mutex);

    for(auto it = m_threads.begin(); it!= m_threads.end(); ++it) {
        pthread_join(*it, NULL);
    }
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

/**
 * @description: 线程池中每个线程的执行函数，线程池对象通过参数传入。
 * @param {void} *arg - 传入的参数为线程池对象的指针。
 * @return {*} - 返回值为 nullptr。
 */
void *ThreadPool::thread_function(void *arg)
{
    ThreadPool *threadPool = (ThreadPool *)arg;
    while(!threadPool->m_stop) {
        pthread_mutex_lock(&threadPool->m_mutex);
        while(threadPool->m_tasks.empty()) {         
            pthread_cond_wait(&threadPool->m_cond, &threadPool->m_mutex);
        }

        Task *task = &threadPool->m_tasks.front();
        threadPool->m_tasks.pop();
        pthread_mutex_unlock(&threadPool->m_mutex);

        task->function(task->arg);
    }
    return nullptr;
}

/**
 * @description: 向线程池中添加任务
 * @param {Task} &task - 待添加任务
 * @return {*}
 */
void ThreadPool::add_task(const Task &task) {
    pthread_mutex_lock(&m_mutex);
    m_tasks.push(task);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);
}

#endif