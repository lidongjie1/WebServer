// google test for Log and HttpRequest
#include "gtest/gtest.h"
#include "../include/http/HttpRequest.h"
#include "../include/Log/log.h"
#include "../include/buffer/buffer.h"
#include "../include/mysqlPool/sql_connection_pool.h"

// Test fixture for HttpRequest class
class HttpRequestTest : public ::testing::Test {
protected:
    void SetUp() override {
        Log::getInstance()->init("/home/adminer/Projects/TinyWebserver/test_file/http_req_test.log", 0, 1024, 500, 0);
    }

    void TearDown() override {
        Log::getInstance()->flush();
    }
};

// Test: ParsePath (Indirectly through parse())
TEST_F(HttpRequestTest, ParsePath) {
    HttpRequest request;
    Buffer buffer;

    // Valid path
    buffer.Append("POST / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(request.parse(buffer));
    EXPECT_EQ(request.path(), "/index.html");

    buffer.RetrieveAll();

    buffer.Append("POST /login HTTP/1.1\r\nHost: localhost\r\n\r\n");
    request.Init();
    EXPECT_TRUE(request.parse(buffer));
    EXPECT_EQ(request.path(), "/login.html");

}

// Test: ParseRequestLine (Indirectly through parse())
TEST_F(HttpRequestTest, ParseRequestLine) {
    HttpRequest request;
    Buffer buffer;

    // Valid request line
    buffer.Append("POST /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_TRUE(request.parse(buffer));
    EXPECT_EQ(request.method(), "POST");
    EXPECT_EQ(request.path(), "/index.html");
    EXPECT_EQ(request.version(), "1.1");

    // Invalid request line
    request.Init();

    buffer.RetrieveAll();
    buffer.Append("INVALID REQUEST LINE\r\n\r\n");
    EXPECT_FALSE(request.parse(buffer));
}

// Test: ParsePost and GetPost
TEST_F(HttpRequestTest, ParsePostAndGetPost) {
    HttpRequest request;
    Buffer buffer;

    buffer.Append("POST /login HTTP/1.1\r\n");
    buffer.Append("Content-Type: application/x-www-form-urlencoded\r\n");
    buffer.Append("Content-Length: 29\r\n\r\n");
    buffer.Append("username=testuser&password=12345");

    EXPECT_TRUE(request.parse(buffer));
    EXPECT_EQ(request.GetPost("username"), "testuser");
    EXPECT_EQ(request.GetPost("password"), "12345");
}

// Test: User authentication via POST
TEST_F(HttpRequestTest, UserAuthentication) {
    HttpRequest request;
    Buffer buffer;

    // Simulate login request
    buffer.Append("POST /login HTTP/1.1\r\n");
    buffer.Append("Content-Type: application/x-www-form-urlencoded\r\n");
    buffer.Append("Content-Length: 29\r\n\r\n");
    buffer.Append("username=testuser&password=12345");
    EXPECT_TRUE(request.parse(buffer));
    // Add assertions or checks depending on how `UserVerify` is integrated.

    request.Init(); // 重置此时状态
    // Simulate registration request
    buffer.RetrieveAll();
    buffer.Append("POST /register HTTP/1.1\r\n");
    buffer.Append("Content-Type: application/x-www-form-urlencoded\r\n");
    buffer.Append("Content-Length: 29\r\n\r\n");
    buffer.Append("username=newuser&password=password");
    EXPECT_TRUE(request.parse(buffer));
    // Add assertions or checks depending on how `UserVerify` is integrated.
}

// Test: IsKeepAlive
TEST_F(HttpRequestTest, IsKeepAlive) {
    HttpRequest request;
    Buffer buffer;

    buffer.Append("POST /index.html HTTP/1.1\r\n");
    buffer.Append("Connection: keep-alive\r\n\r\n");

    EXPECT_TRUE(request.parse(buffer));
    EXPECT_TRUE(request.IsKeepAlive());

    request.Init();
    buffer.RetrieveAll();
    buffer.Append("POST /index.html HTTP/1.0\r\n");
    buffer.Append("Connection: close\r\n\r\n");

    EXPECT_TRUE(request.parse(buffer));
    EXPECT_FALSE(request.IsKeepAlive());
}

// Test: Parse (Complete Request Parsing)
TEST_F(HttpRequestTest, ParseRequest) {
    HttpRequest request;
    Buffer buffer;

    std::string rawRequest =
            "POST /index.html HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Connection: keep-alive\r\n"
            "\r\n";
    buffer.Append(rawRequest);

    EXPECT_TRUE(request.parse(buffer));
    EXPECT_EQ(request.method(), "POST");
    EXPECT_EQ(request.path(), "/index.html");
    EXPECT_EQ(request.version(), "1.1");
    EXPECT_TRUE(request.IsKeepAlive());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
