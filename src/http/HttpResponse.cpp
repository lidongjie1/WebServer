//
// Created by Admin on 2024/12/13.
//

#include "../include/http/HttpResponse.h"
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <filesystem>

using  namespace std;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
        {".html", "text/html"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/nsword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css "},
        {".js", "text/javascript "}
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
        { 400, "/400.html" },
        { 403, "/403.html" },
        { 404, "/404.html" },
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
        { 200, "OK" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
};


HttpResponse::HttpResponse() : code_(-1), isKeepAlive_(false), mmFile_(nullptr) {

}

HttpResponse::~HttpResponse() {
    //解除文件映射
    UnmapFile();
}

void HttpResponse::UnmapFile() {
    if (mmFile_) {
        auto fileSize = std::filesystem::file_size(std::filesystem::path(src_dir_) / path_);
        munmap(mmFile_.get(), fileSize);
        mmFile_.reset();
    }
}

//初始化响应对象
void HttpResponse::Init(const std::string &src_dir, const std::string &path, bool isKeepAlive, int code) {
    if(src_dir.empty())
    {
        throw std::invalid_argument("Source directory cannot be empty");
    }
    if(mmFile_)
    {
        UnmapFile();
    }
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    src_dir_ = src_dir;
    mmFile_.reset();
    mmFileStat_ = std::filesystem::file_status{}; // Reset file status
}

//生成响应内容填冲buffer
void HttpResponse::MakeResponse(Buffer &buffer) {
    try{
        // 检查文件是否存在以及是否为目录
        std::filesystem::path fullPath = std::filesystem::path(src_dir_) / path_;
        mmFileStat_ = std::filesystem::status(fullPath);  // 更新状态信息
        if(!std::filesystem::exists(fullPath) || std::filesystem::is_directory(fullPath)) {
            code_ = 404; //未找到
        }else if ((std::filesystem::status(fullPath).permissions() & std::filesystem::perms::others_read) == std::filesystem::perms::none) {
                code_ = 403; // 文件没有读取权限
        }else if(code_ == -1)
        {
            code_ = 200;//成功访问
        }
    }catch (const std::exception& e){
        LOG_ERROR("Exception while processing response: %s", e.what());
    }
    //把内容加入到缓存区
    ErrorHtml();
    AddStateLine_(buffer);
    AddHeader_(buffer);
    AddContent_(buffer);
}


// 获取文件内容指针
char *HttpResponse::File() {
    return mmFile_.get();
}

// 获取文件大小
size_t HttpResponse::FileLen() const {
    return std::filesystem::file_size(std::filesystem::path(src_dir_) / path_);
}

//错误页面内容
void HttpResponse::ErrorHtml() {
    if(CODE_PATH.count(code_) == 1){
        path_ = CODE_PATH.find(code_)->second;
        std::filesystem::path fullPath = std::filesystem::path(src_dir_) / path_;
        if(std::filesystem::exists(fullPath)){
            mmFileStat_ = std::filesystem::status(fullPath);//更新文件的状态
        }
    }
}

//添加状态行
void HttpResponse::AddStateLine_(Buffer &buffer) {
    string status = (CODE_STATUS.count(code_) == 1) ? CODE_STATUS.at(code_) : "Bad Request";
    buffer.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

//添加头
void HttpResponse::AddHeader_(Buffer &buffer) {
    buffer.Append("Connection: ");
    if(isKeepAlive_){
        buffer.Append("keep-alive\r\n");
        buffer.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buffer.Append("close\r\n");
    }
    buffer.Append("Content-type: " + GetFileType() + "\r\n");
}

//添加内容
void HttpResponse::AddContent_(Buffer &buffer) {
    //文件路径
    std::filesystem::path fullPath = std::filesystem::path(src_dir_) / path_;
    //这里只判断文件是否存在
    if(!std::filesystem::exists(fullPath)){
        ErrorContent(buffer,"File NotFound!");
        return;
    }
    try {
        int src_fd = open(fullPath.c_str(),O_RDONLY); //打开文件，只读取
        if(src_fd < 0){
            ErrorContent(buffer,"File NotFound!");
        }
        auto filesize = std::filesystem::file_size(fullPath);
        //这里不用指针是因为映射是一块连续的内存，指针只会释放指针的内存空间
        mmFile_ = std::make_unique<char[]>(filesize);//分配内存用于映射
        //映射区只读，私有映射，修改不会反射到原始文件
        auto* mmRet = static_cast<char*>(mmap(0,filesize,PROT_READ,MAP_PRIVATE,src_fd,0));
        if(mmRet == MAP_FAILED){
            ErrorContent(buffer,"File Mapping Failed");
            close(src_fd);
            return;
        }
        //指定映射
        mmFile_.reset(mmRet);
        close(src_fd);//映射成功关闭文件描述符

        // TODO 将映射内容加入到响应内容中（TinyWebserver原项目并未把读到内容加入到buffer）
        // TODO 测试要求（暂时将内容追加到buffer）
        // TODO 添加这个会报释放空指针错误
        buffer.Append(mmRet,filesize);

        buffer.Append("Content-length: " + to_string(filesize) + "\r\n\r\n");  // 添加内容长度头
    }catch (const std::exception& e){
        LOG_ERROR("Exception while adding content: %s", e.what());  // 打印错误日志
        ErrorContent(buffer, "File NotFound!");  // 返回错误内容
    }
}

std::string HttpResponse::GetFileType() {
    auto idx = path_.find_last_of('.'); // 查找文件后缀
    //无后缀，返回默认的类型
    if(idx == string::npos){
        return "text/plain";
    }
    string suffix = path_.substr(idx);
    return (SUFFIX_TYPE.count(suffix) ==1) ? SUFFIX_TYPE.at(suffix):"text/plain";
}

//返回错误页面
void HttpResponse::ErrorContent(Buffer &buffer, const std::string &message) {
    string body;
    body += "<html><title>Error</title>";  // HTML 标题
    body += "<body bgcolor=\"ffffff\">";  // HTML 背景颜色
    body += to_string(code_) + " : " + CODE_STATUS.at(code_)  + "\n";  // 添加状态码和状态信息
    body += "<p>" + message + "</p>";  // 错误信息
    body += "<hr><em>TinyWebServer</em></body></html>";  // 服务器签名

    buffer.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");  // 添加内容长度头
    buffer.Append(body);  // 添加错误页面内容
}