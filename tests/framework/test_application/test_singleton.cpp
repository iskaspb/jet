//
//  test_config.cpp
//
//  Created by Alexey Tkachenko on 26/8/13.
//  This code belongs to public domain. You can do with it whatever you want without any guarantee.
//
#include "gtest.hpp"
#include <iostream>

using std::cout;
using std::endl;

TEST(singleton, test)
{
    EXPECT_EQ(1, 1);
}
