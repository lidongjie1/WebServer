//
// Created by Admin on 2024/12/9.
//

#ifndef TINYWEBSERVER_SQL_CONNECTION_POOL_H
#define TINYWEBSERVER_SQL_CONNECTION_POOL_H
#include <iostream>
#include <list>
#include <mysql/mysql.h>
#include <mutex>
#include <condition_variable>
#include <string>
#include "../Log/log.h"

class ConnectionPool{
public:
    //获取mysql连接
    MYSQL *getConnection();

    //释放数据库连接
    bool releaseConnection(MYSQL *conn);

    //获取当前连接数
    int getFreeConn();

    //销毁线程池
    void destroyPool();

    //获取数据库对象（单例模式）
    static ConnectionPool *getInstance();

    void init(const std::string &url, const std::string &User, const std::string &PassWord, const std::string &DataBaseName, int Port, int MaxConn, int close_log);

private:
    ConnectionPool();
    ~ConnectionPool();

private:
    int m_MaxConn;                  // 最大连接数
    int m_CurConn;                  // 当前已使用的连接数
    int m_FreeConn;                 // 当前空闲的连接数
    std::list<MYSQL *> m_connList;  // 连接池
    std::mutex m_mutex;
    std::condition_variable m_cond;

    std::string m_url;              // 主机地址
    int m_Port;                     // 数据库端口号
    std::string m_User;             // 用户名
    std::string m_PassWord;         // 密码
    std::string m_DatabaseName;     // 数据库名
    int m_close_log;                // 日志开关
};

class ConnectionRAII {
public:
    ConnectionRAII(MYSQL **con, ConnectionPool *connPool);
    ~ConnectionRAII();

private:
    MYSQL *conRAII;
    ConnectionPool *poolRAII;
};
#endif //TINYWEBSERVER_SQL_CONNECTION_POOL_H
