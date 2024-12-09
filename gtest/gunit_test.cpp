//
// Created by Admin o

#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace std;


class MyTestFixture : public ::testing::Test {
protected:
    void SetUp() override { /* 初始化操作 */ }
    void TearDown() override { /* 清理操作 */ }

    int shared_data;
};

TEST_F(MyTestFixture, Test1) {
    shared_data = 10;
    EXPECT_EQ(shared_data, 10);
}

TEST_F(MyTestFixture, Test2) {
    shared_data = 20;
    EXPECT_EQ(shared_data, 20);
}

void test()
{
    std::cout << "hello gtest" << std::endl;
}

TEST(TestCase, test1) {

    test();
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}