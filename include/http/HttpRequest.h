//
// Created by Admin on 2024/12/13.
//

#ifndef TINYWEBSERVER_HTTPREQUEST_H
#define TINYWEBSERVER_HTTPREQUEST_H
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cctype>
#include <sstream>
#include <regex>
#include <errno.h>
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../Log/log.h"
#include "../mysqlPool/sql_connection_pool.h"

//用于解析http报文请求
class HttpRequest{
public:
    enum PARSE_STATE{
        REQUEST_LINE,   //解析请求行
        HEADERS,        //解析请求头
        BODY,           //解析请求体
        FINISH          //解析完成
    };

    enum HTTP_CODE{
        NO_REQUEST,//请求不完整
        GET_REQUEST,//成功获取完整的客户端请求
        BAD_REQUEST,//请求有语法错误
        NO_RESOURCE,//请求资源不存在
        FORBIDDEN_REQUEST,//客户端权限不足
        FILE_REQUEST,//请求文件资源
        INTERNAL_ERROR,//服务器内部错误
        CLOSED_CONNECTION//客户端连接关闭
    };

    HttpRequest();
    ~HttpRequest() = default;
    void Init();    //重置HttpRequest对象

    bool parse(Buffer& buffer); //解析http请求报文

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() ; //是否长连接
private:
    bool ParseRequestLine(const std::string& line); //解析请求行
    void ParseHeader(const std::string& line);//解析请求头
    void ParseBody(const std::string& line);//解析请求体

    void ParsePath_();//处理路径（URL解码）
    void ParsePost_();//解析post请求参数

    //解析 HTTP POST 请求中 application/x-www-form-urlencoded 格式的数据
    //该函数负责解析 HTTP POST 请求体中的键值对数据
    void ParseFromUrlencoded_();

    std::string UrlDecode(const std::string& str);

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);
private:
    const std::string STATIC_ROOT = "../source";
    PARSE_STATE state_;  //当前解析状态
    std::string method_, path_, version_, body_; // 请求行
    std::unordered_map<std::string, std::string> header_; // 请求头
    std::unordered_map<std::string, std::string> post_; // post请求参数

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;

    //该函数用于将 URL 编码中的十六进制字符转换为整数
    static int ConverHex(char ch);
};


#endif //TINYWEBSERVER_HTTPREQUEST_H
