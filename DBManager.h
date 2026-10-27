// 封装Mysql的增删改查
#pragma once
#include<mysql/mysql.h>
#include<string>
#include<iostream>

class DBManager{
private:
    MYSQL* conn;
public:
    DBManager();
    ~DBManager();
    bool init(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name);

    // 专门用于注册：插入新用户
    bool registerUser(const std::string& username, const std::string& password);
};