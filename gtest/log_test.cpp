//
// Created by Admin on 2024/12/8.
//
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "../include/Log/log.h"
#include <unistd.h>

// 辅助函数：检查日志文件是否包含指定内容
bool file_contains(const std::string &file_path, const std::string &content) {
    std::ifstream file(file_path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(content) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// 测试日志模块初始化
TEST(LogTest, InitLog) {


    ASSERT_TRUE(Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/test_log.log", 0, 1024, 100, 0))
                                << "Failed to initialize the log module.";
    std::ifstream log_file("/home/adminer/Projects/TinyWebserver/test_file/test_log.log");
    ASSERT_TRUE(log_file.is_open()) << "Log file not created after initialization.";
}

// 测试同步日志写入
TEST(LogTest, WriteSyncLog) {
    Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/sync_test.log", 0, 1024, 100, 0);
    LOG_INFO("Test sync log message.");
    Log::getInstance()->flush(); // 确保写入完成

    // 检查日志内容
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/sync_test.log", "Test sync log message."))
                                << "Log file does not contain the expected log message.";
}

// 测试异步日志写入
TEST(LogTest, WriteAsyncLog) {
    Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/async_test.log", 0, 1024, 100, 10); // 启用异步模式
    LOG_INFO("Test async log message.");
    Log::getInstance()->flush(); // 强制刷新队列和文件流

    // 检查日志内容
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/async_test.log", "Test async log message."))
                                << "Log file does not contain the expected async log message.";
}

// 测试日志文件分割功能
TEST(LogTest, FileSplitLog) {
    Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/split_test.log", 0, 1024, 2, 0); // 设置每个日志文件最多 2 行

    // 写入超过 2 行的日志
    LOG_INFO("Test log message 1.");
    LOG_INFO("Test log message 2.");
    LOG_INFO("Test log message 3.");

    Log::getInstance()->flush(); // 确保写入完成

    // 检查是否生成了新文件
    std::ifstream old_file("/home/adminer/Projects/TinyWebserver/test_file/split_test.log");
    ASSERT_TRUE(old_file.is_open()) << "Old log file not found.";
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/split_test.log", "Test log message 3."))
                                << "New log file does not contain the expected log message.";
}

// 测试日志刷新功能
TEST(LogTest, FlushLog) {
    Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/flush_test.log", 0, 1024, 100, 0);
    LOG_INFO("Test flush log message.");

    // 检查刷新前是否写入
    std::ifstream log_file("/home/adminer/Projects/TinyWebserver/test_file/flush_test.log");
    ASSERT_FALSE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/flush_test.log", "Test flush log message."))
                                << "Log message written before flush.";

    // 调用 flush
    Log::getInstance()->flush();

    // 检查刷新后是否写入
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/flush_test.log", "Test flush log message."))
                                << "Log message not written after flush.";
}

// 测试日志级别
TEST(LogTest, LogLevel) {
    Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/level_test.log", 0, 1024, 100, 0);

    LOG_DEBUG("This is a DEBUG message.");
    LOG_INFO("This is an INFO message.");
    LOG_WARN("This is a WARN message.");
    LOG_ERROR("This is an ERROR message.");

    Log::getInstance()->flush(); // 确保写入完成

    // 检查每种级别的日志是否写入文件
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/level_test.log", "[DEBUG] This is a DEBUG message."))
                                << "DEBUG log not written.";
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/level_test.log", "[INFO] This is an INFO message."))
                                << "INFO log not written.";
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/level_test.log", "[WARN] This is a WARN message."))
                                << "WARN log not written.";
    ASSERT_TRUE(file_contains("/home/adminer/Projects/TinyWebserver/test_file/level_test.log", "[ERROR] This is an ERROR message."))
                                << "ERROR log not written.";
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
