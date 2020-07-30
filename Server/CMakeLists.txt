# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

add_definitions("-Wall -std=c++11")

project ("TCP_Server")

# 包含子项目。
# add_subdirectory ("CMakeTest")
add_executable (TCP_Server  TCP_Server.cpp)