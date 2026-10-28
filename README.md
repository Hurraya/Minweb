# 🚀 MiniWeb - C++ 高并发 Web 框架

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Build](https://img.shields.io/badge/Build-CMake-brightgreen.svg)

本项目是一个从底层 Socket 手写实现的**高并发 Web 服务器框架**，未使用任何第三方 Web 框架（如 Nginx、Drogon 等）。框架采用 Reactor 模式，结合 Epoll 多路复用与线程池，能够高效处理大量并发网络请求，并集成了 MySQL 与 Redis 实现完整的业务闭环。

## ✨ 核心技术栈

- **网络模型**：基于 `epoll` 的 I/O 多路复用，采用**边缘触发（ET）**与**非阻塞 I/O**，单线程即可支撑高并发连接。
- **并发模型**：手写**线程池**（基于 `std::mutex` 与 `std::condition_variable`），采用 **Reactor 模式**，主线程只负责网络 I/O，业务逻辑交由线程池处理，避免阻塞。
- **协议解析**：手写轻量级 HTTP 请求/响应报文解析器，支持处理 Header 与 Body 分离。
- **业务路由**：利用 `std::function` 和 Lambda 表达式实现极简路由注册机制。
- **数据持久化**：集成 `MySQL C API` 与 `hiredis`。
- **鉴权机制**：实现基于 `Cookie-Session` 的用户注册与登录鉴权闭环（Session 存储在 Redis 中）。

## 🏗️ 项目结构

```text
MiniWeb/
├── main.cpp           # 业务代码（路由注册、业务逻辑）
├── HttpServer.h       # 框架核心类声明（Request/Response/Server）
├── HttpServer.cpp     # 框架核心实现（Epoll 事件循环、HTTP 解析）
├── DBManager.h        # 数据库管理器声明
├── DBManager.cpp      # MySQL 与 Redis 连接及操作实现
├── ThreadPool.h       # 手写线程池实现
├── CMakeLists.txt     # CMake 构建脚本
└── README.md          # 项目说明文档

## 📸 运行截图
![images/screenshort.png]

## 🚀 快速开始
### 1. 环境依赖

- Linux 操作系统 (推荐 Ubuntu)
- 支持 C++17 的编译器 (GCC 9+)
- CMake (3.10+)
- MySQL 服务端及开发库 (libmysqlclient-dev)
- Redis 服务端及开发库 (libhiredis-dev)

在 Ubuntu 下，可以一键安装依赖：
```bash
sudo apt update
sudo apt install build-essential cmake libmysqlclient-dev libhiredis-dev redis-server
```
### 2.编译运行
```bash
# 1. 创建并进入 build 目录
mkdir build && cd build
# 2. 生成构建文件并编译
cmake ..
make
# 3. 启动服务器
./webapp
```

### 3.测试访问
打开浏览器，访问 http://localhost:8080，即可体验注册与登录功能。

### 🧠 后续优化方向 (TODO)

    □ 实现数据库连接池，避免多线程竞争单一 MySQL 连接。
    □ 增加 Keep-Alive 长连接支持，减少 TCP 握手开销。
    □ 完善 HTTP Header 解析，支持正则表达式路由。
    □ 使用 epoll 的 EPOLLOUT 事件优化大文件发送（零拷贝）

### 🤝 贡献与反馈

欢迎提交 Issue 或 Pull Request！如果这个项目对你有帮助，请点个 ⭐ Star！