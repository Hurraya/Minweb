// 测试是否连接上hiredis
#include<iostream>
#include<hiredis/hiredis.h>


int main(){
    // 1.连接Redis（参数IP地址，端口号，超时设置）
    redisContext* context = redisConnect("127.0.0.1", 6379);

    // 检查是否连接成功
    if (context == nullptr || context->err){
        if (context){
            std::cerr << "Failed connect: " << context->errstr << std::endl;
        }else{
            std::cerr << "can't allocate Redis context" << std::endl; 
        }
        return 1;
    }
    std::cout << "✅ successdully connect Redis!" << std::endl;
    
    // 2. 执行 SET 命令（相当于SQL的INSERT
    redisReply* reply = (redisReply*)redisCommand(context, "SET username %s", "heeee");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS){
        std::cout << "SET 结果： " << reply->str << std::endl;  // 应该返回 ok
    }
    freeReplyObject(reply);     //用完一定要释放

    // 3.执行GET命令（SELECT
    reply = (redisReply*)redisCommand(context, "GET username");
    if (reply != nullptr || reply->type == REDIS_REPLY_STRING){
        std::cout << "GET result: " << reply->str << std::endl;
    }
    freeReplyObject(reply);

    // 4.断开连接
    redisFree(context);
    return 0;
}