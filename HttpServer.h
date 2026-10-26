// 框架的核心类声明

#pragma once
#include<string>
#include<map>
#include<iostream>
#include<functional>
#include<unordered_map>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/epoll.h>

// 封装解析后的 HTTP请求
class HttpRequest{
public:
    std::string method;     // GET或POST
    std::string path;       // 比如/hello
    std::string body;       // POST的表单数据

    // 解析原始字符串
    bool parse(const std::string& raw_data);
};

// 封装要发回去的HTTP响应
class HttpResponse {
public:
    std::string status = "200 OK";  // defualt
    std::map<std::string, std::string> headers;
    std::string body;

    // 把对象拼装成符合HTTP的协议字符串
    std::string toString() const;

};


using HttpHandle = std::function<void(const HttpRequest&, HttpResponse&)>;
// 框架的核心接口
class HttpServer{
public:
    HttpServer(int port);
    ~HttpServer();

    // 注册路由：当访问 path时，执行 handler
    void route(const std::string& path, HttpHandle handler);

    // 启动服务器（内部包含 epoll 死循环）
    void run();

private:
    int port;
    int server_fd;
    int epoll_fd;

    // 路由表： 路径-> 处理函数
    std::unordered_map<std::string, HttpHandle> routes;

    // 处理某个客户端发来的完整的HTTP请求
    void handleRequest(int client_fd, const std::string& raw_request);

    // 辅助函数：设置非阻塞
    void setNonBlocking(int fd); 

};