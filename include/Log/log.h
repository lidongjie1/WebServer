//
// Created by Admin on 2024/12/6.
//

#ifndef TINYWEBSERVER_LOG_H
#define TINYWEBSERVER_LOG_H

#include "./block_queue.h"
#include <iostream>
#include <string>
#include <fstream>
#include <cstdarg>
#include <mutex>
#include <memory>
#include <thread>

class Log{
public:
    static Log* getInstance()
    {
        static Log instance;
        return &instance;
    }

    // 初始化日志模块()
    bool init(const char* file_name, int close_log, int log_buf_size = 8192,
              int split_lines = 5000000, int max_queue_size = 0);

    // 异步写日志线程
    static void write_log_thread()
    {
        Log::getInstance()->async_write_log();
    }

    //同步写
    void write_log(int level, const char* format, ...);

    // 刷新日志到文件
    void flush();

    // 日志文件分割
    void check_and_split_file();

    bool is_log_closed() const { return m_close_log; }

private:
    Log();
    ~Log();
    //异步写
    void async_write_log();
private:
    char m_dir_name[128];    // 路径名
    char m_log_name[128];    // 日志文件名
    int m_split_lines;       // 日志最大行数
    int m_log_buf_size;      // 日志缓冲区大小
    std::vector<char> m_buffer;           // 动态日志缓冲区
    long long m_count;       // 当前日志行数
    int m_today;             // 按日期分类，记录当前日期
    std::ofstream m_file_stream; // 日志文件流
    std::shared_ptr<BlockQueue<std::string>> m_log_queue; // 日志队列
    std::thread m_async_thread;           // 异步写线程
    bool m_is_async;         // 是否异步
    std::mutex m_mutex;      //
    std::condition_variable m_cond_var;  // 条件变量
    int m_close_log;         // 是否关闭日志
    bool m_stop;

};


#define LOG_DEBUG(format, ...) if (!Log::getInstance()->is_log_closed()) { Log::getInstance()->write_log(0, format, ##__VA_ARGS__); Log::getInstance()->flush(); }
#define LOG_INFO(format, ...) if (!Log::getInstance()->is_log_closed()) { Log::getInstance()->write_log(1, format, ##__VA_ARGS__); Log::getInstance()->flush(); }
#define LOG_WARN(format, ...) if (!Log::getInstance()->is_log_closed()) { Log::getInstance()->write_log(2, format, ##__VA_ARGS__); Log::getInstance()->flush(); }
#define LOG_ERROR(format, ...) if (!Log::getInstance()->is_log_closed()) { Log::getInstance()->write_log(3, format, ##__VA_ARGS__); Log::getInstance()->flush(); }
#endif //TINYWEBSERVER_LOG_H
