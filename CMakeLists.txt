# 1.规定CMake的最小版本
cmake_minimum_required(VERSION 3.10)

# 2.设置项目名称
project(MinWeb)

# 3.设置C++标准（必须大于等于 C++11，用到了lambda和结构化绑定）
set(CMAKE_CXX_STANDARD 17)      # 告诉编译器想用c++17，并不是强制的如果电脑上没有，CMake可能就会悄悄降级
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 必须强制使用指定标准，不允许编译器退回到更老的版本
set(CMAKE_CXX_EXTENSIONS OFF)       # 禁止使用编译器特有的扩展语法（保证代码跨平台

# 告诉Cmake去找Mysql库
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# 设置头文件包含路径
include_directories(${MYSQL_INCLUDE_DIRS})

# 4.告诉CMake：我要一个可执行文件（名字 webapp
#它由哪些源文件组成

add_executable(webapp
    main.cpp
    HttpServer.cpp
    DBManager.cpp
    ThreadPool.h    # 头文件不需要编译，但是列出来方便看
)

# 链接MySQL库
target_link_libraries(webapp ${MYSQL_LIBRARIES} hiredis)