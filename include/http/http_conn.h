//
// Created by Admin on 2024/12/10.
//

#ifndef TINYWEBSERVER_HTTP_CONN_H
#define TINYWEBSERVER_HTTP_CONN_H
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in

#include <string>
#include <atomic>
#include <memory>
#include <optional>
#include <filesystem>    // C++17 filesystem
#include "../Log/log.h"
#include "../mysqlPool/sql_connection_pool.h"
#include "../buffer/buffer.h"
#include "http/HttpResponse.h"
#include "http/HttpRequest.h"


class HttpConn{
public:

    HttpConn();
    ~HttpConn();

    void Init(int sockFd, const sockaddr_in& addr);

    ssize_t Read(int* saveErrno);
    ssize_t Write(int* saveErrno);

    int GetFd() const;
    int GetPort() const;
    std::string GetIp() const;
    sockaddr_in GetAddr() const;

    void Close();

    bool Process();

    int ToWriteBytes() const;
    bool IsKeepAlive() ;

    static void SetSrcDir(const std::string& dir);
    static void SetETMode(bool isET);
    Buffer& GetWriteBuffer(){ return writeBuffer_;}

private:
    int fd_;
    sockaddr_in addr_;  //客户端地址
    std::atomic<bool> isClose;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuffer_;
    Buffer writeBuffer_;

    HttpRequest request_;
    HttpResponse response_;

    static std::atomic<int> userCount_;
    static bool isET_;
    static std::string src_dir_;
};
#endif //TINYWEBSERVER_HTTP_CONN_H
