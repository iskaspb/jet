// jet.config library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_CONFIG_CONFIG_ERROR_HEADER_GUARD
#define JET_CONFIG_CONFIG_ERROR_HEADER_GUARD

#include "utils/exception.hpp"

namespace jet
{

struct config_error: virtual jet::exception {};

}//namespace jet

#endif //JET_CONFIG_CONFIG_ERROR_HEADER_GUARD
