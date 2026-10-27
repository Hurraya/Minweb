#include <iostream>
#include<mysql/mysql.h> //引入MySQL

int main(){
    // 1. 初始化MySQL连接对象
    MYSQL* conn = mysql_init(nullptr);
    if (conn == nullptr){
        std::cerr << "mysql_init failed " << std::endl;
        return 1;
    }

    // 2.连接数据库
    // 参数：连接对象，主机名、用户名、密码、数据库名，端口，socket、flags
    if (mysql_real_connect(conn, "localhost", "root", "123456", nullptr, 3306, nullptr, 0) == nullptr){
        std::cerr << "failed to connect" << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return 1;
    } 
    std::cout << "✅ successfully connect to MySQL database!!" << std::endl;

    // 3.创建数据库
    mysql_query(conn, "CREATE DATABASE IF NOT EXISTS test_db");  // 这第二个参数是什么意思，是一个SQL语句！
    // 切换到 test_db
    mysql_query(conn, "USE test_db");

    // 4.创建一张表
    std::string create_table_sql =
    "CREATE TABLE IF NOT EXISTS users ("
    "id INT AUTO_INCREMENT PRIMARY KEY,"
    "username VARCHAR(50) NOT NULL,"
    "password VARCHAR(50) NOT NULL)";
    if (mysql_query(conn, create_table_sql.c_str()) != 0){
        std::cerr << "failed to create table. " << mysql_error(conn) << std::endl;
    }else{
        std::cout << "✅ successfully create table!" << std::endl;
    }

    // 5.插入一条测试（SQL 注入测试就不防了，就测试基本功能
    std::string insert_sql = "INSERT INTO users (username, password) VALUES ('Alice', '123')";
    if (mysql_query(conn, insert_sql.c_str()) != 0){
        std::cerr << "failed insert:" << mysql_error(conn) << std::endl;

    }else{
        std::cout << "✅ successfully insert!" << std::endl;
    }

    // 6.插入数据
    mysql_query(conn, "SELECT id, username FROM users");
    MYSQL_RES* result = mysql_store_result(conn);   // 获取结果
    if (result){
        MYSQL_ROW row;
        std::cout << "--- user Lists---" << std::endl;
        // 循环读取每一行
        while((row = mysql_fetch_row(result))){
            std::cout << "ID: " << row[0] << ",Username: " << row[1] << std::endl;
        }
        mysql_free_result(result);  // 释放结果
    }

    // 7.关闭连接
    mysql_close(conn);
    std::cout << "close connect." << std::endl;
    return 0;
}