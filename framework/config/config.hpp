// jet.config library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_CONFIG_CONFIG_HEADER_GUARD
#define JET_CONFIG_CONFIG_HEADER_GUARD

#include "config_source.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace jet
{

class config_node
{
    class impl;
    config_node(
        const std::string& path,
        const std::shared_ptr<impl>& impl,
        const void* tree_node);
public:
    config_node(const config_node& copee);
    config_node& operator=(const config_node& copee);
    virtual ~config_node();
    std::string name() const;//...name := app_name [ '..' instance_name ] [ '.' path ]
    const std::string& app_name() const;
    const std::string& instance_name() const;
    const std::string& path() const { return path_; }
    const std::string node_name() const; //...this is the last part of 'path', e.g. for path 'long.path.to.element' node_name equal to 'element'
    
    config_node get_node(const std::string& path) const;
    boost::optional<config_node> get_node_optional(const std::string& path) const;
    std::vector<config_node> get_children_of(const std::string& path = std::string()) const;
    
    std::string get(const std::string& attr_name = std::string()) const;
    template<typename T>
    T get(const std::string& attr_name = std::string()) const;
    
    boost::optional<std::string> get_optional(const std::string& attr_name = std::string()) const;
    template<typename T>
    boost::optional<T> get_optional(const std::string& attr_name = std::string()) const;
    
    std::string get(const std::string& attr_name, const std::string& default_value) const;
    template<typename T>
    T get(const std::string& attr_name, const T& default_value) const;
protected:
    config_node(const std::string& app_name, const std::string& instance_name);
    void merge(const config_source& source);
    void lock();
    void print(std::ostream& os) const;
private:
    void throw_value_conversion_error[[noreturn]](const std::string& attr_name, const std::string& value) const;
    friend std::ostream& operator<<(std::ostream& os, const config_node& config);
    //...
    std::string path_;
    std::shared_ptr<impl> impl_;
    const void* tree_node_;
};

using config_nodes = std::vector<config_node>;

extern std::ostream& operator<<(std::ostream& os, const config_node& config);

struct config_lock {};
extern const config_lock lock;//...this is used to lock config, that is to finish creation from config_source. Efectively it makes config immutable

class config: public config_node
{
public:
    explicit config(
        const std::string& app_name,
        const std::string& instance_name = std::string());

    config& operator<<(const config_source& source);
    void operator<<(config_lock);
};

template<typename T>
inline T config_node::get(const std::string& attr_name) const
{
    const std::string value(get(attr_name));
    try
    {
        return boost::lexical_cast<T>(value);
    }
    catch(const boost::bad_lexical_cast&)
    {
        throw_value_conversion_error(attr_name, value);
    }
}

template<typename T>
inline boost::optional<T> config_node::get_optional(const std::string& attr_name) const
{
    const boost::optional<std::string> value(get_optional(attr_name));
    if(!value)
        return boost::none;
    try
    {
        return boost::lexical_cast<T>(*value);
    }
    catch(const boost::bad_lexical_cast&)
    {
        throw_value_conversion_error(attr_name, *value);
    }
}

template<typename T>
inline T config_node::get(const std::string& attr_name, const T& default_value) const
{
    boost::optional<std::string> value(get_optional(attr_name));
    if(!value)
        return default_value;
    try
    {
        return boost::lexical_cast<T>(*value);
    }
    catch(const boost::bad_lexical_cast&)
    {
        throw_value_conversion_error(attr_name, *value);
    }
}

}//namespace jet

#endif /*JET_CONFIG_CONFIG_HEADER_GUARD*/
