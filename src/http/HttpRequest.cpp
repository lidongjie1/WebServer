//
// Created by Admin on 2024/12/13.
//

#include "../include/http/HttpRequest.h"


const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
        "/index", "/register", "/login",
        "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
        {"/register.html", 0}, {"/login.html", 1},  };


HttpRequest::HttpRequest() {
    Init();
}


void HttpRequest::Init() {
    state_ = REQUEST_LINE;
    method_ = path_ = version_ = "";
    header_.clear();
    post_.clear();
}

bool HttpRequest::ParseRequestLine(const std::string &line) {
    //正则表达式，以字符串开始，[^ ]* 表示不包括空格的任意字符序列
    //
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    //std::regex_match 函数尝试使用正则表达式 pattern 来匹配 line 字符串
    std::smatch match;
    if(std::regex_match(line,match,pattern))
    {
        //分别对应正则表达式[^ ]*
        method_ = match[1];
        path_ = match[2];
        version_ = match[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader(const std::string &line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        header_[subMatch[1]] = subMatch[2];
    } else if (line.empty()) {  // 碰到空行切换到 BODY
        state_ = BODY;
    }
}

void HttpRequest::ParseBody(const std::string &line) {
    try {
        body_ = line;
        ParsePost_();
        state_ = FINISH;
        LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
    } catch (const std::exception& e) {
        LOG_ERROR("ParseBody Exception: %s", e.what());
    }
}


//将键值对的参数用&连接起来，如果有空格，将空格转换为+加号；有特殊符号，将特殊符号转换为ASCII HEX值(POST请求编码格式 application/x-www-form-urlencoded)
std::string HttpRequest::UrlDecode(const std::string &str) {
    std::string result;
    size_t len = str.size();
    for(size_t i=0; i<len;i++){
        if(str[i] == '+'){
            result += ' ';
        }else if(str[i] == '%' && i+2<len && std::isxdigit(str[i+1]) && std::isxdigit(str[i+2]))
        {
            //解析 %xx
            char hex[3] = {str[i+1], str[i+2], '\0'};
            //strtol 函数将这个十六进制数转换为它的十进制等价值。
            result += static_cast<char>(std::strtol(hex, nullptr, 16));
            i +=2;
        }else{
            result += str[i];
        }
    }
    return result;
}

void HttpRequest::ParseFromUrlencoded_() {
    if(body_.empty()) return;

    size_t start = 0, end = 0;
    std::string key, value;
    while((end = body_.find('&',start))!= std::string::npos){
        //键值对用&隔开
        std::string pair = body_.substr(start,end-start);
        //格式为 键=值
        size_t pos = pair.find('=');
        if(pos != std::string::npos){
            key = UrlDecode(pair.substr(0,pos));
            value = UrlDecode(pair.substr(pos+1));
            post_[key] = value;
        }
        start = end +1;
    }

    //处理最后一个键值对
    if(start < body_.size())
    {
        std::string pair = body_.substr(start);
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, pos));
            value = UrlDecode(pair.substr(pos + 1));
            post_[key] = value;
        }
    }
}

//验证用户密码是否正确
bool HttpRequest::UserVerify(const std::string &name, const std::string &pwd, bool isLogin) {
    if (name.empty() || pwd.empty()) {
        LOG_WARN("Username or password is empty.");
        return false;
    }
    LOG_INFO("Verifying name: %s, pwd: [HIDDEN]", name.c_str());

    MYSQL* sql;
    //获取数据库实例
    ConnectionRAII(&sql,ConnectionPool::getInstance());
    assert(sql);

    //登录成功标志
    bool success = false;
    char query[256] = {0};
    MYSQL_RES *res = nullptr;

    //查询数据库是否有
    snprintf(query,sizeof(query),"SELECT password FROM user WHERE username='%s' LIMIT 1",name.c_str());
    LOG_DEBUG("SQL Query: %s", query);

    //查询失败，返回0表示查询成功
    if(mysql_query(sql,query))
    {
        LOG_ERROR("MySQL query failed: %s", mysql_error(sql));
        return false;
    }

    res = mysql_store_result(sql);
    if(!res)
    {
        LOG_ERROR("MySQL store result failed: %s", mysql_error(sql));
        return false;
    }
    //取结果中的一行数据
    MYSQL_ROW row = mysql_fetch_row(res);
    if(isLogin){
        if(row){
            std::string db_pwd(row[0]);
            if(pwd == db_pwd){
                success = true;
            } else{
                LOG_WARN("Incorrect password for user: %s", name.c_str());
            }
        } else{
            LOG_WARN("User not found: %s", name.c_str());
        }
    } else{ //注册用户
        if(row){
            LOG_WARN("Username already taken: %s", name.c_str());
        } else{
            snprintf(query,sizeof(query),"INSERT INTO user(username,password) VALUES('%s','%s')",name.c_str(),pwd.c_str());
            LOG_DEBUG("SQL Insert: %s", query);
            //创建用户成功
            if(mysql_query(sql,query) == 0)
            {
                success = true;
            } else{
                LOG_ERROR("MySQL insert failed: %s", mysql_error(sql));
            }
        }
    }
    mysql_free_result(res);
    //TODO 这里的释放数据库不知道有无bug,只是单纯的清除连接数
    ConnectionPool::getInstance()->getFreeConn();
    if (success) {
        LOG_INFO("User verification succeeded for user: %s", name.c_str());
    } else {
        LOG_WARN("User verification failed for user: %s", name.c_str());
    }
    return success;
}

//解析POST
void HttpRequest::ParsePost_() {
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded")
    {
        //编码解析
        ParseFromUrlencoded_();
        if(DEFAULT_HTML_TAG.count(path_)){
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1){
                //判断是否为登录页
                bool isLogin = (tag == 1);
                //验证是否正确
                if(UserVerify(post_["username"], post_["password"], isLogin)) {
                    path_ = "/welcome.html";
                }
                else {
                    path_ = "/error.html";
                }
            }
        }
    }
    //拼接静态资源路径
    //TODO 这里的路径拼接可能需要
//    path_ = STATIC_ROOT + path_;
}

void HttpRequest::ParsePath_() {
    if(path_ == "/") {
        path_ = "/index.html";
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

bool HttpRequest::parse(Buffer &buffer) {
    const char CRLF[] = "\r\n";
    if(buffer.ReadableBytes() <= 0) return false;

    while(buffer.ReadableBytes() && state_!=FINISH)
    {
        //查找结束符号
        const char *lineEnd = std::search(buffer.Peek(),buffer.BeginWriteConst(),CRLF,CRLF + 2);
        std::string line(buffer.Peek(),lineEnd);
        switch (state_) {
            case REQUEST_LINE:
                if(!ParseRequestLine(line)){
                    return false;
                }
                ParsePath_();
                break;
            case HEADERS:
                ParseHeader(line);
                if(buffer.ReadableBytes() <=2){ //处理结束，只剩下\r\n
                    state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buffer.BeginWrite()) break;
        //更新此时的读位置
        buffer.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

//TODO 待测试这里的version是否对
bool HttpRequest::IsKeepAlive() {
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

std::string HttpRequest::path() const{
    return path_;
}

std::string& HttpRequest::path(){
    return path_;
}
std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string &key) const {
    assert(key.empty());
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char *key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}


