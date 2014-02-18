//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "utils/stacktrace.hpp"
#include <gtest/gtest.h>
#include <iostream>
using std::cout;
using std::endl;

TEST(stacktrace, simple)
{
    for(const auto& frame : jet::stacktrace())
    {
        cout << frame << endl;
    }
    EXPECT_EQ(true, true);
}
