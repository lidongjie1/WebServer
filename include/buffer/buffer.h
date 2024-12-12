//
// Created by Admin on 2024/12/12.
//

#ifndef TINYWEBSERVER_BUFFER_H
#define TINYWEBSERVER_BUFFER_H
#include <vector>
#include <atomic>
#include <string>
#include <cstring>
#include <cassert>
#include <sys/uio.h> // readv, writev
#include <unistd.h>


class Buffer{
public:

     Buffer(int initBuffSize = 1024);

    ~Buffer() = default;

    //可写字节数
    size_t WritableBytes() const;

    //可读字节数
    size_t ReadableBytes() const;

    //已经预留字节数
    size_t PrependableBytes() const;

    //返回当前数据的可读数据的起始地址
    /*
    const（在函数名之后）：这表示 Peek 函数是一个常量成员函数，它承诺不会修改 Buffer 类的任何成员变量。换句话说，调用 Peek 函数不会改变 Buffer 对象的状态。
    const（在 char* 返回类型之后）：这表示 Peek 函数返回的指针指向的数据是常量，调用者不应该通过这个指针来修改数据。这提供了一种保护，防止调用者意外或恶意地修改数据。
     */
    const char* Peek() const;

    //确保有足够的写空间
    void EnsureWriteable(size_t len);

    //标记以及写入的字节
    void HasWritten(size_t len);

    //读取指定长度的数据
    void Retrieve(size_t len);

    //清空缓存区
    void RetrieveAll();

    //讲缓存区的数据转换为字符串并清空
    std::string RetrieveAllToStr();

    //写入数据到缓存区
    void Append(const std::string& str);
    void Append(const char* data, size_t len);
    void Append(const Buffer& buff);

    //从文件描述符读取数据到缓存区
    ssize_t ReadFd(int fd, int* Errno);

    //从缓存区读数据写到文件描述符
    ssize_t WriteFd(int fd, int* Errno);
private:

    //返回一个指向缓冲区当前写入位置的指针
    char* BeginWrite();

    const char* BeginWriteConst();

    //返回一个指向缓冲区起始位置的指针
    char* BeginPtr_();

    const char* BeginPtr_() const ;
    //扩充缓冲区
    void MakeSpace_(size_t len) ;

private:
    std::vector<char> buffer_;  //缓冲区
    std::atomic<size_t> readPos_;           // 读取位置
    std::atomic<size_t> writePos_;          // 写入位置
};
#endif //TINYWEBSERVER_BUFFER_H
