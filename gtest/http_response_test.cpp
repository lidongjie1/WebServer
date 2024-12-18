//
// Created by Admin on 2024/12/17.
//
#include <gtest/gtest.h>
#include "../include/http/HttpResponse.h"
#include "../include/buffer/buffer.h"
#include <filesystem>
#include <cstring>

class HttpResponseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时目录和测试文件
        std::string path = "/home/adminer/Projects/TinyWebserver/test_file/http_response_test";
        test_dir_ = std::filesystem::path(path);

        std::ofstream file_ok(test_dir_ / "test.html");
        file_ok << "<html><body>Test OK</body></html>";
        file_ok.close();

        std::ofstream file_forbidden(test_dir_ / "forbidden.html");
        file_forbidden << "<html><body>Forbidden</body></html>";
        file_forbidden.close();
        //移除forbidden.html的所有访问权限
        std::filesystem::permissions(test_dir_ / "forbidden.html", std::filesystem::perms::none);
    }

    void TearDown() override {
        // 清理临时目录和文件
//        std::filesystem::remove_all(test_dir_);
    }

    std::filesystem::path test_dir_;
};

TEST_F(HttpResponseTest, HandleValidFile) {
    HttpResponse response;
    Buffer buffer;
    response.Init(test_dir_.string(), "test.html", true, 200);
    response.MakeResponse(buffer);

    std::string response_str = buffer.RetrieveAllToStr();
    EXPECT_TRUE(response_str.find("HTTP/1.1 200 OK") != std::string::npos);
    EXPECT_TRUE(response_str.find("Content-type: text/html") != std::string::npos);
    EXPECT_TRUE(response_str.find("<html><body>Test OK</body></html>") != std::string::npos);
}

TEST_F(HttpResponseTest, HandleNotFoundFile) {
    HttpResponse response;
    Buffer buffer;
    response.Init(test_dir_.string(), "404.html", true, -1);
    response.MakeResponse(buffer);

    std::string response_str = buffer.RetrieveAllToStr();
    EXPECT_TRUE(response_str.find("HTTP/1.1 404 Not Found") != std::string::npos);
    EXPECT_TRUE(response_str.find("File NotFound!") != std::string::npos);
}

TEST_F(HttpResponseTest, HandleForbiddenFile) {
    HttpResponse response;
    Buffer buffer;
    response.Init(test_dir_.string(), "forbidden.html", true, -1);
    response.MakeResponse(buffer);

    std::string response_str = buffer.RetrieveAllToStr();
    EXPECT_TRUE(response_str.find("HTTP/1.1 403 Forbidden") != std::string::npos);
}

TEST_F(HttpResponseTest, ErrorHtmlPage) {
    HttpResponse response;
    Buffer buffer;
    response.Init(test_dir_.string(), "nonexistent.html", true, 404);
    response.ErrorHtml();
    response.MakeResponse(buffer);

    std::string response_str = buffer.RetrieveAllToStr();
    EXPECT_TRUE(response_str.find("HTTP/1.1 404 Not Found") != std::string::npos);
    EXPECT_TRUE(response_str.find("<html><title>Error</title>") != std::string::npos);
}

TEST_F(HttpResponseTest, KeepAliveHeader) {
    HttpResponse response;
    Buffer buffer;
    response.Init(test_dir_.string(), "test.html", true, 200);
    response.MakeResponse(buffer);

    std::string response_str = buffer.RetrieveAllToStr();
    EXPECT_TRUE(response_str.find("Connection: keep-alive") != std::string::npos);
}

TEST_F(HttpResponseTest, CloseConnectionHeader) {
    HttpResponse response;
    Buffer buffer;
    response.Init(test_dir_.string(), "test.html", false, 200);
    response.MakeResponse(buffer);

    std::string response_str = buffer.RetrieveAllToStr();
    EXPECT_TRUE(response_str.find("Connection: close") != std::string::npos);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
