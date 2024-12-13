//
// Created by Admin on 2024/12/12.
//
#include "../include/buffer/buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}


size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}


size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::PrependableBytes() const {
    return readPos_;
}

void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len)
    {
        MakeSpace_(len);
    }
}

void Buffer::HasWritten(size_t len) {
    assert(WritableBytes() >= len);
    writePos_ += len;
}

const char *Buffer::Peek() const {

    return BeginPtr_() + readPos_;
}

void Buffer::Append(const Buffer &buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::Append(const char *data, size_t len) {
    EnsureWriteable(len);
    std::copy(data, data + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const std::string &str) {
    Append(str.data(),str.size());
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;

}
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

//更新读指针位置
void Buffer::RetrieveUntil(const char *end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}
void Buffer::RetrieveAll() {
    memset(&buffer_[0], 0, buffer_.size());  //清空缓存区
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string result(Peek(), ReadableBytes());
    RetrieveAll();
    return result;

}
/*readv 系统调用从文件描述符 fd
 * 使用 readv 和 write 可以提高文件操作的效率，因为它们允许一次性操作多个缓冲区。
 */
ssize_t Buffer::ReadFd(int fd, int *Errno) {
    char extraBuffer[65536]; // 额外的缓冲区
    struct iovec iov[2];

    size_t writeable = WritableBytes();
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;

    iov[1].iov_base = extraBuffer;
    iov[1].iov_len = sizeof(extraBuffer);

    //读取数据,如果一次超出，超出部分先放到额外缓存区，追加进buffer中
    ssize_t len = readv(fd, iov, 2);
    if(len < 0)
    {
        *Errno = errno;
    }
    else if(static_cast<size_t>(len) < writeable)                //static_cast<size_t>(len) 确保了 len 被转换为一个无符号的 size_t 类型
    {
        HasWritten(len);
    }
    else
    {
        HasWritten(len);
        Append(extraBuffer,len-writeable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int *Errno) {
    size_t readable = ReadableBytes();
    ssize_t len = write(fd,Peek(),readable);
    if (len < 0) {
        *Errno = errno;
        return len;
    }
    Retrieve(len);
    return len;
}

char *Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}


void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len){
        buffer_.resize(writePos_ + len);
    } else{
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());//如果剩下的空间够的话，就将可写与预留的空间拼接在一起
        readPos_ = 0;
        writePos_ = readable;
    }
}