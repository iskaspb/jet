// jet.application library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_APPLICATION_EXCEPTION_HEADER_GUARD
#define JET_APPLICATION_EXCEPTION_HEADER_GUARD

#include <exception>
#include <typeindex>
#include <string>
#include <iosfwd>
#include <vector>
#include <tuple>

namespace jet
{
//TODO: optional stacktrace

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
        (void)message_.c_str();
    }
    //...
    void set_location(const exception::location& location) { location_ = location; }
    
    void set_message(const std::string& message) { (message_ = message).c_str(); }

    const char* what() const noexcept override { return message_.c_str(); }
    std::string diagnostics() const;
    std::vector<std::tuple<std::type_index, location, std::string>> detailed_diagnostics() const;
private:
    void diagnostics(std::ostream& os) const;
    static std::exception_ptr get_chained_exception();
    std::string message_;
    exception::location location_;
    std::exception_ptr nested_{std::current_exception()};
};

std::ostream& operator<<(std::ostream& os, const exception& ex);
std::ostream& operator<<(std::ostream& os, const exception::location& location);
}//namespace jet

#endif /*JET_APPLICATION_EXCEPTION_HEADER_GUARD*/
