//
// Created by Admin on 2024/12/19.
//

#ifndef TINYWEBSERVER_HEAPTIMER_H
#define TINYWEBSERVER_HEAPTIMER_H
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <cassert>


using TimeoutCallBack = std::function<void()>;//超时回调函数
using Clock = std::chrono::high_resolution_clock; //定义高分辨时钟别名
using MS = std::chrono::milliseconds; //毫秒
using TimeStamp = Clock::time_point;//时间戳别名

//定时器节点结构体
struct TimerNode{
    int id;
    TimeStamp expires;  //定时器过期时间
    TimeoutCallBack timeoutCallBack; //定时器回调函数

    //小根堆比较运算符，过期时间早的优先级更高
    bool operator < (const TimerNode& other) const{
        return expires > other.expires;
    }
};

class HeapTimer{
public:
    HeapTimer();

    ~HeapTimer();

    void adjust(int id, int newExpires); //调整定时器时间

    void add(int id, int timeout, TimeoutCallBack cb); //添加定时器

    void do_work(int id);

    void clear();   //清空定时器

    void pop(); // 删除堆顶计时器

    void tick(); //删除超时定时器

    int get_next_tick();    //获取下一个定时器的剩余时间
private:
    void del_(size_t index); //删除指定位置的定时器
    void sift_up_(size_t index); //上移调整堆
    bool sift_down_(size_t index, size_t n); //下移调整堆
    void swap_nodes_(size_t i, size_t j); //交换堆中的两个节点

    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;   //定时器id到堆索引的映射

};
#endif //TINYWEBSERVER_HEAPTIMER_H
