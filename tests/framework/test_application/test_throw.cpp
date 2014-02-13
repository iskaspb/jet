//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "application/throw.hpp"
#include "gtest.hpp"
#include <stdexcept>
#include <iostream>
using std::cout;
using std::endl;

void throw_helper(const std::string& msg)
{//...I use throw_helper thrughout this test suite to avoid rewriting test cases every time I change rearrange the code
    JET_THROW() << msg;
}

template<typename ex_type>
void throw_helper_ex(const std::string& msg)
{
    JET_THROW_EX(ex_type) << msg;
}

TEST(exception, naked_throw)
{
    try
    {
        throw jet::exception{};
    }
    catch(const jet::exception& ex)
    {
        EXPECT_EQ(std::string{}, ex.what());
        EXPECT_EQ("jet::exception: no message\n", ex.diagnostics());
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
        EXPECT_EQ(std::string{"error message"}, ex.what());
        EXPECT_EQ("jet::exception: error message\n", ex.diagnostics());
    }
}

TEST(exception, throw_with_complex_message)
{
    try
    {
        JET_THROW() << "ONE " << 1 << ", TWO " << 2;
    }
    catch(const std::exception& ex)
    {
        EXPECT_EQ(std::string{"ONE 1, TWO 2"}, ex.what());
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
        EXPECT_EQ(std::string{"error message"}, ex.what());
        EXPECT_EQ(
            "jet::exception[void throw_helper(const std::string &) @ test_throw.cpp:15]: error message\n",
            ex.diagnostics());
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
R"(jet::exception[void throw_helper(const std::string &) @ test_throw.cpp:15]: error message
std::runtime_error: initial exception
)";
        EXPECT_EQ(expected, ex.diagnostics());
        const std::vector<jet::exception::details> expected_diagnostics
        {
            {
                typeid(jet::exception),
                jet::exception::location{"test_throw.cpp", 15, "void throw_helper(const std::string &)"},
                "error message"
            },
            {
                typeid(std::runtime_error),
                jet::exception::location{},
                "initial exception"
            }
        };
        const auto diagnostics = ex.detailed_diagnostics();
        ASSERT_EQ(expected_diagnostics.size(), diagnostics.size());
        EXPECT_TRUE(std::equal(diagnostics.begin(), diagnostics.end(), expected_diagnostics.begin()));
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
R"(jet::exception[void throw_helper(const std::string &) @ test_throw.cpp:15]: error message
jet::exception[void throw_helper(const std::string &) @ test_throw.cpp:15]: initial exception
)";
        EXPECT_EQ(expected, ex.diagnostics());
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
R"(jet::exception[void throw_helper(const std::string &) @ test_throw.cpp:15]: error message
my_error[void throw_helper_ex(const std::string &) [ex_type = my_error] @ test_throw.cpp:21]: initial exception
)";
        EXPECT_EQ(expected, ex.diagnostics());
        const std::vector<jet::exception::details> expected_diagnostics
        {
            {
                typeid(jet::exception),
                jet::exception::location{"test_throw.cpp", 15, "void throw_helper(const std::string &)"},
                "error message"
            },
            {
                typeid(my_error),
                jet::exception::location
                {
                    "test_throw.cpp",
                    21,
                    "void throw_helper_ex(const std::string &) [ex_type = my_error]"
                },
                "initial exception"
            }
        };
        const auto diagnostics = ex.detailed_diagnostics();
        ASSERT_EQ(expected_diagnostics.size(), diagnostics.size());
        EXPECT_TRUE(std::equal(diagnostics.begin(), diagnostics.end(), expected_diagnostics.begin()));
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
R"(jet::exception[void throw_helper(const std::string &) @ test_throw.cpp:15]: error message
unknown exception
)";
        EXPECT_EQ(expected, ex.diagnostics());
        const std::vector<jet::exception::details> expected_diagnostics
        {
            {
                typeid(jet::exception),
                jet::exception::location{"test_throw.cpp", 15, "void throw_helper(const std::string &)"},
                "error message"
            },
            {
                typeid(jet::unknown_exception),
                jet::exception::location{},
                std::string()
            }
        };
        const auto diagnostics = ex.detailed_diagnostics();
        ASSERT_EQ(expected_diagnostics.size(), diagnostics.size());
        EXPECT_TRUE(std::equal(diagnostics.begin(), diagnostics.end(), expected_diagnostics.begin()));
    }
}
