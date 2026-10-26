// 框架的具体实现
#include<fcntl.h>
#include<cerrno>
#include "HttpServer.h"

bool HttpRequest::parse(const std::string& raw_data){
    size_t pos_method = raw_data.find(" ");
    if (pos_method == std::string::npos){
        std::cerr << "can't parse method\n"; 
        return false;
    }
    method = raw_data.substr(0, pos_method);

    size_t pos_path = raw_data.find(" ", pos_method + 1);   // 从method后面开始找
    if (pos_path == std::string::npos){
        std::cerr << "can't parse path";
        return false;
    }
    path = raw_data.substr(pos_method + 1, pos_path - pos_method -1);       // 第二个参数是“长度”


    size_t pos_body = raw_data.find("\r\n\r\n");
    if (pos_body != std::string::npos){
        body = raw_data.substr(pos_body + 4);           // +4跳过\r\n\r\n
    }else{
        body = "";
    }
    

    return true;
}

std::string HttpResponse::toString() const{
    std::string res = "HTTP/1.1 " + status + "\r\n";
    for (const auto& [key, value]: headers){
        res += key + ": " + value + "\r\n";
    }
    res += "Content-Length: " + std::to_string(body.length());
    res += "\r\n\r\n";
    res += body;
    
    return res;
}

void HttpServer::route(const std::string& path, HttpHandle handler){
    // 把path和handle存进routes字典
    routes[path] = handler;
}

void HttpServer::handleRequest(int client_fd, const std::string& raw_request){
    HttpRequest req;
    HttpResponse res;

    if (req.parse(raw_request)){
        // 查看有没有对应的路由
        if (routes.count(req.path)){    // C++20可以用contains，这里的count有返回什么？没有返回什么
            // 查表看有没有对应的路由处理函数
            routes[req.path](req, res);

        }else{
            // 没找到
            res.status = "404 Not Found";
            res.body = "<h1>404 Not Found</h1>";
        }

       
    }else{
        // 解析失败
        res.status = "400 Bad Request";
        res.body = "<h1>400 Bad Request</h1>";

    } 
    // 把组装好的响应发送给客户端
    std::string response_str = res.toString();
    write(client_fd, response_str.c_str(), response_str.length());
}

HttpServer::HttpServer(int port) : port(port){
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    epoll_fd = epoll_create1(0);

    // 解决端口占用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));// 解决刚关闭的端口，马上启动的端口占用问题
}

HttpServer::~HttpServer(){
    close(server_fd);
    close(epoll_fd);
}

void HttpServer::setNonBlocking(int fd){
    // 获取server_fd的flags，设置非阻塞
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void HttpServer::run(){
    // 核心，启动epoll server
    // 1.创建socket/欢迎socket
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->port);

    // 设置非阻塞
    setNonBlocking(server_fd);

    // 2.绑定端口
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1){
        std::cerr << "failed to bind,maybe cause the port has been used";
        return;
    }

    // 3.监听
    listen(server_fd, 10);
    
    // ========== Epoll 核心==============
    // 1.创建Epoll实例,类中的成员变量
    
    if (this->epoll_fd == -1){
        std::cerr << "epoll_creat1 failed";
        return;
    }

    // 2.把server_fd注册进入 Epoll
    struct epoll_event ev, events[1024];
    ev.events = EPOLLIN | EPOLLET;      // 关注读，与边缘触发
    ev.data.fd = server_fd;             
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev); //将server_fd注册入 epoll

    // 3. 死循环调用 epoll_wait
    std::cout << "🚀 HTTP Server running on http://localhost:" << port << "\n";
    while (true){
        std::cout << "  waiting to connect...\n";
        // 4.调用 epool_wait，这一步会blocking，-1表示无限等待（配合read、write的非阻塞
        int num_server = epoll_wait(epoll_fd, events, 1024, -1);    // events记录变动的socket

        std::cout << "Find [" << num_server << "] changed\n";

        // 5.遍历所有的有动静的socket
        for (int i = 0; i < num_server; i++){
            int current_fd = events[i].data.fd;

            // 情况A：新的client connect
            if (current_fd == server_fd){
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                // 设置非阻塞
                setNonBlocking(client_fd);
                //std::cout << "  ->new client connect. Socket FD: [" << client_fd << "]\n";

                // 把clientfd也注册入 epoll
                ev.events = EPOLLIN | EPOLLET; 
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            }
            // 情况B：如果某个客户端的动静，就是发来了数据
            else{
                
                std::string request;
                // read 这里时死循环读，边缘触发ET，配合的
                while (true){
                    char buffer[1024] = {0};
                    int bytes_read = read(current_fd, buffer, sizeof(buffer) - 1);
                    if (bytes_read > 0){
                        std::cout << "  ->recived from client:\n" ;
                        request += std::string(buffer);
                    }else if(bytes_read == -1 && errno == EAGAIN){
                        break;  // 读完了退出
                    }else{
                        // bytes_read==0,说明客户端断开了，或者出错
                        std::cout << "   ->client disconnect.Socket FD:[" << current_fd << "]\n";
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                        close(current_fd);      // 删除客户端
                        break;
                    }
                }
                if (!request.empty()){
                    handleRequest(current_fd, request);
                    // 因为我们设置了 connection::close，处理完直接关闭
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, nullptr);
                    close(current_fd);
                }
            }

        }

    }


}