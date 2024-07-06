#ifndef BUFFER_H
#define BUFFER_H
#include<iostream>
#include<assert.h>
#include<atomic>
#include<vector>
#include<unistd.h>
#include<cstring>
#include<sys/uio.h>

using namespace std;

class Buffer{

public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;    // 返回可写入的字节
    size_t ReadableBytes() const ;  // 返回可读取的字节
    size_t PrependableBytes() const; // 返回可预留的字节

    const char* Peek() const;   // 返回缓冲区中可读数据的指针
    void EnsureWriteable(size_t len); // 确保缓冲区有足够的可写空间，以存储指定长度的数据
    void HasWritten(size_t len);    // 标记已经写入指定长度的数据,移动写下标，在Append中使用

    void Retrieve(size_t len);  // 从缓冲区读取指定长度数据,移动下标
    void RetrieveUntil(const char* end);    // 从缓冲区中读取数据，直到指定的结束位置

    void RetrieveAll();     // 从缓冲区读取全部数据，位置重置
    string RetrieveAllToStr(); //从缓冲区取出剩余可读的str

    const char* BeginWriteConst() const;    // 返回一个指向可写数据的常量指针
    char* BeginWrite(); // 返回一个指向可写数据的指针

    void Append(const string& str); // 将字符串追加到缓冲区中
    void Append(const char* str, size_t len); // 将指定长度的字符数组追加到缓冲区中
    void Append(const void* data, size_t len);// 将指定长度的数据追加到缓冲区中
    void Append(const Buffer& buff); //  将buffer中的读下标的地方放到该buffer中的写下标位置

    ssize_t ReadFd(int fd, int* Errno); //从文件描述符中读取数据到缓冲区中
    ssize_t WriteFd(int fd, int* Errno); // 将缓冲区中的数据写入到文件描述符中

private:
    char* BeginPtr_();  // buffer开头，返回指向缓冲区开头的指针。它用于获取缓冲区的起始位置
    const char* BeginPtr_() const;// 后面的const表示为const成员函数
    void MakeSpace_(size_t len); //用于在缓冲区中创建足够的空间以容纳指定长度的数据

    vector<char> buffer_;  // 用于存储实际的数据
    atomic<size_t> readPos_;  // 原子类型数据成员，读的下标
    atomic<size_t> writePos_; // 原子类型数据成员，写的下标
};
#endif