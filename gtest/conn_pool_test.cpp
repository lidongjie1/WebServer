//
// Created by Admin on 2024/12/10.
//
#include <gtest/gtest.h>
#include "../include/mysqlPool/sql_connection_pool.h"

class ConnectionPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化数据库连接池
        connPool = ConnectionPool::getInstance();
        connPool->init("192.168.65.128", "root", "web123!@#", "tiny_webserver", 3306, 10, 0);  // 使用真实的配置
    }

    void TearDown() override {
        // 销毁连接池
        connPool->destroyPool();
    }

    ConnectionPool *connPool;
};

// 测试：初始化后，空闲连接数是否正确
TEST_F(ConnectionPoolTest, InitPool) {
    EXPECT_EQ(connPool->getFreeConn(), 10);  // 初始空闲连接应等于最大连接数
}

// 测试：获取连接是否成功
TEST_F(ConnectionPoolTest, GetConnection) {
    MYSQL *conn = connPool->getConnection();
    ASSERT_NE(conn, nullptr);  // 确保连接非空
    connPool->releaseConnection(conn);  // 释放连接
}

// 测试：释放连接是否成功
TEST_F(ConnectionPoolTest, ReleaseConnection) {
    MYSQL *conn = connPool->getConnection();
    connPool->releaseConnection(conn);
    EXPECT_EQ(connPool->getFreeConn(), 10);  // 确保释放后空闲连接数恢复
}

// 测试：多线程获取连接
TEST_F(ConnectionPoolTest, ConcurrentAccess) {
    std::vector<std::thread> threads;
    int successfulConnections = 0;

    auto connectionTask = [&]() {
        MYSQL *conn = connPool->getConnection();
        if (conn) {
            ++successfulConnections;
            connPool->releaseConnection(conn);
        }
    };

    // 创建多个线程同时获取连接
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back(connectionTask);
    }

    for (auto &t : threads) {
        t.join();
    }

    // 确保总成功获取的连接数 <= 最大连接数
    EXPECT_LE(successfulConnections, 10);
}

// 测试：连接池耗尽时的等待机制
TEST_F(ConnectionPoolTest, PoolExhaustion) {
    std::vector<MYSQL *> connections;

    // 消耗所有连接
    for (int i = 0; i < 10; ++i) {
        MYSQL *conn = connPool->getConnection();
        ASSERT_NE(conn, nullptr);
        connections.push_back(conn);
    }

    // 此时没有可用连接，getConnection 应该阻塞或等待
    std::thread blockingThread([&]() {
        MYSQL *conn = connPool->getConnection();
        ASSERT_NE(conn, nullptr);
        connPool->releaseConnection(conn);
    });

    // 释放一个连接，允许阻塞线程获取
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    connPool->releaseConnection(connections.back());
    connections.pop_back();

    blockingThread.join();

    // 释放所有连接
    for (auto conn : connections) {
        connPool->releaseConnection(conn);
    }
    EXPECT_EQ(connPool->getFreeConn(), 10);
}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}