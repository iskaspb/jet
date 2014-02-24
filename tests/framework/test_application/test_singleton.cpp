//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>
#include "application/impl/singularity.hpp"
#include "utils/demangle.hpp"
#include <iostream>

using std::cout;
using std::endl;

class a_class
{
public:
    explicit a_class(int val): value_(val) {}
    int value() const { return value_; }
private:
    int value_;
};

using a_singularity = jet::singularity<a_class>;

TEST(singularity, simple)
{
    EXPECT_EQ(
        "jet::singularity<a_class, jet::single_threaded>",
        jet::demangle(typeid(a_singularity).name()));
    a_singularity::create_global(15);
    EXPECT_EQ(15, a_singularity::get_global().value());
    a_singularity::destroy();
}
