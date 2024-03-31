#定义变量
set(SRC_FILE memtable.cc skiplist.cc memtable_test.cc)

#DEBUG
set(CMAKE_BUILD_TYPE "Debug")

#指定C++编译标准
set(CMAKE_CXX_STANDARD 20)

#指定输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/)

#指定使用的cmake的最低版本
cmake_minimum_required(VERSION 3.0)

#指定工程名字等信息
project(cmake_demo)

#定义工程会生成可执行程序
#add_executable(<可执行程序名> <...源文件>)
add_executable(memtable_test ${SRC_FILE})
