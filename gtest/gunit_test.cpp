//
// Created by Admin o

#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace std;

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