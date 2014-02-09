// jet.config library
//
//  Copyright Alexey Tkachneko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_CONFIG_CONFIG_ERROR_HEADER_GUARD
#define JET_CONFIG_CONFIG_ERROR_HEADER_GUARD

#include <exception>
#include <string>

namespace jet
{

class config_error: virtual public std::exception
{
public:
    explicit config_error(const std::string& msg): msg_(msg)
    {
        msg_.c_str();
    }
    ~config_error() throw() {}
    const char* what() const throw() { return msg_.c_str(); }
private:
    std::string msg_;
};

}//namespace jet

#endif //JET_CONFIG_CONFIG_ERROR_HEADER_GUARD
