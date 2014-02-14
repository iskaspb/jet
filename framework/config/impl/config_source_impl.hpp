// jet.config library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_CONFIG_CONFIG_SOURCE_IMPL_HEADER_GUARD
#define JET_CONFIG_CONFIG_SOURCE_IMPL_HEADER_GUARD

#include "config_source.hpp"
#include <boost/property_tree/ptree.hpp>

#define ROOT_NODE_NAME     "config"
#define DEFAULT_NODE_NAME  "default"
#define INSTANCE_NODE_NAME "instance"
#define INSTANCE_DELIMITER ".."
#define NODE_DELIMITER "."

namespace jet
{

class config_source::impl: boost::noncopyable
{
public:
    impl(
        std::istream& input,
        const std::string& name,
        config_source::input_format format,
        config_source::file_name_style fname_style);
    impl(
        const std::string& filename,
        config_source::input_format format,
        config_source::file_name_style fname_style);
    impl(
        const boost::property_tree::ptree& root,
        const std::string& name,
        config_source::file_name_style fname_style);
    std::string to_string(bool pretty) const;
    const std::string& name() const { return name_; }
    const boost::property_tree::ptree& get_root() const { return root_; }
private:
    void process_raw_tree(config_source::file_name_style fname_style);
    void normalize_xml_attributes(boost::property_tree::ptree& raw_tree) const;
    void normalize_xml_attributes_impl(
        const boost::property_tree::path& current_path,
        boost::property_tree::ptree& raw_tree) const;
    void normalize_root_node(boost::property_tree::ptree& raw_tree) const;
    void normalize_instance_delimiter(boost::property_tree::ptree& raw_tree) const;
    boost::property_tree::ptree::iterator normalize_instance_delimiter_impl(
        boost::property_tree::ptree& parent,
        const boost::property_tree::ptree::iterator& child) const;
    void copy_unique_children(
        const boost::property_tree::path& current_path,
        const boost::property_tree::ptree& from,
        boost::property_tree::ptree& to) const;
    void normalize_keywords(
        boost::property_tree::ptree& tree,
        config_source::file_name_style fname_style) const;
    boost::property_tree::ptree::iterator normalize_keywords_impl(
        boost::property_tree::ptree& tree,
        const boost::property_tree::ptree::iterator& child,
        config_source::file_name_style fname_style) const;
    //...
    boost::property_tree::ptree root_;
    std::string name_;
};

}//namespace jet

#endif /*JET_CONFIG_CONFIG_SOURCE_IMPL_HEADER_GUARD*/
