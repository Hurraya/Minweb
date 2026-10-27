# C++ 高并发 Web 框架 (MiniWeb)

本项目是一个基于 Linux 的高并发 Web 服务器框架，从底层 Socket 实现，未使用任何第三方 Web 框架。

### 核心技术栈
- **网络模型**：基于 `epoll` 的 I/O 多路复用，采用边缘触发（ET）与非阻塞 I/O。
- **并发模型**：手写线程池（基于 `std::mutex` 与 `std::condition_variable`），采用 Reactor 模式。
- **协议解析**：手写 HTTP 请求/响应报文解析器。
- **业务路由**：利用 `std::function` 和 Lambda 表达式实现轻量级路由注册机制。
- **数据库**：集成 MySQL C API，支持数据持久化存储（附：用户注册 Demo）。
- **构建工具**：CMake。
