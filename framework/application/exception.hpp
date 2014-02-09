// jet.application library
//
//  Copyright Alexey Tkachneko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_APPLICATION_EXCEPTION_HEADER_GUARD
#define JET_APPLICATION_EXCEPTION_HEADER_GUARD

#include <sstream>
#include <exception>
#include <string>
#include <boost/current_function.hpp>
#include <boost/foreach.hpp>

namespace jet
{

class exception: virtual public std::exception
{
    friend std::ostream& operator<<(std::ostream& os, const exception& ex);
public:
    class location
    {
    public:
        location() {}
        location(const char* file, int line, const char* function):
            file_{remove_dir_from_path(file)},
            line_{line},
            function_{function}
        {}
        const char* file() const { return file_; }
        int line() const { return line_; }
        const char* function() const { return function_; }
        bool is_valid() const { return file_ && line_ && function_; }
    private:
        const char* remove_dir_from_path(const char* file);
        //...
        const char* file_{};
        int line_{};
        const char* function_{};
    };
    //...
    exception() {}
    explicit exception(
        const std::string& message,
        const exception::location& location = exception::location{}):
        message_{message},
        location_{location}
    {
        (void)message.c_str();
    }
    //...
    const std::exception_ptr& get_nested() const { return nested_; }
    
    const exception::location& get_location() const { return location_; }
    void set_location(const exception::location& location) { location_ = location; }
    
    const std::string& get_message() const { return message_; }
    void set_message(const std::string& message) { (message_ = message).c_str(); }

    const char* what() const noexcept override { return message_.c_str(); }
    std::string diagnostics() const;
private:
    void diagnostics(std::ostream& os) const;
    std::string message_;
    exception::location location_;
    std::exception_ptr nested_{std::current_exception()};
};

std::ostream& operator<<(std::ostream& os, const exception& ex);
std::ostream& operator<<(std::ostream& os, const exception::location& location);
}//namespace jet

#ifdef JET_THROW_EX_WITH_LOCATION
#error "JET_THROW_EX_WITH_LOCATION is already defined. Consider changing its name"
#endif /*JET_THROW_EX_WITH_LOCATION*/

#define JET_THROW_EX_WITH_LOCATION(                                     \
    FILE, LINE, FUNCTION, EXCEPTION, MESSAGE)                           \
    do                                                                  \
    {                                                                   \
        EXCEPTION new_ex;                                               \
        {                                                               \
            std::ostringstream strm;                                    \
            strm << MESSAGE;                                            \
            new_ex.set_message(strm.str());                             \
        }                                                               \
        new_ex.set_location(                                            \
            jet::exception::location(                                   \
                FILE, LINE, FUNCTION));                                 \
        throw new_ex;                                                   \
    }                                                                   \
    while(0)

#ifdef JET_THROW_EX
#error "JET_THROW_EX is already defined. Consider changing its name"
#endif /*JET_THROW_EX*/

#define JET_THROW_EX(EXCEPTION, MESSAGE)                                \
    JET_THROW_EX_WITH_LOCATION(                                         \
        __FILE__, __LINE__, BOOST_CURRENT_FUNCTION, EXCEPTION, MESSAGE)

#ifdef JET_THROW
#error "JET_THROW is already defined. Consider changing its name"
#endif /*JET_THROW*/

#define JET_THROW(MESSAGE)                                              \
    JET_THROW_EX(jet::exception, MESSAGE)

#endif /*JET_APPLICATION_EXCEPTION_HEADER_GUARD*/
