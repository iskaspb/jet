//  Copyright Alexey Tkachneko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "application/exception.hpp"
#include "gtest.hpp"
#include <iostream>

using std::cout;
using std::endl;

TEST(exception, naked_throw)
{
    try
    {
        throw jet::exception{};
    }
    catch(const jet::exception& ex)
    {
        EXPECT_EQ(ex.what(), std::string{});
        EXPECT_EQ(ex.diagnostics(), "exception(jet::exception)\n");
    }
}

TEST(exception, throw_with_message)
{
    try
    {
        throw jet::exception{"error message"};
    }
    catch(const jet::exception& ex)
    {
        EXPECT_EQ(ex.what(), std::string{"error message"});
    }
}
