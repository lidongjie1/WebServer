#include "buffer.h"

// 读写下标初始化，vector<char>初始化
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0)  {}  


// 可读的数量： 写下标 - 读下标
size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}

// 可写的数量： buffer.size - 写下标

size_t  Buffer::WritableBytes() const
{
    return buffer_.size() - writePos_;
}

// 可预留空间：已经读过的就没用了，等于读下标
size_t  Buffer::PrependableBytes() const
{
    return readPos_;
}

//返回可读取数据的指针
const char* Buffer::Peek() const
{
    return &buffer_[readPos_];
}

//确保缓冲区有足够的可写空间，以存储指定长度的数据
void Buffer::EnsureWriteable(size_t len)
{

}

//标记已经写入指定长度的数据,移动写下标，在Append中使用
void Buffer::HasWritten(size_t len)
{

}

//从缓冲区读取指定长度数据
void Buffer::Retrieve(size_t len)
{

}

//从缓冲区中读取数据，直到指定的结束位置
void Buffer::RetrieveUntil(const char * end)
{


}

//从缓冲区读取全部数据，位置重置
void Buffer::RetrieveAll()
{

}


//从缓冲区取出剩余可读的str
string Buffer::RetrieveAllToStr()
{


}

//写指针的位置
const char* Buffer::BeginWriteConst() const
{


}

char* Buffer::BeginWrite()
{

}

//将字符串追加到缓冲区中
void Buffer::Append(const char* str, size_t len) 
{
   
}
void Buffer::Append(const string& str) 
{
   
}

void Append(const void* data, size_t len) 
{
   
}
// 将buffer中的读下标的地方放到该buffer中的写下标位置
void Append(const Buffer& buff) 
{
   
}

// 将fd的内容读到缓冲区，即writable的位置
ssize_t Buffer::ReadFd(int fd, int* Errno) 
{

}


// 将buffer中可读的区域写入fd中
ssize_t Buffer::WriteFd(int fd, int* Errno) 
{

}

// 扩展空间
void Buffer::MakeSpace_(size_t len) 
{
    
}