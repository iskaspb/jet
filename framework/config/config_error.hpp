//
//  config_error.hpp
//
//  Created by Alexey Tkachenko on 1/9/13.
//  This code belongs to public domain. You can do with it whatever you want without any guarantee.
//

#ifndef JET_CONFIG_CONFIG_ERROR_HEADER_GUARD
#define JET_CONFIG_CONFIG_ERROR_HEADER_GUARD

#include <exception>
#include <string>

namespace jet
{

class config_error: public std::exception
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
