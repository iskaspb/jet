//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef JET_CONFIG_GTEST_HEADER_GUARD
#define JET_CONFIG_GTEST_HEADER_GUARD

#include "utils/exception.hpp"
#include <gtest/gtest.h>
#include <boost/optional.hpp>
#include <vector>
#include <string>

#define DUMP_ERROR(EXPRESSION)                           \
    try { EXPRESSION; }                                  \
    catch(const jet::exception& ex) { std::cout << ex; } \
    catch(const std::exception& ex) { std::cout << ex.what() << std::endl; }

namespace jet
{
namespace test
{

template<typename derived_matcher>
struct message_chain
{
    derived_matcher& operator()(const std::string& msg = std::string{})
    {
        messages_.push_back(msg);
        return *static_cast<derived_matcher*>(this);
    }
protected:
    std::vector<std::string> messages_;
};

struct equal: message_chain<equal>
{
    explicit equal(const std::string& msg = std::string{})
    {
        (*this)(msg);
    }
    boost::optional<std::string> find_error(const std::exception& ex) const
    {
        if(messages_.size() != 1)
        {
            std::ostringstream strm;
            strm << "Expected error:\n";
            for(const auto& msg: messages_)
                strm << "  " << msg << '\n';
            strm << "\nHas more messages than single actual error:\n" << ex.what();
            return strm.str();
        }
        const std::string& expected_msg{messages_[0]};
        const std::string error_msg{ex.what()};
        if(expected_msg == error_msg)
            return boost::none;
        std::ostringstream strm;
        strm<< "Error message: [" << error_msg
            << "]\n is not equal to expected message: [" << expected_msg << "]\n";
        return strm.str();
    }
    boost::optional<std::string> find_error(const exception& ex) const
    {
        std::vector<::jet::exception::details> errors{ ex.detailed_diagnostics() };
        if(messages_.size() != errors.size())
        {
            std::ostringstream strm;
            strm << "Expected error:\n";
            for(const auto& msg: messages_)
                strm << "  " << msg << '\n';
            strm << "\nHas different number of messages than actual error:\n";
            for(const auto& details: errors)
                strm << "  " << std::get<2>(details) << '\n';
            return strm.str();
        }
        for(size_t index = 0; index != messages_.size(); ++index)
        {
            const std::string& expected_msg{messages_[index]};
            const std::string& error_msg{std::get<2>(errors[index])};
            if(expected_msg == error_msg)
                continue;
            std::ostringstream strm;
            strm<< "Error message: [" << error_msg
                << "]\n is not equal to expected message: [" << expected_msg << "]\n";
            return strm.str();
        }
        return boost::none;
    }
};
struct start_with: message_chain<start_with>
{
    explicit start_with(const std::string& msg = std::string{})
    {
        (*this)(msg);
    }
    boost::optional<std::string> find_error(const std::exception& ex) const
    {
        if(messages_.size() != 1)
        {
            std::ostringstream strm;
            strm << "Expected error:\n";
            for(const auto& msg: messages_)
                strm << "  " << msg << '\n';
            strm << "\nHas more messages than single actual error:\n" << ex.what();
            return strm.str();
        }
        const std::string& expected_msg{messages_[0]};
        const std::string error_msg{std::string{ex.what()}.substr(0, expected_msg.size())};
        if(expected_msg == error_msg)
            return boost::none;
        std::ostringstream strm;
        strm<< "Error message: [" << ex.what()
            << "]\n is not equal to expected message: [" << expected_msg << "]\n";
        return strm.str();
    }
    boost::optional<std::string> find_error(const ::jet::exception& ex) const
    {
        std::vector<::jet::exception::details> errors{ ex.detailed_diagnostics() };
        if(messages_.size() > errors.size())
        {
            std::ostringstream strm;
            strm << "Expected error:\n";
            for(const auto& msg: messages_)
                strm << "  " << msg << '\n';
            strm << "\nHas more messages than actual error:\n";
            for(const auto& details: errors)
                strm << "  " << std::get<2>(details) << '\n';
            return strm.str();
        }
        for(size_t index = 0; index != messages_.size(); ++index)
        {
            const std::string& expected_msg{messages_[index]};
            const std::string error_msg{std::get<2>(errors[index]).substr(0, expected_msg.size())};
            if(expected_msg == error_msg)
                continue;
            std::ostringstream strm;
            strm<< "Error message: [" << std::get<2>(errors[index])
                << "]\n doesn't match to expected message: [" << expected_msg << "]\n";
            return strm.str();
        }
        return boost::none;
    }
};

}//namespace test
}//namespace jet

#define EXPECT_ERROR_EX(EXPRESSION, EXCEPTION, MATCHER) \
do \
{ \
    try \
    { \
        do { EXPRESSION; } while(0); \
        FAIL() << "Negative test: exception " #EXCEPTION " is expected"; \
    } \
    catch(const EXCEPTION& ex) \
    { \
        boost::optional<std::string> error{MATCHER.find_error(ex)}; \
        if(error) \
            FAIL() << *error; \
    } \
} \
while(0)

#define EXPECT_JET_ERROR(EXPRESSION, MATCHER) \
    EXPECT_ERROR_EX(EXPRESSION, ::jet::exception, MATCHER)

#define EXPECT_STD_ERROR(EXPRESSION, MATCHER) \
    EXPECT_ERROR_EX(EXPRESSION, ::std::exception, MATCHER)

#endif /*JET_CONFIG_GTEST_HEADER_GUARD*/
