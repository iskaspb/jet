//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>
#include "application/impl/singularity.hpp"
#include "application/singleton.hpp"
#include "utils/demangle.hpp"
#include <iostream>

using std::cout;
using std::endl;

class a_class
{
public:
    explicit a_class(int val = 10): value_(val)
    {
        ++number_of_instances_;
    }
    ~a_class()
    {
        --number_of_instances_;
    }
    int value() const { return value_; }
    static int number_of_instances() { return number_of_instances_; }
private:
    int value_;
    static int number_of_instances_;
};

int a_class::number_of_instances_{};

using a_singularity = jet::singularity<a_class>;

TEST(singularity, simple_use_case)
{
    EXPECT_EQ(
        "jet::singularity<a_class, jet::single_threaded>",
        jet::demangle(typeid(a_singularity).name()));
    EXPECT_EQ(0, a_class::number_of_instances());
    a_singularity::create_global(15);
    EXPECT_EQ(1, a_class::number_of_instances());
    EXPECT_EQ(15, a_singularity::get_global().value());
    a_singularity::destroy();
    EXPECT_EQ(0, a_class::number_of_instances());
}

using a_singleton = jet::singleton<a_class>;

TEST(singleton, simple_use_case)
{
    EXPECT_EQ(
        "jet::singleton<a_class>",
        jet::demangle(typeid(a_singleton).name()));
    
    EXPECT_EQ(0, a_class::number_of_instances());
    {
        jet::singleton_registry singleton_registry;
        EXPECT_EQ(1, a_class::number_of_instances());
        EXPECT_EQ(10, jet::singleton<a_class>::instance().value());
    }
    EXPECT_EQ(0, a_class::number_of_instances());
}

//TODO: add integration with configuration and application