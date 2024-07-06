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
    if(len > WritableBytes())
    {
        MakeSpace_(len);
    }
}

//标记已经写入指定长度的数据,移动写下标，在Append中使用
void Buffer::HasWritten(size_t len)
{
    writePos_ += len;
}

//从缓冲区读取指定长度数据
void Buffer::Retrieve(size_t len)
{
    readPos_ +=len;
}

//从缓冲区中读取数据，直到指定的结束位置
void Buffer::RetrieveUntil(const char * end)
{
    Retrieve(end- Peek());  // end指针-读指针

}

//从缓冲区读取全部数据，位置重置
void Buffer::RetrieveAll()
{
    bzero(&buffer_[0], buffer_.size()); // 覆盖原本数据
    readPos_ = writePos_ = 0;
}


//从缓冲区取出剩余可读的str
string Buffer::RetrieveAllToStr()
{
    string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

//写指针的位置
const char* Buffer::BeginWriteConst() const
{

      return &buffer_[writePos_];
}

char* Buffer::BeginWrite()
{
      return &buffer_[writePos_];
}

//将字符串追加到缓冲区中
void Buffer::Append(const char* str, size_t len) 
{
    assert(str);
    EnsureWriteable(len);   // 确保可写的长度
    std::copy(str, str + len, BeginWrite());    // 将str放到写下标开始的地方
    HasWritten(len);    // 移动写下标
}

void Buffer::Append(const string& str) {
    Append(str.c_str(), str.size());
}

void Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
}

// 将buffer中的读下标的地方放到该buffer中的写下标位置
void Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}


// fd 中读取内容并将其存储到缓冲区中
ssize_t Buffer::ReadFd(int fd, int* Errno) 
{
    char buff[65535];   // 栈区
    struct iovec iov[2]; //结构体成员为缓冲区起始地址、缓冲区长度
    size_t writeable = WritableBytes();//可写的字节
    // 分散读， 保证数据全部读完
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *Errno = errno;
    } else if(static_cast<size_t>(len) <= writeable) {   // 若len小于writable，说明写区可以容纳len
        writePos_ += len;   // 直接移动写下标
    } else {    
        writePos_ = buffer_.size(); // 写区写满了,下标移到最后
        Append(buff, static_cast<size_t>(len - writeable)); // 剩余的长度
    }
    return len;
}


// 将缓冲区中可读取的数据写入到文件描述符 fd
ssize_t Buffer::WriteFd(int fd, int* Errno) 
{
    ssize_t len = write(fd,Peek(),ReadableBytes());//可读取的长度进行写入
    if(len<0)
    {
        *Errno = errno;
        return len;
    }
    Retrieve(len);  //这里表示数据读取完了，移动读指针
    return len;

}

// 扩展空间
void Buffer::MakeSpace_(size_t len) 
{
    if(WritableBytes()+PrependableBytes()<len)  //这里指需要扩展
    {
        buffer_.resize(writePos_+len+1);
    }
    else    //将readPos与writePos之间的数据移动到开头
    {
        size_t readable = ReadableBytes();
        copy(BeginPtr_()+readPos_,BeginPtr_()+writePos_,BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == ReadableBytes());
    }
    
}