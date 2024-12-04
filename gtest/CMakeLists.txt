

set(TEST_PROJECT_NAME TinyWebSever_test)
set(PROJECT_NAME Tiny_WebSever)

project(${TEST_PROJECT_NAME})
#将gtest添加到项目中
# 关闭 gmock 功能
set(BUILD_GMOCK OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/googletest-1.15.2 googletest)

add_executable(${TEST_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/gunit_test.cpp)
target_include_directories(${TEST_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${TEST_PROJECT_NAME} PUBLIC  GTest::gtest GTest::gtest_main)

add_test(NAME ${TEST_PROJECT_NAME}
        COMMAND ${TEST_PROJECT_NAME})