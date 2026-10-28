// 封装Mysql的增删改查
#pragma once
#include<mysql/mysql.h>
#include<hiredis/hiredis.h>
#include<string>
#include<iostream>


class DBManager{
private:
    MYSQL* conn;
    redisContext* redis_conn;
public:
    DBManager();
    ~DBManager();
    bool init(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name);

    // 专门用于注册：插入新用户(同时写 MySQL和redis)
    bool registerUser(const std::string& username, const std::string& password);

    // 从redis快速查数据
    std::string getCachedUser(const std::string& username);

    // 设置session
    bool setSession(std::string& session_id, const std::string& username);

    // 查找username by session
    std::string getSession(std::string session_id);


};