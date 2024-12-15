cmake_minimum_required(VERSION 3.25)

# 项目名称
set(PROJECT_NAME TinyWebSever_test)

# 关闭 GMock 功能，只使用 GTest
set(BUILD_GMOCK OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/googletest-1.15.2 googletest) # 添加 Google Test 子目录

# 包含头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(LogLib STATIC ${CMAKE_SOURCE_DIR}/src/Log/log.cpp)
add_library(SqlLib STATIC ${CMAKE_SOURCE_DIR}/src/mysqlPool/sql_connection_pool.cpp)
add_library(BuflLib STATIC ${CMAKE_SOURCE_DIR}/src/buffer/buffer.cpp)
add_library(HttpRequestLib STATIC ${CMAKE_SOURCE_DIR}/src/http/HttpRequest.cpp)
# SqlLib 依赖 LogLib
target_link_libraries(SqlLib PUBLIC LogLib)

target_link_libraries(HttpRequestLib PUBLIC LogLib BuflLib SqlLib)
# 添加 MySQL 的头文件和库路径
include_directories(/usr/local/mysql/include) # 替换为 MySQL 头文件路径
link_directories(/usr/local/mysql/lib)       # 替换为 MySQL 库文件路径

# 定义测试源文件列表
set(TEST_SOURCES
        gunit_test.cpp
        block_queue_test.cpp
        log_test.cpp
        conn_pool_test.cpp
        buffer_test.cpp
        http_request_test.cpp
)

# 循环为每个测试文件创建独立的目标和测试
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 提取源文件名（不带扩展名），作为测试名称
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    # 创建可执行文件，命名为测试名
    add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE})

    # 链接 Google Test、pthread、LogLib、SqlLib 和 MySQL 客户端库
    target_link_libraries(${TEST_NAME}
            PUBLIC
            GTest::gtest
            GTest::gtest_main
            pthread
            LogLib
            SqlLib
            BuflLib
            HttpRequestLib
            mysqlclient # 链接 MySQL 客户端库
    )

    # 添加该测试到 CTest 测试框架
    add_test(NAME ${TEST_NAME}
            COMMAND ${TEST_NAME})
endforeach()
