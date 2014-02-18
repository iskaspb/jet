// jet.utils library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "stacktrace.hpp"
#include "demangle.hpp"
#ifndef _WIN32
#include <boost/algorithm/string.hpp>
#include <execinfo.h>
#include <cxxabi.h>
#include <stdlib.h>
#endif /*_WIN32*/

namespace jet
{

std::vector<std::string> stacktrace(const size_t start_from_frame, const size_t depth)
{
    std::vector<std::string> result{};
#ifndef _WIN32
    void** frames = static_cast<void**>(::alloca(sizeof(void*) * depth));
    
    const size_t actual_depth = ::backtrace(frames, static_cast<int>(depth));
    char** symbols = ::backtrace_symbols(frames, static_cast<int>(actual_depth));
    
    if(symbols)
    {
        for(size_t index = 0; actual_depth != index; ++index)
        {
#ifdef __clang__
            std::vector<std::string> parts;
            boost::split(parts, symbols[index], boost::is_any_of(" "), boost::token_compress_on);
            if(parts.size() > 3)
            {
                result.push_back(demangle(parts[3]));
                continue;
            }
#endif /*__clang__*/
            result.push_back(symbols[index]);
        }
        
        ::free(symbols);
    }
#endif /*_WIN32*/
    return result;
}

}//namespace jet
