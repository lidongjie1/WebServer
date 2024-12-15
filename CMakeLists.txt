# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.25)

# 设置项目名称
project(TinyWebServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加子目录（Google Test）
add_subdirectory(gtest)

INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(/usr/include)
# 包含头文件目录
# 推荐使用 target_include_directories，而不是全局 include_directories
#include_directories(${CMAKE_SOURCE_DIR}/include)

# 搜集所有源文件
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 添加可执行文件
add_executable(TinyWebServer main.cpp ${SRC_FILES})
target_include_directories(TinyWebServer PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 链接需要的库
#target_link_libraries(TinyWebServer PRIVATE pthread gtest_main)