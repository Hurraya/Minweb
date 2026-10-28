// 业务代码，只写路由逻辑
#include"HttpServer.h"
#include"DBManager.h"
#include<random>    //生成随机数

// 辅助函数：生成一个随机的session ID
std::string generate_session_id(){
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnoqrstuvwxyz";
    std::string s;
    std::srand(std::time(nullptr));     // 给C语言的rand播种，避免每次生成的随机数一样
    for (int i = 0; i < 16; ++i){
        s += alphanum[rand() % (sizeof(alphanum) - 1)];  // (0, 62)
    }
    return s;
}


// 全局的数据管理器
DBManager db;

int main(){
    
    // 1.初始化数据库连接，redis连接
    if (!(db.init("localhost", "root", "123456", "test_db"))){
        std::cerr << "failed init,please ensure MySQL is turned on";
        return 1;
    }

    HttpServer app(8080);

    // 注册路由
    app.route("/", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html;  charset=utf-8";
        res.body = R"(
            <html><body>
                <h1>🎊welcome to C++ system</h1>
                <form action="/do_register" method="POST">
                    用户名:<input type="text" name="username" required><br><br>
                    密&nbsp;&nbsp;&nbsp;码: <input type="password" name="password" required><br><br>
                    <button type="submit">注册</button>
                </form>
            </body></html>
        )";
    });

    // 处理：post注册表单
    app.route("/do_register", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html;charset=utf-8";

        // name和password在handle里面解析了
        if (req.username.empty() || req.password.empty()){
            res.body = "<h1>wrong argument...</h>";
            return;
        }
        // 存入数据库
        if (db.registerUser(req.username, req.password)){
            res.body = "<h1> 🎉 successfully register!</h1><a href='/'>BACK</a>";
        }else{
            res.body = "<h1> failed register,username maybe repeat.</h1>";
        }

    });

    app.route("/hello", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html; charset=utf-8";
        res.body = "<h1>Hello Route!</h1><a href='/'>go back to home</a>";
    });

    app.route("/sleep", [](const HttpRequest& req, HttpResponse& res){
        sleep(5);   // 假装处理了5秒钟业务
        res.body = "I finally woke up!";
    });

    // 登录接口
    app.route("/login", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html; charset=utf-8";

        // 如果是直接访问 /login，给他返回表单
        if (req.method == "GET"){
            res.body = R"(
                <form action="/login" method="POST">
                    用户名：<input type="text" name="username"><br>
                    密码：<input type="password" name="password"><br>
                    <button type="submit">登录</button>
                </form>            
            )";
            return;
        }

        // 如果是POST 提交密码
        if (req.method == "POST"){

            // 这里没有检查密码是否正确。都按正确处理
            
            std::string session_id = generate_session_id();

            
            // 存入redis
            if (!db.setSession(session_id, req.username)){
                std::cerr << "Failed in redis";
                res.body ="<h1>Fialed setSession</h1>";
                return;
            };

            // 告诉浏览器把cookie存起来,路径为/
            res.headers["Content-Type"] = "text/html; charset=utf-8";
            res.headers["Set-Cookie"] = "session_id=" + session_id + "; Path=/;HttpOnly";
            res.body = "<h1>successfully login!! <a href='/profile'> goto personal center</a></h1>";
            return;

        }
    });

    app.route("/profile", [](const HttpRequest& req, HttpResponse& res){
        res.headers["Content-Type"] = "text/html; charset=utf-8";

        
        std::cout << "\n=== 收到 /profile 请求的 Headers ===" << std::endl;
        for (const auto& pair : req.headers) {
            std::cout << pair.first << " : " << pair.second << std::endl;
        }

        // 提取出session_id
        if (req.headers.count("Cookie") == 0){
            res.status = "401 Unauthorized";
            res.body = "<h1>Please login! <a href='/login'>goto login</a><h1>";
            return;
        }
        std::string cookie = req.headers.at("Cookie");
        // cookie可能是 session_id=xxxx;other=yyy
        size_t pos = cookie.find("session_id=");

        if (pos == std::string::npos){
            res.status = "401 Unauthorized";
            res.body = "<h1>Cookie error!</h1>";
            return;
        }

        size_t end_pos = cookie.find(";", pos);
        if (end_pos == std::string::npos){
            end_pos = cookie.length();  //如果没有分号就截取到末尾
        }
        std::string session_id = cookie.substr(pos + 11, end_pos - (pos + 11));

        std::cout << "=== 解析到的 Session ID: [" << session_id << "]" << std::endl;
        // 拿session_id去redis 查用户
        std::string username = db.getSession(session_id);

        if (username.empty()){
            res.status = "401 Unauthorized";
            res.body = "<h1>session expied! <a href='/login'>goto login</a></h1>";
        }else{
            res.body = "<h1>welcome to personal center," + username + "!<h1>";
        }

    });

    // 启动服务
    app.run();

    return 0;
}