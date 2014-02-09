// jet.application library
//
//  Copyright Alexey Tkachneko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "exception.hpp"

namespace jet
{

std::ostream& operator<<(std::ostream& os, const exception& ex)
{
    ex.diagnostics(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const exception::location& location)
{
    if(!location.is_valid())
        return os << "[unknown location]";
    return os << "[function: " << location.function() << " at " << location.file() << ':' << location.line() << ']';
}

void exception::diagnostics(std::ostream& os) const
{
    os << "exception(" << typeid(*this).name() << ')';
    char const * const msg = what();
    if(msg && msg[0])
        os << ": " << msg;
    if(location_.is_valid())
        os << " raised from " << location_;
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
            os << "exception(" << typeid(ex).name() << "): " << ex.what() << '\n';
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

}//namespace jet
