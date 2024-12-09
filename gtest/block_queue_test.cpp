//
// Created by Admin on 2024/12/7.
//
#include "../include/Log/block_queue.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>

// 测试生产者线程
void producer(BlockQueue<int>& queue, int start, int count, std::atomic<bool>& producer_done) {
    for (int i = start; i < start + count; ++i) {
        queue.push(i);
        std::cout << "Produced: " << i << std::endl;
    }
    producer_done = true;
    std::cout << "Producer done." << std::endl;
}

// 测试消费者线程
void consumer(BlockQueue<int>& queue, std::vector<int>& consumed_items, int count) {
    for (int i = 0; i < count; ++i) {
        int item;
        queue.pop(item);
        consumed_items.push_back(item);
        std::cout << "Consumed: " << item << std::endl;
    }
}

// 测试 BlockQueue 的基本功能
TEST(BlockQueueTest, BasicFunctionality) {
    BlockQueue<int> queue(5); // 队列容量为 5
    std::atomic<bool> producer_done(false); // 标记生产者是否完成
    std::vector<int> consumed_items;        // 存储消费者消费的物品

    // 创建生产者和消费者线程
    std::thread prod(producer, std::ref(queue), 0, 10, std::ref(producer_done));
    std::thread cons(consumer, std::ref(queue), std::ref(consumed_items), 10);

    prod.join();
    cons.join();

    // 验证生产者完成
    ASSERT_TRUE(producer_done);

    // 验证消费者消费的物品数量
    ASSERT_EQ(consumed_items.size(), 10);

    // 验证消费的元素是否按顺序
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(consumed_items[i], i);
    }
}

// 测试 BlockQueue 的容量限制
TEST(BlockQueueTest, CapacityLimit) {
    BlockQueue<int> queue(3); // 队列容量为 3
    std::atomic<bool> producer_done(false); // 标记生产者是否完成
    std::vector<int> consumed_items;        // 存储消费者消费的物品

    // 创建生产者线程，尝试插入超过容量的元素
    std::thread prod([&queue, &producer_done]() {
        for (int i = 0; i < 5; ++i) {
            queue.push(i); // 阻塞直到队列有空位
        }
        producer_done = true;
    });

    // 消费者延迟启动，确保生产者会阻塞
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(producer_done); // 生产者应该被阻塞

    // 创建消费者线程
    std::thread cons(consumer, std::ref(queue), std::ref(consumed_items), 5);

    prod.join();
    cons.join();

    // 验证生产者完成
    ASSERT_TRUE(producer_done);

    // 验证消费者消费的物品数量
    ASSERT_EQ(consumed_items.size(), 5);

    // 验证消费的元素是否按顺序
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(consumed_items[i], i);
    }
}

// 测试多生产者多消费者场景
TEST(BlockQueueTest, MultiProducerMultiConsumer) {
    BlockQueue<int> queue(10); // 队列容量为 10
    std::atomic<bool> producer1_done(false), producer2_done(false); // 标记生产者是否完成
    std::vector<int> consumed_items1, consumed_items2;              // 存储消费者消费的物品

    // 创建两个生产者线程
    std::thread prod1(producer, std::ref(queue), 0, 10, std::ref(producer1_done));
    std::thread prod2(producer, std::ref(queue), 10, 10, std::ref(producer2_done));

    // 创建两个消费者线程
    std::thread cons1(consumer, std::ref(queue), std::ref(consumed_items1), 10);
    std::thread cons2(consumer, std::ref(queue), std::ref(consumed_items2), 10);

    prod1.join();
    prod2.join();
    cons1.join();
    cons2.join();

    // 验证生产者完成
    ASSERT_TRUE(producer1_done);
    ASSERT_TRUE(producer2_done);

    // 验证消费者消费的物品总数量
    ASSERT_EQ(consumed_items1.size() + consumed_items2.size(), 20);

    // 验证所有消费的元素是否存在于生产的范围内
    std::vector<int> all_consumed(consumed_items1);
    all_consumed.insert(all_consumed.end(), consumed_items2.begin(), consumed_items2.end());
    std::sort(all_consumed.begin(), all_consumed.end());

    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(all_consumed[i], i);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}