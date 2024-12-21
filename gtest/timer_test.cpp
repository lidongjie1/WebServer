//
// Created by Admin on 2024/12/21.
//
#include <gtest/gtest.h>
#include "../include/timer/heaptimer.h"
#include <thread>  // 用于模拟延迟

// 测试添加定时器功能
TEST(HeapTimerTest, AddTimer) {
    HeapTimer timer;
    bool callbackTriggered = false;

    timer.add(1, 100, [&]() { callbackTriggered = true; }); // 添加定时器，100ms后触发
    EXPECT_EQ(timer.get_next_tick(), 99); // 检查下一个超时时间是否为100ms

    std::this_thread::sleep_for(std::chrono::milliseconds(120)); // 模拟等待120ms

    timer.tick(); // 检查超时定时器

    EXPECT_TRUE(callbackTriggered); // 回调函数是否被触发
}

// 测试调整定时器功能
TEST(HeapTimerTest, AdjustTimer) {
    HeapTimer timer;
    bool callbackTriggered = false;

    timer.add(1, 200, [&]() { callbackTriggered = true; }); // 添加定时器，200ms后触发
    timer.adjust(1, 100); // 将定时器调整到100ms后触发

    EXPECT_EQ(timer.get_next_tick(), 99); // 检查下一个超时时间是否为100ms

    std::this_thread::sleep_for(std::chrono::milliseconds(120)); // 模拟等待120ms
    timer.tick(); // 检查超时定时器

    EXPECT_TRUE(callbackTriggered); // 回调函数是否被触发
}

// 测试触发特定定时器回调函数
TEST(HeapTimerTest, DoWork) {
    HeapTimer timer;
    bool callbackTriggered = false;

    timer.add(1, 200, [&]() { callbackTriggered = true; }); // 添加定时器
    timer.do_work(1); // 手动触发回调函数

    EXPECT_TRUE(callbackTriggered); // 检查回调函数是否被触发
}

// 测试清空定时器
TEST(HeapTimerTest, ClearTimers) {
    HeapTimer timer;
    timer.add(1, 100, []() {}); // 添加定时器
    timer.add(2, 200, []() {}); // 添加定时器

    timer.clear(); // 清空定时器

    EXPECT_EQ(timer.get_next_tick(), -1); // 确保没有定时器
}

// 测试删除超时定时器
TEST(HeapTimerTest, TickRemovesExpiredTimers) {
    HeapTimer timer;
    bool callbackTriggered1 = false;
    bool callbackTriggered2 = false;

    timer.add(1, 150, [&]() { callbackTriggered1 = true; }); // 添加定时器
    timer.add(2, 300, [&]() { callbackTriggered2 = true; }); // 添加定时器

    std::this_thread::sleep_for(std::chrono::milliseconds(150)); // 模拟等待120ms
    timer.tick(); // 检查并删除超时定时器

    EXPECT_TRUE(callbackTriggered1); // 第一个回调函数被触发
    EXPECT_FALSE(callbackTriggered2); // 第二个回调函数未触发

    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 再等100ms
    timer.tick(); // 检查并删除超时定时器

    EXPECT_TRUE(callbackTriggered2); // 第二个回调函数被触发
}
