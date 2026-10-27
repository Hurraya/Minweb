#include"DBManager.h"

DBManager::DBManager(){
    conn = mysql_init(nullptr);
}

DBManager::~DBManager(){
    if (conn) mysql_close(conn);
}

bool DBManager::init(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name){
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),db_name.c_str(),3306, nullptr, 0)){
        std::cerr << "failed connected to MYSQL" << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}

bool DBManager::registerUser(const std::string& username, const std::string& password){
    // 构造插入SQL(我们这里使用的简单的字符串拼接。正常需要预编译，防止SQL注入)
    std::string sql = "INSERT INTO users(username, password) VALUES ('"+username + "',' " + password + "')";

    if (mysql_query(conn, sql.c_str()) != 0){
        std::cerr << "failed register(wrong in SQL): " << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}