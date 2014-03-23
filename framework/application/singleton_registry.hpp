// jet.application library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef application_singleton_registry_hpp
#define application_singleton_registry_hpp


#include "impl/singularity.hpp"
#include <boost/noncopyable.hpp>
#include <boost/application/context.hpp>
#include <functional>
#include <typeindex>
#include <map>

namespace jet
{

class singleton_registry: boost::noncopyable
{
    struct factory
    {
        virtual ~factory() {}
        virtual void create(const boost::application::context& app_context) = 0;
        virtual void destroy() = 0;
    };
    template<typename T> struct factory_impl: factory
    {
        void create(const boost::application::context& app_context) override
        {
            singularity<T>::create_global();
        }
        void destroy() override
        {
            singularity<T>::destroy();
        }
    };
public:
    explicit singleton_registry(const boost::application::context& app_context = {});
    ~singleton_registry();
    template<typename T>
    static void register_singleton()
    {
        std::type_index type_index{typeid(T)};
        if(registry_.count(type_index))
            return;
        registry_[type_index] = std::unique_ptr<factory>{new factory_impl<T>{}};
    }
private:
    static std::map<std::type_index, std::unique_ptr<factory>> registry_;
    static bool initialized_;
};

}//namespace jet

#endif /*application_singleton_registry_hpp*/
