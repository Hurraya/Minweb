#include"DBManager.h"

DBManager::DBManager(){
    conn = mysql_init(nullptr);
    redis_conn = nullptr;       // 初始化
}

DBManager::~DBManager(){
    if (conn) mysql_close(conn);
    if (redis_conn) redisFree(redis_conn);
}

bool DBManager::init(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name){
    // 1.连接sql
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),db_name.c_str(),3306, nullptr, 0)){
        std::cerr << "failed connected to MYSQL" << mysql_error(conn) << std::endl;
        return false;
    }
    // 2.连接redis
    redis_conn = redisConnect("127.0.0.1", 6379);
    if (redis_conn == nullptr || redis_conn->err){
        std::cerr << "Failed to cinnect redis";
        return false;
    }
    return true;
}

bool DBManager::registerUser(const std::string& username, const std::string& password){
    // 构造插入SQL(我们这里使用的简单的字符串拼接。正常需要预编译，防止SQL注入)
    // 1.存入Mysql
    std::string sql = "INSERT INTO users(username, password) VALUES ('"+username + "',' " + password + "')";

    if (mysql_query(conn, sql.c_str()) != 0){
        std::cerr << "failed register(wrong in SQL): " << mysql_error(conn) << std::endl;
        return false;
    }

    // 2.存入redis
    std::string redis_key = "user:" + username;
    redisReply* reply = (redisReply*)redisCommand(redis_conn, "SET %s %s", redis_key.c_str(), password.c_str());
    if (reply != nullptr){
        std::cout << "[redis] successfully in redis!!" << redis_key << std::endl;
        freeReplyObject(reply);
    }

    return true;
}

std::string DBManager::getCachedUser(const std::string& username){
    std::string redis_key = "user: " + username;
    redisReply* reply = (redisReply*)redisCommand(redis_conn, "GET %s", redis_key.c_str());

    std::string result = "";
    if (reply != nullptr && reply->type == REDIS_REPLY_STRING){
        result = reply->str;
    }

    if (reply) freeReplyObject(reply);
    return result;
}