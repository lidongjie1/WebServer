//
// Created by Admin on 2024/12/7.
//

#ifndef TINYWEBSERVER_BLOCK_QUEUE_H
#define TINYWEBSERVER_BLOCK_QUEUE_H
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <queue>
#include <thread>


template<typename T>
class BlockQueue{
public:
    explicit BlockQueue(size_t max_size=1000):m_max_size(max_size)
    {
        if(max_size <= 0){
            throw std::invalid_argument("Max size must be greater than 0");
        }
    }
    ~BlockQueue() = default;

    //清空队列
    void clear(){
        std::unique_lock<std::mutex> lock(m_mutex);
        std::queue<T> empty;
        std::swap(m_queue,empty);
    }

    //判断队列是否为空
    bool empty(){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    //判断队列是否为满
    bool full(){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size()>=m_max_size;
    }
    //获取队首元素
    bool front(T &value){
        std::unique_lock<std::mutex> lock(m_mutex);
        try{
            if(m_queue.empty())
            {
                throw std::runtime_error("Queue is empty");
            }
            value = m_queue.front();
        }catch (const std::exception &e)
        {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    //获取对尾元素
    bool back(T &value){
        std::unique_lock<std::mutex> lock(m_mutex);
        try{
            if(m_queue.empty())
            {
                throw std::runtime_error("Queue is empty");
            }
            value = m_queue.back();
        }catch (const std::exception &e)
        {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    //获取当前队列大小
    size_t size(){
        return m_queue.size();
    }

    //获取队列最大容量
    size_t max_size()
    {
        return  m_max_size;
    }
    //往队列中添加元素,生产者
    bool push(T &item){
        std::unique_lock<std::mutex> lock(m_mutex);
        //生产者，判断当前有没有资源，没有资源就等待
        m_cond_producer.wait(lock, [this]() { return m_queue.size() < m_max_size; });
        //入队
        m_queue.push(item);
        //通知消费者
        m_cond_consumer.notify_one();
        return true;
    }

    //从队列中取元素，消费者
    bool pop(T &item){
        std::unique_lock<std::mutex> lock(m_mutex);
        // 条件变量，若资源为空就等待
        m_cond_consumer.wait(lock, [this]() { return !m_queue.empty(); });
        item = m_queue.front();
        m_queue.pop();
        //通知生产者
        m_cond_producer.notify_one();
        return true;
    }
    //带超时的出队操作
    bool pop(T &item, int ms_timeout){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_cond_consumer.wait(lock,std::chrono::milliseconds(ms_timeout),[this](){return !m_queue.empty();}))
        {
            return false;
        }
        item = m_queue.front();
        m_queue.pop();
        m_cond_producer.notify_one();
        return true;
    }
private:
    std::queue<T> m_queue;     //队列容器
    size_t m_max_size;          // 队列最大值

    std::mutex m_mutex;         //互斥锁
    std::condition_variable m_cond_producer;    //生产者条件变量
    std::condition_variable m_cond_consumer;    //消费者条件变量
};



#endif //TINYWEBSERVER_BLOCK_QUEUE_H
