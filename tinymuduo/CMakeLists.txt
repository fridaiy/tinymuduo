#指定cmake版本
cmake_minimum_required(VERSION 2.5)
#项目名称
project(tinymuduo)
#设置编译好的动态库路径 默认在根目录的Lib文件夹下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置调试信息 启动c++14标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14")
#定义参与代码编译的源文件
aux_source_directory(. SRC_LIST)
#编译生成动态库
add_library(tinymuduo SHARED ${SRC_LIST})