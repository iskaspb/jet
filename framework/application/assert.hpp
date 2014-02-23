//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef JET_APPLICATION_ASSERT_HEADER_GUARD
#define JET_APPLICATION_ASSERT_HEADER_GUARD

#include "throw.hpp"

namespace jet { struct assert_error: virtual exception {}; }

//TODO: test it
#define JET_ASSERT(CONDITION)                                  \
    for(jet::error_stream jet_assert_strm;!(CONDITION);        \
        jet_assert_strm.empty()?                               \
            jet::throw_exception<jet::assert_error>(           \
                JET_ERROR_LOCATION,                            \
                "Assertion failed (" #CONDITION ")"):          \
            jet::throw_exception<jet::assert_error>(           \
                JET_ERROR_LOCATION,                            \
                "Assertion failed (" #CONDITION "), message: " + jet_assert_strm.str())) \
        jet_assert_strm

#endif /*JET_APPLICATION_ASSERT_HEADER_GUARD*/
