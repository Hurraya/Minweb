// 业务代码，只写路由逻辑
#include"HttpServer.h"
#include"DBManager.h"

// 全局的数据管理器

DBManager db;
int main(){
    
    // 1.初始化数据库连接
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
        res.headers["Context-Type"] = "text/html;charset=utf-8";

        // 需要把body部分取出，
        std::string user_str = "username=";
        std::string pass_str = "password=";
        std::string username = "";
        std::string password = "";

        size_t u_pos = req.body.find(user_str);
        size_t p_pos = req.body.find("&"+pass_str);

        if (u_pos != std::string::npos && p_pos != std::string::npos){
            username = req.body.substr(u_pos + user_str.length(), p_pos - (u_pos + user_str.length()));
            password = req.body.substr(p_pos + pass_str.length() + 1);
        }

        if (username.empty() || password.empty()){
            res.body = "<h1>wrong argument...</h>";
            return;
        }
        // 存入数据库
        if (db.registerUser(username, password)){
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

    // 启动服务
    app.run();

    return 0;
}