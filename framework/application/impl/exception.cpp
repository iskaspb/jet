// jet.application library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "exception.hpp"
#include "utils/demangle.hpp"
#include <sstream>

namespace jet
{

const char* exception::location::remove_dir_from_path(const char* file)
{//...trim long path
    if(!file)
        return file;
    constexpr char separator =
#ifdef _WIN32
    '\\'
#else /*_WIN32*/
    '/'
#endif /*_WIN32*/
    ;
    const char* res = file;
    while(const char ch = *file++)
        if(separator == ch)
            res = file;
    return res;
}

std::ostream& operator<<(std::ostream& os, const exception& ex)
{
    ex.diagnostics(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const exception::location& location)
{
    if(!location.is_valid())
        return os << "[unknown location]";
    return os << '[' << location.function() << " @ " << location.file() << ':' << location.line() << ']';
}

void exception::diagnostics(std::ostream& os) const
{
    os << demangle(typeid(*this).name());
    if(location_.is_valid())
        os << location_;
    {
        os << ": ";
        char const * const msg = what();
        if(msg && msg[0])
            os << msg;
        else
            os << "no message";
    }
    os << '\n';
    if (nested_ != std::exception_ptr())
    {
        try
        {
            std::rethrow_exception(nested_);
        }
        catch(const jet::exception& ex)
        {
            os << ex;
        }
        catch(const std::exception& ex)
        {
            os << demangle(typeid(ex).name()) << ": ";
            char const * const msg = ex.what();
            if(msg && msg[0])
                os << msg;
            else
                os << "no message";
            os << '\n';
        }
        catch(...)
        {
            os << "unknown exception\n";
        }
    }
}

std::string exception::diagnostics() const
{
    std::ostringstream strm;
    diagnostics(strm);
    return strm.str();
}

std::vector<exception::details> exception::detailed_diagnostics() const
{
    std::vector<details> chained_details;
    populate_details(chained_details);
    return chained_details;
}

void exception::populate_details(std::vector<details>& chained_details) const
{
    chained_details.push_back(
        details{
            typeid(*this),
            location_,
            what()? std::string{what()}: std::string{}});
    if (nested_ != std::exception_ptr())
    {
        try
        {
            std::rethrow_exception(nested_);
        }
        catch(const jet::exception& ex)
        {
            ex.populate_details(chained_details);
        }
        catch(const std::exception& ex)
        {
            chained_details.push_back(
                details{
                    std::type_index(typeid(ex)),
                    location{},
                    ex.what()? std::string{ex.what()}: std::string{}});
        }
        catch(...)
        {
            chained_details.push_back(
                details{
                    typeid(unknown_exception),
                    location{},
                    std::string{}});
        }
    }
}

}//namespace jet
