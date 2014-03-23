//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>
#include "application/impl/singularity.hpp"
#include "application/singleton.hpp"
#include "utils/demangle.hpp"
#include <iostream>

using std::cout;
using std::endl;

template<unsigned N>
class a_class
{
public:
    explicit a_class(int val = 10): value_(val)
    {
        ++number_of_instances_;
    }
    ~a_class()
    {
        --number_of_instances_;
    }
    int value() const { return value_; }
    static int number_of_instances() { return number_of_instances_; }
private:
    int value_;
    static int number_of_instances_;
};

template<unsigned N>
int a_class<N>::number_of_instances_{};


TEST(singularity, simple_use_case)
{
    using a_class = a_class<__LINE__>;
    using a_singularity = jet::singularity<a_class>;
    EXPECT_EQ(
        "jet::singularity<" + jet::demangle(typeid(a_class).name()) + ", jet::single_threaded>",
        jet::demangle(typeid(a_singularity).name()));
    EXPECT_EQ(0, a_class::number_of_instances());
    a_singularity::create_global(15);
    EXPECT_EQ(1, a_class::number_of_instances());
    EXPECT_EQ(15, a_singularity::get_global().value());
    a_singularity::destroy();
    EXPECT_EQ(0, a_class::number_of_instances());
}

TEST(singleton, simple_use_case)
{
    using a_class = a_class<__LINE__>;
    using a_singleton = jet::singleton<a_class>;
    EXPECT_EQ(
        "jet::singleton<" + jet::demangle(typeid(a_class).name()) + " >",
        jet::demangle(typeid(a_singleton).name()));
    
    EXPECT_EQ(0, a_class::number_of_instances());
    {
        jet::singleton_registry singleton_registry;
        EXPECT_EQ(1, a_class::number_of_instances());
        EXPECT_EQ(10, jet::singleton<a_class>::instance().value());
    }
    EXPECT_EQ(0, a_class::number_of_instances());
}

TEST(singleton, double_registration)
{
    using a_class = a_class<__LINE__>;
    using a_singleton = jet::singleton<a_class>;
    
    EXPECT_EQ(0, a_class::number_of_instances());
    {
        jet::singleton_registry singleton_registry;
        jet::singleton_registry::register_singleton<a_class>();
        EXPECT_EQ(1, a_class::number_of_instances());
        EXPECT_EQ(10, jet::singleton<a_class>::instance().value());
    }
    EXPECT_EQ(0, a_class::number_of_instances());
}

void foo(int i, std::string s)
{
    cout << i << ", " << s << endl;
}

template <class ...Args>
void wrapper_foo(Args&& ...args)
{
    foo(std::forward<Args>(args)...);
}

template<std::size_t ...>
struct indices { };

template<std::size_t N, std::size_t ...Indices>
struct build_indices : build_indices<N-1, N-1, Indices...> { };

template<std::size_t ...Indices>
struct build_indices<0, Indices...> {
  typedef indices<Indices...> type;
};

template<typename ...Args, std::size_t... Indices>
void apply_helper(indices<Indices...>, std::tuple<Args...>&& params)
{
    return wrapper_foo( std::forward<Args>( std::get<Indices>(params))... );
} 

template<typename ...Args>
void apply(const std::tuple<Args...>& params)
{
    return apply_helper(typename build_indices<sizeof...(Args)>::type{}, std::tuple<Args...>{params});
}

template<typename ...Args>
void apply(std::tuple<Args...>&& params)
{
    return apply_helper(typename build_indices<sizeof...(Args)>::type{}, std::forward<std::tuple<Args...>>(params));
}

TEST(singleton, tuple_unpack)
{
    const auto params = std::make_tuple(10, std::string{"hello"});
    cout << jet::demangle(typeid(params).name()) << endl;
//    call_func<1, 2>(typename build_indices<0, 1, 2>::type());
    apply(params);
    
}
