//
// Created by Admin on 2024/12/12.
//
#include <gtest/gtest.h>
#include "../include/buffer/buffer.h"

class BufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = new Buffer(1024);
    }

    void TearDown() override {
        delete buffer;
    }

    Buffer* buffer;
};

TEST_F(BufferTest, TestWritableBytes) {
    EXPECT_EQ(buffer->WritableBytes(), 1024);
}

TEST_F(BufferTest, TestReadableBytes) {
    buffer->Append("Hello", 5);
    EXPECT_EQ(buffer->ReadableBytes(), 5);
    EXPECT_EQ(buffer->WritableBytes(), 1019);
}

TEST_F(BufferTest, TestAppendAndRetrieve) {
    std::string data = "Hello, World!";
    buffer->Append(data);
    EXPECT_EQ(buffer->ReadableBytes(), data.size());
    EXPECT_EQ(buffer->RetrieveAllToStr(), data);
    EXPECT_EQ(buffer->ReadableBytes(), 0);
}

TEST_F(BufferTest, TestPrependableBytes) {
    EXPECT_EQ(buffer->PrependableBytes(), 0);
    buffer->Append("Hello", 5);
    buffer->Retrieve(3);
    EXPECT_EQ(buffer->PrependableBytes(), 3);
}

TEST_F(BufferTest, TestReadFd) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);
    std::string data = "Hello from pipe";
    write(pipefd[1], data.c_str(), data.size());

    int errNo = 0;
    ssize_t bytesRead = buffer->ReadFd(pipefd[0], &errNo);
    EXPECT_EQ(bytesRead, data.size());
    EXPECT_EQ(buffer->ReadableBytes(), data.size());
    EXPECT_EQ(buffer->RetrieveAllToStr(), data);

    close(pipefd[0]);
    close(pipefd[1]);
}

TEST_F(BufferTest, TestWriteFd) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);
    std::string data = "Hello to pipe";
    buffer->Append(data);

    int errNo = 0;
    ssize_t bytesWritten = buffer->WriteFd(pipefd[1], &errNo);
    EXPECT_EQ(bytesWritten, data.size());

    char readBuffer[1024] = {0};
    read(pipefd[0], readBuffer, sizeof(readBuffer));
    EXPECT_EQ(std::string(readBuffer, data.size()), data);

    close(pipefd[0]);
    close(pipefd[1]);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
