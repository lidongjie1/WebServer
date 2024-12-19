//
// Created by Admin on 2024/12/13.
//

#ifndef TINYWEBSERVER_HTTPRESPONSE_H
#define TINYWEBSERVER_HTTPRESPONSE_H
#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap
#include <string>
#include <memory>
#include <filesystem>

#include "../include/Log/log.h"
#include "../include/buffer/buffer.h"

class HttpResponse{
public:
    HttpResponse();
    ~HttpResponse();
    /**
     * 初始化响应对象
     * @param src_dir 资源目录
     * @param path  请求路径
     * @param isKeepAlive
     * @param code
     */
    void Init(const std::string& src_dir, const std::string& path, bool isKeepAlive = false,int code = -1);

    /**
     * 将http响应添加到buffer
     * @param buffer
     */
    void MakeResponse(Buffer& buffer);

    //获取映射文件指针
    char * File();

    //文件大小
    size_t FileLen() const;

    //将错误内容填充到buffer
    void ErrorContent(Buffer& buffer, const std::string& message);

    //获取当前状态码
    int Code() const {return code_;}

    //解除文件映射
    void UnmapFile();

    //根据状态码生成对应的错误页码路径
    void ErrorHtml();

private:
    //添加状态行到响应
    void AddStateLine_(Buffer& buffer);

    //添加头部响应
    void AddHeader_(Buffer& buffer);

    //添加响应数据内容到buffer
    void AddContent_(Buffer& buffer);



    //根据文件路径返回文件类型
    std::string GetFileType();
private:
    int code_;
    bool isKeepAlive_;
    std::string path_;
    std::string src_dir_;
    char* mmFile_; // 使用智能指针管理的文件映射内存
    std::filesystem::file_status mmFileStat_; //文件状态信息

    // 文件后缀 -> 文件类型映射
    static const std::unordered_map<std::string ,std::string> SUFFIX_TYPE;

    //状态码 -> 状态描述符映射
    static const std::unordered_map<int , std::string> CODE_STATUS;

    //状态码 -> 错误页面映射
    static const std::unordered_map<int , std::string> CODE_PATH;

};


#endif //TINYWEBSERVER_HTTPRESPONSE_H
