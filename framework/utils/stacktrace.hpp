// jet.utils library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_UTILS_STACKTRACE_HEADER_GUARD
#define JET_UTILS_STACKTRACE_HEADER_GUARD

#include <string>
#include <vector>

namespace jet
{

enum { DEFAULT_STACK_DEPTH = 20 };

std::vector<std::string> stacktrace(size_t start_from_frame = 1, size_t depth = DEFAULT_STACK_DEPTH);

}//namespace jet

#endif /*JET_UTILS_STACKTRACE_HEADER_GUARD*/
