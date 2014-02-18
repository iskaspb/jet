// jet.utils library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "demangle.hpp"
#include <cstdlib>
#ifndef _WIN32
#include <cxxabi.h>
#endif /*_WIN32*/

namespace jet
{

std::string demangle(const std::string& name)
{
#ifdef _WIN32
    return name;
#else /*_WIN32*/
    int status = 0;
    char* const res = abi::__cxa_demangle(name.c_str(), 0, 0, &status);
    if(!res)
        return name;
    try
    {
        const std::string demangled{res};
        ::free(res);
        return demangled;
    }
    catch (...)
    {
        ::free(res);
        return name;
    }
#endif /*_WIN32*/
}

}//namespace jet
