//
// Created by Admin on 2024/12/18.
//
#include "gtest/gtest.h"
#include "../include/http/http_conn.h"
#include <arpa/inet.h>
#include <unistd.h>

class HttpConnTest : public ::testing::Test {
protected:
    HttpConn conn;       // HttpConn 实例
    int serverFd;        // 服务器文件描述符
    int clientFd;        // 客户端文件描述符
    sockaddr_in serverAddr{}, clientAddr{};

    void SetUp() override {
        conn.SetSrcDir("/home/adminer/Projects/TinyWebserver/test_file/http_response_test");
        // 初始化日志模块
        Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/http_conn_test.log", 0, 1024, 500, 0);

        // 创建服务器 socket
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_GT(serverFd, 0);

        // 设置地址复用选项
        int opt = 1;
        ASSERT_EQ(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), 0);

        // 设置服务器地址
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(8080);

        // 绑定地址并开始监听
        ASSERT_EQ(bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)), 0) << "Bind failed, check port 8080 availability.";
        ASSERT_EQ(listen(serverFd, 5), 0);

        // 创建客户端 socket 并连接服务器
        clientFd = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_GT(clientFd, 0);
        ASSERT_EQ(connect(clientFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)), 0);

        // 接收客户端连接
        socklen_t clientLen = sizeof(clientAddr);
        int testFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
        ASSERT_GT(testFd, 0);

        // 初始化 HttpConn 实例
        conn.Init(testFd, clientAddr);
    }


    void TearDown() override {
        // 关闭客户端和服务器 socket
        if (clientFd > 0) close(clientFd);
        if (serverFd > 0) close(serverFd);
        conn.Close();
    }
};

TEST_F(HttpConnTest, GetIpAndPort) {
    EXPECT_EQ(conn.GetIp(), "127.0.0.1");
    //
    EXPECT_EQ(conn.GetPort(), 8080);
}

TEST_F(HttpConnTest, Read) {
    // 模拟写入数据到客户端 socket
    const char* testData = "POST /test.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    ssize_t sentBytes = send(clientFd, testData, strlen(testData), 0);
    ASSERT_GT(sentBytes, 0);

    // 测试 HttpConn::Read 方法
    int saveErrno = 0;
    ssize_t bytesRead = conn.Read(&saveErrno);
    EXPECT_GT(bytesRead, 0);
    EXPECT_EQ(saveErrno, 0);
}

// 暂时未仔细测这个写入
TEST_F(HttpConnTest, Write) {
    // 模拟请求处理
    conn.Process();

    // 测试 HttpConn::Write 方法
    int saveErrno = 0;
    ssize_t bytesWritten = conn.Write(&saveErrno);
    EXPECT_GT(bytesWritten, 0);
    EXPECT_EQ(saveErrno, 0);
}

TEST_F(HttpConnTest, Process) {
    // 模拟写入数据到客户端 socket
    const char* testData = "POST 403.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    ssize_t sentBytes = send(clientFd, testData, strlen(testData), 0);
    ASSERT_GT(sentBytes, 0);

    // 测试 HttpConn::Process 方法
    int saveErrno = 0;
    conn.Read(&saveErrno);

    bool success = conn.Process();
    EXPECT_TRUE(success);

    // 验证响应内容
    Buffer& writeBuffer = conn.GetWriteBuffer();
    std::string responseHeader(writeBuffer.Peek(), writeBuffer.ReadableBytes());
    EXPECT_TRUE(responseHeader.find("HTTP/1.1 200 OK") != std::string::npos);
}

TEST_F(HttpConnTest, KeepAlive) {
    // 模拟长连接请求
    const char* testData = "POST 403.html HTTP/1.1\r\nConnection: keep-alive\r\n\r\n";
    ssize_t sentBytes = send(clientFd, testData, strlen(testData), 0);
    ASSERT_GT(sentBytes, 0);

    // 处理请求
    int saveErrno = 0;
    conn.Read(&saveErrno);
    conn.Process();

    // 验证是否保持连接
    EXPECT_TRUE(conn.IsKeepAlive());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
