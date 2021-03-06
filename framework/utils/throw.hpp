//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef JET_APPLICATION_THROW_HEADER_GUARD
#define JET_APPLICATION_THROW_HEADER_GUARD

#include "exception.hpp"
#include <boost/current_function.hpp>
#include <boost/noncopyable.hpp>
#include <sstream>

namespace jet
{
struct error_stream: boost::noncopyable
{
    template<typename T>
    std::ostream& operator<<(T&& arg)
    {
        return strm() << std::forward<T>(arg);
    }
    std::string str() { return strm().str(); }
    bool empty() const { return !holder_.valid(); }
private:
    std::ostringstream& strm() { return holder_; }
    struct holder
    {
        holder(): ptr_{} {}
        ~holder()
        {
            if(valid())
                ptr_->~basic_ostringstream();
        }
        operator std::ostringstream&()
        {
            if(!valid())
                ptr_ = new (memory_) std::ostringstream{};
            return *ptr_;
        }
        bool valid() const { return ptr_; }
    private:
        alignas(std::ostringstream) char memory_[sizeof(std::ostringstream)];
        std::ostringstream* ptr_;
    } holder_;
};

template<typename exception_type>
inline void throw_exception [[noreturn]] (
    const exception::location& location, const std::string& message)
{
    exception_type ex{};
    ex.set_message(message);
    ex.set_location(location);
    throw ex;
}
template<typename exception_type>
inline void throw_exception [[noreturn]] (
    const exception::location& location)
{
    exception_type ex{};
    ex.set_location(location);
    throw ex;
}

}//namespace jet

#define JET_ERROR_LOCATION                                              \
        ::jet::exception::location{                                     \
            __FILE__,                                                   \
            __LINE__,                                                   \
            BOOST_CURRENT_FUNCTION}

#define JET_THROW_EX_WITH_LOCATION(EXCEPTION, LOCATION)                 \
    for(jet::error_stream jet_throw_ex_with_location_strm;;             \
        jet_throw_ex_with_location_strm.empty()?                        \
            jet::throw_exception<EXCEPTION>(LOCATION):                  \
            jet::throw_exception<EXCEPTION>(                            \
                LOCATION, jet_throw_ex_with_location_strm.str()))       \
        jet_throw_ex_with_location_strm

#define JET_THROW_EX(EXCEPTION)                                         \
    JET_THROW_EX_WITH_LOCATION(                                         \
        EXCEPTION,                                                      \
        JET_ERROR_LOCATION)

#define JET_THROW()                                                     \
    JET_THROW_EX(::jet::exception)

#endif /*JET_APPLICATION_THROW_HEADER_GUARD*/
