//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef JET_APPLICATION_SINGLETON_HEADER_GUARD
#define JET_APPLICATION_SINGLETON_HEADER_GUARD

#include "singleton_registry.hpp"
#include <boost/noncopyable.hpp>

namespace jet
{

template<typename T>
class singleton: boost::noncopyable
{
    singleton()
    {
        singleton_registry::register_singleton<T>();
    }
public:
    static T& instance()
    {
        (void)instance_;//...this is to explain compiler that instance_ has to be created
        return jet::singularity<T>::get_global();
    }
private:
    static singleton instance_;
};

template<typename T>
singleton<T> singleton<T>::instance_{};

} //namespace jet

#endif /*JET_APPLICATION_SINGLETON_HEADER_GUARD*/
