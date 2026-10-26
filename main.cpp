// 业务代码，只写路由逻辑
#include"HttpServer.h"

int main(){
    HttpServer app(8080);

    // 注册路由
    app.route("/", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html;  charset=utf-8";
        res.body = "<h1> 🎊welcome to your C++ web frame!!</h1> <p>this is wrote in Epoll</p>";
    });

    app.route("/hello", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html; charset=utf-8";
        res.body = "<h1>Hello Route!</h1><a href='/'>go back to home</a>";
    });

    app.route("/sleep", [](const HttpRequest& req, HttpResponse& res){
        sleep(5);   // 假装处理了5秒钟业务
        res.body = "I finally woke up!";
    });

    // 启动服务
    app.run();

    return 0;
}