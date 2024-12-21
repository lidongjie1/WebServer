//
// Created by Admin on 2024/12/19.
//

#include "../include/timer/heaptimer.h"


//调整节点
void HeapTimer::adjust(int id, int newExpires) {
    assert(ref_.count(id)>0);
    size_t index = ref_[id];
    heap_[index].expires = Clock::now() + MS(newExpires);
    if(!sift_down_(index,heap_.size())){
        sift_up_(index);
    }
}

void HeapTimer::add(int id, int timeout, TimeoutCallBack cb) {
    TimeStamp expires = Clock::now() + MS(timeout);
    if(ref_.count(id)){     //如果该定时器已经存在
        size_t index = ref_[id];
        heap_[index].expires = expires;
        heap_[index].timeoutCallBack = cb;
        // 调整堆,先下调整，若不用向下调整再尝试向上调整
        if(!sift_down_(index,heap_.size())){
            sift_up_(index);
        }
    }else{
        //添加到堆尾
        heap_.push_back({id, expires, std::move(cb)});
        size_t index = heap_.size() - 1;
        ref_[id] = index;
        sift_up_(index);
    }

}

// 触发指定定时器的回调函数
void HeapTimer::do_work(int id) {
    if(ref_.count(id) == 0) return;
    size_t index = ref_[id];
    TimerNode node = heap_[index];
    node.timeoutCallBack();
    del_(index);//删除该定时器节点
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);    //删除堆顶元素
}
//
void HeapTimer::del_(size_t index) {
    assert(index < heap_.size());  // 确保索引合法
    size_t lastIndex = heap_.size() - 1;  // 获取堆尾索引
    if (index < lastIndex) {  // 如果要删除的节点不是堆尾
        swap_nodes_(index, lastIndex);  // 将节点与堆尾交换
        ref_.erase(heap_.back().id);    // 删除ID映射
        heap_.pop_back();               // 移除堆尾节点
        // 调整堆
        if (!sift_down_(index, lastIndex)) {
            sift_up_(index);
        }
    } else {  // 如果删除的是堆尾节点
        ref_.erase(heap_.back().id);  // 删除ID映射
        heap_.pop_back();             // 移除堆尾节点
    }
}

//删除超时定时器
void HeapTimer::tick() {
    TimeStamp now = Clock::now();   //获取当前时间
    while (!heap_.empty()){
        TimerNode& node = heap_.front();    //获取堆顶元素
        if(now < node.expires) break;//堆顶元素未超时
        node.timeoutCallBack();
        pop();//删除堆顶元素
    }
}

//获取下一个定时器的剩余时间
int HeapTimer::get_next_tick() {
    if (heap_.empty()) return -1;  // 如果堆为空，返回-1
    TimeStamp now = Clock::now();  // 获取当前时间
    auto duration = std::chrono::duration_cast<MS>(heap_.front().expires - now);  // 计算剩余时间
    return (duration.count() > 0) ? duration.count() : 0;  // 返回剩余时间，最小为0
}

void HeapTimer::sift_up_(size_t index) {
    while (index > 0){
        size_t parent = (index - 1) / 2;
        if(heap_[parent]<heap_[index]) break;
        swap_nodes_(index,parent);
        index = parent;
    }
}

bool HeapTimer::sift_down_(size_t index, size_t n) {
    size_t smallest = index;
    while(true){
        size_t left = 2 * smallest + 1;
        size_t right = 2 * smallest + 2;
        //更新最小值节点位置
        if(left<n && heap_[left] < heap_[smallest]) smallest = left;
        if(right<n && heap_[right] < heap_[smallest]) smallest = right;
        if(smallest == index) break;
        //交换节点信息
        swap_nodes_(index,smallest);
        index = smallest;
    }
    return index!=smallest;
}

void HeapTimer::swap_nodes_(size_t i, size_t j) {
    assert(i<heap_.size() && j < heap_.size());
    std::swap(heap_[i],heap_[j]);
    //交换节点后，更新此时的节点映射
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void HeapTimer::clear() {
    heap_.clear();  // 清空堆
    ref_.clear();  // 清空映射表
}