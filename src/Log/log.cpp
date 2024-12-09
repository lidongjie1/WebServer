//
// Created by Admin on 2024/12/6.
//
#include "../include//Log/log.h"
#include <ctime>
#include <sys/time.h>
#include <cstring>
#include <unistd.h>


Log::Log() : m_count(0), m_today(0), m_is_async(false), m_stop(false), m_log_queue(nullptr) {}

Log::~Log() {
    //异步线程关闭
    if(m_async_thread.joinable()){
        m_stop = true;
        m_cond_var.notify_all();
        m_async_thread.join();
    }
    //关闭日志前关闭文件流
    if (m_file_stream.is_open()) {
        m_file_stream.close();
    }
}


/*
 *初始化日志
 * 初始化参数、日志路径、日志时间等
 * 是否要启用异步写入
 * 打开文件流
 */

bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size) {
    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_split_lines = split_lines;
    m_buffer.resize(m_log_buf_size); //缓存区大小

    // 初始化异步队列
    if(max_queue_size > 0)
    {
        m_is_async = true;
        // m_log_queue 是一个 std::shared_ptr，指向一个以 max_queue_size 为最大容量的 BlockQueue<std::string> 对象
        m_log_queue = std::make_shared<BlockQueue<std::string>>(max_queue_size);
        //这个完善一点可以加上捕获异常
        m_async_thread = std::thread(&Log::async_write_log, this);
    }

    //获取当前时间
    time_t  now = time(nullptr);
    struct tm* sys_tm = localtime(&now);
    m_today = sys_tm->tm_mday;

    //设置日志文件路径和名称
    //strrchr函数返回一个指向字符串中最后一次出现的字符 c 的指针。如果字符 c 不在字符串中出现，函数返回 nullptr。
    const char* slash_pos = strrchr(file_name, '/');
    if (slash_pos) {
        strncpy(m_dir_name, file_name, slash_pos - file_name + 1); //获取目录名
        strncpy(m_log_name, slash_pos + 1, sizeof(m_log_name)); //获取文件名
    } else {
        strncpy(m_log_name, file_name, sizeof(m_log_name));
    }

    char full_name[256];
    snprintf(full_name, sizeof(full_name), "%s%d_%02d_%02d_%s", m_dir_name,
             sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, m_log_name);
    std::string charTostring(file_name);
    m_file_stream.open(charTostring, std::ios::out | std::ios::app);

    return m_file_stream.is_open();
}

void Log::write_log(int level, const char *format, ...) {
    //获取当前时间
    time_t  now = time(nullptr);
    struct tm* sys_tm = localtime(&now);

    //日志级别记录
    const char* level_str = nullptr;
    switch (level) {
        case 0: level_str = "[DEBUG]"; break;
        case 1: level_str = "[INFO]"; break;
        case 2: level_str = "[WARN]"; break;
        case 3: level_str = "[ERROR]"; break;
        default: level_str = "[INFO]"; break;
    }

    //日志信息写
    std::unique_lock<std::mutex> lock(m_mutex);
    m_count++;
    //检查是否是要分
    check_and_split_file();

    // 写入日志内容（时间）
    int n = snprintf(&m_buffer[0], m_log_buf_size, "%d-%02d-%02d %02d:%02d:%02d %s ",
                     sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday,
                     sys_tm->tm_hour, sys_tm->tm_min, sys_tm->tm_sec, level_str);
    //可变参数列表处理
    va_list args;
    va_start(args,format);
    //参数，具体的内容
    int m = vsnprintf(&m_buffer[0] + n, m_log_buf_size - n - 1, format, args);
    va_end(args);

    m_buffer[n + m] = '\n';
    m_buffer[n + m + 1] = '\0';

    //将字符数组转成字符串
    std::string charTostring(&m_buffer[0]);

    if (m_is_async && m_log_queue) {
        m_log_queue->push(charTostring);
    } else {
        m_file_stream << charTostring;
    }

}

//判断日志是否为当天或者是否超出最大行
void Log::check_and_split_file()
{
    time_t now = time(nullptr);
    struct tm* sys_tm = localtime(&now);
    if(m_today != sys_tm->tm_mday || m_count > m_split_lines)
    {
        m_file_stream.close();//关闭文件流
        char full_name[256];
        snprintf(full_name, sizeof(full_name), "%s%d_%02d_%02d_%s", m_dir_name,
                 sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, m_log_name);

        m_file_stream.open(full_name, std::ios::out | std::ios::app);
        m_today = sys_tm->tm_mday;
        m_count = 0;
    }
}

void Log::async_write_log() {
    //停止标志
    while (!m_stop) {
        std::string log_entry;
        if (m_log_queue->pop(log_entry)) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_file_stream << log_entry;
        }
    }
}

void Log::flush() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_file_stream.flush();
}
