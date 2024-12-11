//
// Created by Admin on 2024/12/9.
//

#ifndef TINYWEBSERVER_THREADPOOL_H
#define TINYWEBSERVER_THREADPOOL_H
#include <list>
#include <cstdio>
#include <exception>
#include <thread>
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../Log/log.h"
#include "../mysqlPool/sql_connection_pool.h"

template<typename T>
class ThreadPool{
public:
    ThreadPool(int actor_model, ConnectionPool *connPool, int thread_number = 8, int max_request = 10000);

    ~ThreadPool();

    bool append(T *request, int state);
    bool append_p(T *request);
private:
    void run();
private:
    int m_thread_numbers;// 线程池中的线程数
    int m_max_requests;// 请求队列中允许的最大请求数
    std::vector<std::thread> m_threads;// 线程集合
    std::queue<T *> m_work_queue;// 请求队列
    std::mutex m_queue_mutex;// 请求队列的互斥锁
    std::condition_variable m_queue_cond;// 条件变量
    ConnectionPool *connectionPool;// 数据库连接池
    int m_actor_model;                           // Reactor 或 Proactor 模型切换
    bool m_stop;                                 // 是否停止线程池
};

//线程池初始化
template<typename T>
ThreadPool<T>::ThreadPool(int actor_model, ConnectionPool *connPool, int thread_number, int max_request) {
    m_actor_model = actor_model;
    connectionPool = connPool;
    m_thread_numbers = thread_number;
    m_max_requests = max_request;
    m_stop = false;
    if (thread_number <= 0 || max_request <= 0) {
        throw std::invalid_argument("Invalid thread number or max request size.");
    }

    // 创建线程并绑定到工作函数
    for (int i = 0; i < thread_number; ++i) {
        m_threads.emplace_back([this] { run(); }); //emplace_back会在容器内进行构建std::thread(),然后绑定lambda表达式
    }

}

//析构线程池
template<typename T>
ThreadPool<T>::~ThreadPool() {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    m_stop = true;
    m_queue_cond.notify_all();//通知所有线程退出
    for(auto &worker:m_threads){
        if(worker.joinable())
        {
            worker.join();//等待当前线程执行完毕
        }
    }
}

//添加任务
template<typename T>
bool ThreadPool<T>::append_p(T *request) {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    if(m_work_queue.size() >= m_max_requests){
        LOG_INFO("Queue is full");
        return false;
    }
    m_work_queue.push(request);
    m_queue_cond.notify_one();//唤醒一个工作线程
    return true;

}

//添加任务，待状态位
template<typename T>
bool ThreadPool<T>::append(T *request, int state) {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    if(m_work_queue.size() >= m_max_requests)
    {
        LOG_INFO("Queue is full");
        return false; //队列满
    }
    request->m_state = state; // 设置请求的状态
    m_work_queue.push(request); // 将请求加入队列
    m_queue_cond.notify_one();//通知一个工作线程处理
}

//线程执行函数
template<typename T>
void ThreadPool<T>::run() {
    while (true) {
        T *request = nullptr;
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_queue_cond.wait(lock, [this] { return m_stop || !m_work_queue.empty(); }); // 等待任务

        if (m_stop && m_work_queue.empty()) {
            return; // 停止线程池并退出线程
        }

        request = m_work_queue.front(); // 取出一个任务
        m_work_queue.pop();

        if (!request) {
            continue;
        }
        //TODO 这里模式切换有问题，request类型未知，其调用的函数未知
        // 根据 actor 模型处理请求
        if (m_actor_model == 1) { // Reactor 模型
            if (request->m_state == 0) { // 读事件
                if (request->read_once()) {
                    request->improv = 1;
                    ConnectionRAII mysql_con(&request->mysql, connectionPool);
                    request->process();
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            } else { // 写事件
                if (request->write()) {
                    request->improv = 1;
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        } else { // Proactor 模型
            ConnectionRAII mysql_con(&request->mysql, connectionPool);
            request->process();
        }
    }
}




#endif //TINYWEBSERVER_THREADPOOL_H
