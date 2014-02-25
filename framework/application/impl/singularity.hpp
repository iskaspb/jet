
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//----------------------------------------------------------------------------
//! \file
//! \brief Singularity design pattern enforces a single instance of a class.
//!
//! Unlike Singleton, singularity does not force global access, nor does it
//! require that the class have a default constructor.  The lifetime of the
//! object is simply defined between ::create() and ::destroy().
//! An object created with singularity must be passed into objects which depend
//! on them, just like any other object.  Unless created with
//! ::create_global(), in which case the object is accessible with ::get_global().
//----------------------------------------------------------------------------
//  Event event;
//
//  Usage as a Factory:
//
//  Horizon & horizonF = singularity<Horizon, single_threaded>::create(1, &event, event);
//                       singularity<Horizon, single_threaded>::destroy();
//
//  Usage as a Base Class:
//
//  class Horizon : public singularity<Horizon, multi_threaded>
//  Horizon & horizonB = Horizon::create_global(1, &event, event);
//  Horizon & horizonC = Horizon::get_global();
//                       Horizon::destroy();
//----------------------------------------------------------------------------

#ifndef JET_APPLICATION_SINGULARITY_HEADER_GUARD
#define JET_APPLICATION_SINGULARITY_HEADER_GUARD

#include "singularity_policies.hpp"
#include "../throw.hpp"
#include "utils/demangle.hpp"

namespace jet
{

struct singleton_error : virtual exception{};

namespace detail
{

// This pointer only depends on type T, so regardless of the threading
// model, only one singularity of type T can be created.
template <typename T> struct singularity_instance
{
    static bool get_enabled;
    static std::unique_ptr<T> ptr;
};

template <typename T> bool singularity_instance<T>::get_enabled{false};
template <typename T> std::unique_ptr<T> singularity_instance<T>::ptr{};

} // detail namespace

template <typename T, template <class> class M = single_threaded>
class singularity
{
public:
    template <class ...A>
    static T& create(A && ...args)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T{std::forward<A>(args)...});
        return *detail::singularity_instance<T>::ptr;
    }

    template <class ...A>
    static T& create_global(A && ...args)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T{std::forward<A>(args)...});
        return *detail::singularity_instance<T>::ptr;
    }

    static void destroy()
    {
        M<T> guard;
        (void)guard;

        if (!detail::singularity_instance<T>::ptr.get())
            JET_THROW_EX(singleton_error) << "singularity<" << demangle(typeid(T).name()) << "> already destroyed";

        detail::singularity_instance<T>::ptr.reset();
    }

    static T& get_global()
    {
        M<T> guard;
        (void)guard;

        if (!detail::singularity_instance<T>::get_enabled)
            JET_THROW_EX(singleton_error) << "singularity<" << demangle(typeid(T).name()) << "> doesn't have global access";

        if (!detail::singularity_instance<T>::ptr.get())
            JET_THROW_EX(singleton_error) << "singularity<" << demangle(typeid(T).name()) << "> isn't created";

        return *detail::singularity_instance<T>::ptr;
    }
private:
    static void verify_not_created()
    {
        if (detail::singularity_instance<T>::ptr.get())
            JET_THROW_EX(singleton_error) << "singularity<" << demangle(typeid(T).name()) << "> is already created";
    }
};

// Convenience macro which generates the required friend statement
// for use inside classes which are created by singularity.
#define FRIEND_CLASS_SINGULARITY \
    template <typename T, template <class> class M> friend class singularity

} //jet namespace

#endif /*JET_APPLICATION_SINGULARITY_HEADER_GUARD*/
