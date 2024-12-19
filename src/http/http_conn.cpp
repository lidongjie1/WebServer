//
// Created by Admin on 2024/12/10.
//
#include "../include/http/http_conn.h"
#include <cassert>
#include <unistd.h> // close

std::atomic<int> HttpConn::userCount_ = 0;
bool HttpConn::isET_ = false;
std::string HttpConn::src_dir_;


HttpConn::HttpConn() : fd_(-1), isClose(true), iovCnt_(0){}

HttpConn::~HttpConn(){
    Close();
}

//初始化函数
void HttpConn::Init(int sockFd, const sockaddr_in &addr) {
    assert(sockFd > 0);
    userCount_++;
    fd_ = sockFd;
    addr_ = addr;
    readBuffer_.RetrieveAll();
    writeBuffer_.RetrieveAll();
    isClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIp().c_str(), GetPort(), (int)userCount_);
}

void HttpConn::Close() {
    response_.UnmapFile();//解除映射
    if(!isClose){
        isClose = true;
        userCount_--;
        close(fd_);//关闭文件描述符
        LOG_INFO("Client[%d](%s:%d) disconnected, Remaining Users: %d", fd_, GetIp().c_str(), GetPort(), (int)userCount_);  // 记录日志
    }
}

int HttpConn::GetFd() const {
    return fd_;
}

sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

std::string HttpConn::GetIp() const {
    return inet_ntoa(addr_.sin_addr);//inet_ntoa转字符串
}

int HttpConn::GetPort() const {
    return htons(addr_.sin_port);// ntohs转端口号
}

bool HttpConn::IsKeepAlive()  {
    return request_.IsKeepAlive();
}

void HttpConn::SetSrcDir(const std::string &dir) {
    src_dir_ = dir;
}

void HttpConn::SetETMode(bool isET) {
    isET_ = isET;
}

//计算待写入的字节数
int HttpConn::ToWriteBytes() const {
    return iov_[0].iov_len + iov_[1].iov_len;
}


ssize_t HttpConn::Read(int *saveErrno) {
    ssize_t len = -1;
    do{
        len = readBuffer_.ReadFd(fd_,saveErrno);
        if(len <= 0){
            break;
        }
    } while (isET_);//ET模式缓冲区有数据就会一直触发
    return len;
}

ssize_t HttpConn::Write(int *saveErrno) {
    ssize_t len = -1;
    do{
        len = writev(fd_,iov_,iovCnt_);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len == 0) break;//数据已经全部写入
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {//数据超出第一段
            //确保缓冲区是连续
            iov_[1].iov_base = (uint8_t*)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            writeBuffer_.RetrieveAll(); // 清空缓存区域,因为第一段已经写入完毕（保证不会重复写入，数据混乱）
            iov_[0].iov_len = 0;
        }else{
            // 写入第一段数据
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuffer_.Retrieve(len);  // 从缓存中读取已写入的字节数
        }
    } while (isET_ || ToWriteBytes() > 10240);
    return len;
}

//处理请求(这里要结合线程池处理)
bool HttpConn::Process() {
    request_.Init(); //请求初始化
    if(readBuffer_.ReadableBytes() <= 0) return false; //没有可读数据
    //解析请求
    if(request_.parse(readBuffer_)){
        LOG_DEBUG("HTTP Request Path: %s", request_.path().c_str());  // 记录请求路径
        //初始化响应
        response_.Init(src_dir_,request_.path(),request_.IsKeepAlive(),200);
    }
    //生成响应流程
    response_.MakeResponse(writeBuffer_);

    //设置响应头
    iov_[0].iov_base = const_cast<char*>(writeBuffer_.Peek());
    iov_[0].iov_len = writeBuffer_.ReadableBytes();
    iovCnt_ = 1;

    //设置响应文件(如果文件存在，则为响应头 + 响应文件)
    if(response_.FileLen() > 0 && response_.File()){
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("Response File Size: %d, iovCnt: %d, Bytes to Write: %d",  // 记录响应信息
              response_.FileLen(), iovCnt_, ToWriteBytes());
    return true;  // 返回 true，表示处理成功
}