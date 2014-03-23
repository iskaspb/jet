// jet.application library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "singleton_registry.hpp"

namespace jet
{
std::map<std::type_index, std::unique_ptr<singleton_registry::factory>> singleton_registry::registry_;
bool singleton_registry::initialized_{false};

singleton_registry::singleton_registry(const boost::application::context& app_context)
{
    if(initialized_)
        JET_THROW_EX(singleton_error) << "Double initalization of singleton_registry";
    auto iter = registry_.begin();
    try
    {
        for(const auto end = registry_.end(); end != iter; ++iter)
            iter->second->create(app_context);
        initialized_ = true;
    }
    catch(...)
    {
        const auto end = iter;
        for(iter = registry_.begin(); end != iter; ++iter)
            iter->second->destroy();
        throw;
    }
}

singleton_registry::~singleton_registry()



{
    initialized_ = false;
    for(auto& item : registry_)
        try { item.second->destroy(); } catch(...) {}
}

}//namespace jet