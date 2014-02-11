//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef JET_APPLICATION_THROW_HEADER_GUARD
#define JET_APPLICATION_THROW_HEADER_GUARD

#include "exception.hpp"
#include <boost/current_function.hpp>
#include <sstream>

namespace jet
{
template<typename exception_type>
inline void throw_exception [[noreturn]] (
    const std::string& message, const exception::location& location)
{
    exception_type ex;
    ex.set_message(message);
    ex.set_location(location);
    throw ex;
}
}//namespace jet

//TODO: make unique 'strm' object name
#define JET_THROW_EX_WITH_LOCATION(EXCEPTION, LOCATION)                 \
    for(std::ostringstream strm;;                                       \
        ::jet::throw_exception<EXCEPTION>(strm.str(), LOCATION))        \
        strm

#define JET_THROW_EX(EXCEPTION)                                         \
    JET_THROW_EX_WITH_LOCATION(                                         \
        EXCEPTION, \
        ::jet::exception::location( \
            __FILE__, \
            __LINE__, \
            BOOST_CURRENT_FUNCTION))

#define JET_THROW()                                                     \
    JET_THROW_EX(::jet::exception)

#endif /*JET_APPLICATION_THROW_HEADER_GUARD*/
