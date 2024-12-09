#set(TEST_PROJECT_NAME TinyWebSever_test)
#set(PROJECT_NAME Tiny_WebSever)
#
#project(${TEST_PROJECT_NAME})
##将gtest添加到项目中
## 关闭 gmock 功能
#set(BUILD_GMOCK OFF)
#add_subdirectory(${CMAKE_SOURCE_DIR}/googletest-1.15.2 googletest) # 将 将Google Test 的源代码目录添加为子目录
#
#add_executable(${TEST_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/gunit_test.cpp) # 创建一个可执行文件，名字是 TinyWebSever_test，源文件是当前目录下的 gunit_test.cpp。
#
#target_include_directories(${TEST_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}) # 为测试项目设置头文件搜索路径，使其能够访问主项目的头文件。
#target_link_libraries(${TEST_PROJECT_NAME} PUBLIC  GTest::gtest GTest::gtest_main) # 链接 Google Test 的库文件 GTest::gtest 和 GTest::gtest_main。
#
#add_test(NAME ${TEST_PROJECT_NAME}
#        COMMAND ${TEST_PROJECT_NAME}) # 将测试添加到 CTest 测试框架。测试的名字是 TinyWebSever_test，测试时将运行 TinyWebSever_test 可执行文件。

cmake_minimum_required(VERSION 3.25)

# 项目名称
set(PROJECT_NAME TinyWebSever_test)

# 关闭 GMock 功能，只使用 GTest
set(BUILD_GMOCK OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/googletest-1.15.2 googletest) # 添加 Google Test 子目录

# 包含头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(LogLib STATIC ${CMAKE_SOURCE_DIR}/src/Log/log.cpp)
# 定义测试源文件列表
set(TEST_SOURCES
        gunit_test.cpp
        block_queue_test.cpp
        log_test.cpp
)

# 循环为每个测试文件创建独立的目标和测试
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 提取源文件名（不带扩展名），作为测试名称
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # 创建可执行文件，命名为测试名
    add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE})

    # 链接 Google Test 和 pthread
    target_link_libraries(${TEST_NAME}
            PUBLIC
            GTest::gtest
            GTest::gtest_main
            pthread
            LogLib
    )

    # 添加该测试到 CTest 测试框架
    add_test(NAME ${TEST_NAME}
            COMMAND ${TEST_NAME})
endforeach()