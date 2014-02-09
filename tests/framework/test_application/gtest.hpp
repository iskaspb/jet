//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef JET_APPLICATION_GTEST_HEADER_GUARD
#define JET_APPLICATION_GTEST_HEADER_GUARD

#include <gtest/gtest.h>
#include <string>

#define EXPECT_EXCEPTION(EXPRESSION, EXCEPTION, MESSAGE) \
{ \
    const std::string message(MESSAGE); \
    try \
    { \
        do { EXPRESSION; } while(0); \
        FAIL() << "Negative test: exception " #EXCEPTION " is expected"; \
    } \
    catch(const EXCEPTION& ex) \
    { \
        const std::string exceptionMessage(std::string(ex.what()).substr(0, message.size())); \
        EXPECT_EQ(message, exceptionMessage); \
    } \
}

#endif /*JET_CONFIG_GTEST_HEADER_GUARD*/
