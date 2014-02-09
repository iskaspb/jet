//  Copyright Alexey Tkachneko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "application/exception.hpp"
#include "gtest.hpp"
#include <stdexcept>
#include <iostream>
using std::cout;
using std::endl;

void throw_helper(const std::string& msg)
{//...I use throw_helper thrughout this test suite to avoid rewriting test cases every time I change rearrange the code
    JET_THROW(msg);
}

template<typename ExType>
void throw_helper_ex(const std::string& msg)
{
    JET_THROW_EX(ExType, msg);
}

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
        EXPECT_EQ(ex.diagnostics(), "exception(jet::exception): error message\n");
    }
}

TEST(exception, throw_with_complex_message)
{
    try
    {
        JET_THROW("ONE " << 1 << ", TWO " << 2);
    }
    catch(const std::exception& ex)
    {
        EXPECT_EQ(ex.what(), std::string{"ONE 1, TWO 2"});
    }
}


TEST(exception, throw_with_location)
{
    try
    {
        throw_helper("error message");
    }
    catch(const jet::exception& ex)
    {
        EXPECT_EQ(ex.what(), std::string{"error message"});
        EXPECT_EQ(
            ex.diagnostics(),
            "exception(jet::exception): error message raised from [function: void throw_helper(const std::string &) at test_throw.cpp:15]\n");
    }
}

TEST(exception, chained_with_runtime_error)
{
    try
    {
        try
        {
            throw std::runtime_error("initial exception");
        }
        catch(...)
        {
            throw_helper("error message");
        }
    }
    catch(const jet::exception& ex)
    {
        const auto expected =
R"(exception(jet::exception): error message raised from [function: void throw_helper(const std::string &) at test_throw.cpp:15]
exception(std::runtime_error): error message
)";
        EXPECT_EQ(ex.diagnostics(), expected);
    }
}

TEST(exception, chained_with_jet_exception)
{
    try
    {
        try
        {
            throw_helper("initial exception");
        }
        catch(...)
        {
            throw_helper("error message");
        }
    }
    catch(const jet::exception& ex)
    {
        const auto expected =
R"(exception(jet::exception): error message raised from [function: void throw_helper(const std::string &) at test_throw.cpp:15]
exception(jet::exception): initial exception raised from [function: void throw_helper(const std::string &) at test_throw.cpp:15]
)";
        EXPECT_EQ(ex.diagnostics(), expected);
    }
}

struct my_error: virtual jet::exception {};

TEST(exception, chained_with_jet_exception_derived_exception)
{
    try
    {
        try
        {
            throw_helper_ex<my_error>("initial exception");
        }
        catch(...)
        {
            throw_helper("error message");
        }
    }
    catch(const jet::exception& ex)
    {
        const auto expected =
R"(exception(jet::exception): error message raised from [function: void throw_helper(const std::string &) at test_throw.cpp:15]
exception(my_error): initial exception raised from [function: void throw_helper_ex(const std::string &) [ExType = my_error] at test_throw.cpp:21]
)";
        EXPECT_EQ(ex.diagnostics(), expected);
    }
}

TEST(exception, chained_with_unknown_exception)
{
    try
    {
        try
        {
            throw 0;
        }
        catch(...)
        {
            throw_helper("error message");
        }
    }
    catch(const jet::exception& ex)
    {
        const auto expected =
R"(exception(jet::exception): error message raised from [function: void throw_helper(const std::string &) at test_throw.cpp:15]
unknown exception
)";
        EXPECT_EQ(ex.diagnostics(), expected);
    }
}
