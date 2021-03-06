// jet.config library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "config_source_impl.hpp"
#include "config_throw.hpp"
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/adaptor/reversed.hpp>

namespace PT           = boost::property_tree;
using value_type       = PT::ptree::value_type;
using tree_iter        = PT::ptree::iterator;
using assoc_tree_iter  = PT::ptree::assoc_iterator;
using cassoc_tree_iter = PT::ptree::const_assoc_iterator;
using tree             = PT::ptree;
using path             = PT::path;

namespace jet
{

namespace
{

inline std::string prune_string(const std::string& data, size_t maxSize = 10)
{
    if(data.size() > maxSize)
    {
        if(maxSize < 6)
            return data.substr(0, maxSize);
        else
            return data.substr(0, maxSize - 3) + "...";
    }
    return data;
}

class tree_validator: boost::noncopyable
{
public:
    tree_validator(const std::string& source_name, const tree& root):
        source_name_{source_name}, root_{root}
    {
        assert(root_.front().first == ROOT_NODE_NAME);
    }
    void check_tree_does_not_have_data_and_attribute_nodes() const
    {
        const tree::value_type& config { root_.front() };
        
        check_node_does_not_have_data_and_attribute(config.first, config.second);
    }
    void check_no_data_in_config_node() const
    {
        const tree::value_type& config {root_.front()};
        if(!config.second.data().empty())
            JET_THROW_CFG()
                << "Invalid data node '" << prune_string(config.second.data())
                << "' under '" ROOT_NODE_NAME "' node in config source '" << source_name_ << '\'';
    }
    void check_no_data_in_default_node() const
    {
        const tree::value_type& config{root_.front()};
        const cassoc_tree_iter default_iter { config.second.find(DEFAULT_NODE_NAME) };
        if(config.second.not_found() == default_iter)
            return;
        if(!default_iter->second.data().empty())
            JET_THROW_CFG()
                << "Invalid data node '" << prune_string(default_iter->second.data())
                << "' under '" DEFAULT_NODE_NAME "' node in config source '" << source_name_ << '\'';
    }
    void check_no_data_in_app_and_instance_node() const
    {
        const tree::value_type& config { root_.front() };
        for(const tree::value_type& app_node : config.second)
        {
            if(DEFAULT_NODE_NAME == app_node.first)
                continue;
            if(!app_node.second.data().empty())
                JET_THROW_CFG()
                    << "Invalid data node '" << prune_string(app_node.second.data())
                    << "' under '" << app_node.first << "' node in config source '" << source_name_ << '\'';
            const cassoc_tree_iter instance_iter { app_node.second.find(INSTANCE_NODE_NAME) };
            if(app_node.second.not_found() == instance_iter)
                continue;
            for(const tree::value_type& instanceNode : instance_iter->second)
            {
                if(!instanceNode.second.data().empty())
                {
                    std::string instance_name { app_node.first };
                    instance_name += INSTANCE_DELIMITER;
                    instance_name += instanceNode.first;
                    JET_THROW_CFG()
                        << "Invalid data node '" << prune_string(instanceNode.second.data())
                        << "' under '" << instance_name << "' node in config source '"
                        << source_name_ << '\'';
                }
            }
        }
    }
    void check_no_default_node_duplicates() const
    {
        const tree::value_type& config { root_.front() };
        if(config.second.count(DEFAULT_NODE_NAME) > 1)
            JET_THROW_CFG()
                << "Duplicate default node in config source '" << source_name_ << '\'';
    }
    void check_no_default_subnode_duplicates() const
    {
        const tree::value_type& config { root_.front() };
        const cassoc_tree_iter default_iter { config.second.find(DEFAULT_NODE_NAME) };
        if(config.second.not_found() == default_iter)
            return;

        for(const tree::value_type& default_child : default_iter->second)
        {
            if(default_iter->second.count(default_child.first) > 1)
                JET_THROW_CFG()
                    << "Duplicate default node '" << default_child.first
                    << "' in config source '" << source_name_ << '\'';
        }
    }
    void check_no_default_instance_node() const
    {
        const tree::value_type& config { root_.front() };
        const cassoc_tree_iter default_iter { config.second.find(DEFAULT_NODE_NAME) };
        if(config.second.not_found() == default_iter)
            return;
        for(const tree::value_type& node : default_iter->second)
        {
            const std::string node_name { boost::to_lower_copy(node.first) };
            if(node_name == INSTANCE_NODE_NAME)
                JET_THROW_CFG()
                    << "config source '" << source_name_
                    << "' is invalid: '" DEFAULT_NODE_NAME "' node can not contain '"
                    << node.first<< "' node";
        }
    }
    void check_no_direct_default_attributes() const
    {
        const tree::value_type& config { root_.front() };
        const cassoc_tree_iter default_iter { config.second.find(DEFAULT_NODE_NAME) };
        if(config.second.not_found() == default_iter)
            return;
        for(const tree::value_type& node : default_iter->second)
        {
            if(!node.second.data().empty())
                JET_THROW_CFG()
                    << "config source '" << source_name_
                    << "' is invalid: '" DEFAULT_NODE_NAME
                    "' node can not contain direct properties. See '" DEFAULT_NODE_NAME "."
                    << node.first << "' property";
        }
    }
    void check_no_app_node_duplicates() const
    {
        const tree::value_type& config { root_.front() };
        for(const tree::value_type& node : config.second)
        {
            if(config.second.count(node.first) > 1)
                JET_THROW_CFG()
                    << "Duplicate node '" << node.first
                    << "' in config source '" << source_name_ << '\'';
        }
    }
    void check_no_instance_node_duplicates() const
    {
        const tree::value_type& config { root_.front() };
        for(const tree::value_type& node : config.second)
        {
            check_no_instance_node_duplicates_impl(node.first, node.second);
        }
    }
    void check_no_instance_subnode_duplicates() const
    {
        const tree::value_type& config { root_.front() };
        for(const tree::value_type& node : config.second)
        {
            check_no_instance_subnode_duplicates_impl(node.first, node.second);
        }
    }
private:
    void check_node_does_not_have_data_and_attribute(const path& current_path, const tree& tree) const
    {
        if(!tree.empty() && !tree.data().empty())
            JET_THROW_CFG()
                << "Invalid element '" << current_path.dump()
                << "' in config source '" << source_name_
                << "' contains both value and child attributes";
        for(const tree::value_type& node : tree)
        {
            const path node_path { current_path/path{node.first} };
            check_node_does_not_have_data_and_attribute(node_path, node.second);
        }
    }
    void check_no_instance_node_duplicates_impl(const std::string& app_name, const tree& app_node) const
    {
        if(DEFAULT_NODE_NAME == app_name)//...in fact this is not an application node
            return;
        if(app_node.count(INSTANCE_NODE_NAME) > 1)
            JET_THROW_CFG()
                << "Duplicate " INSTANCE_NODE_NAME " node under '" << app_name
                << "' node in config source '" << source_name_ << '\'';
    }
    void check_no_instance_subnode_duplicates_impl(const std::string& app_name, const tree& app_node) const
    {
        if(DEFAULT_NODE_NAME == app_name)//...in fact this is not an application node
            return;
        const cassoc_tree_iter instance_iter { app_node.find(INSTANCE_NODE_NAME) };
        if(app_node.not_found() == instance_iter)
            return;
        for(const tree::value_type& instance_subnode : instance_iter->second)
        {
            if(instance_iter->second.count(instance_subnode.first) > 1)
            {
                std::string instance_name{app_name};
                instance_name += INSTANCE_DELIMITER;
                instance_name += instance_subnode.first;
                JET_THROW_CFG()
                    << "Duplicate node '" << instance_name << "' in config source '"
                    << source_name_ << '\'';
            }
        }
    }
    const std::string& source_name_;
    const tree& root_;
};

}//anonymous namespace

config_source::impl::impl(
    std::istream& input,
    const std::string& name,
    config_source::input_format format,
    config_source::file_name_style fname_style):
    name_{name}
{
    switch (format)
    {
        case config_source::xml:
            PT::read_xml(
                input,
                root_,
                PT::xml_parser::trim_whitespace | PT::xml_parser::no_comments);
            normalize_xml_attributes(root_);
            break;
        default:
            JET_THROW_CFG() << "Parsing of config format " << format << " is not implemented";
    }
    process_raw_tree(fname_style);
}

config_source::impl::impl(
    const std::string& filename,
    config_source::input_format format,
    config_source::file_name_style fname_style):
    name_{filename}
{
    switch (format)
    {
        case config_source::xml:
            PT::read_xml(
                filename,
                root_,
                PT::xml_parser::trim_whitespace | PT::xml_parser::no_comments);
            normalize_xml_attributes(root_);
            break;
        default:
            JET_THROW_CFG() << "Parsing of config format " << format << " is not implemented";
    }
    process_raw_tree(fname_style);
}

config_source::impl::impl(
    const boost::property_tree::ptree& root,
    const std::string& name,
    config_source::file_name_style fname_style):
    root_{root},
    name_{name}
{
    process_raw_tree(fname_style);
}

void config_source::impl::process_raw_tree(config_source::file_name_style fname_style)
{
    if (root_.empty())
        JET_THROW_CFG() << "config source '" << name() << "' is empty";

    normalize_root_node(root_);
    normalize_keywords(root_, fname_style);
    normalize_instance_delimiter(root_);
    
    const tree_validator validator(name(), root_);
    validator.check_no_data_in_config_node();
    validator.check_no_data_in_default_node();
    validator.check_no_data_in_app_and_instance_node();
    validator.check_tree_does_not_have_data_and_attribute_nodes();
    validator.check_no_default_node_duplicates();
    validator.check_no_default_subnode_duplicates();
    validator.check_no_default_instance_node();
    validator.check_no_direct_default_attributes();
    validator.check_no_app_node_duplicates();
    validator.check_no_instance_node_duplicates();
    validator.check_no_instance_subnode_duplicates();
}

std::string config_source::impl::to_string(bool pretty) const
{
    std::stringstream strm;
    if(pretty)
        PT::write_xml(strm, root_, PT::xml_writer_make_settings(' ', 2));
    else
        PT::write_xml(strm, root_);
    std::string firstString;
    std::getline(strm, firstString);
    if(firstString.size() > 2 && //...get rid of the first line with <?xml ...?>
        '<' == firstString[0] && '?' == firstString[1])
        return strm.str().substr(firstString.size() + 1);
    return strm.str();
}

void config_source::impl::normalize_xml_attributes(tree& raw_tree) const
{
    if (raw_tree.empty())
        JET_THROW_CFG() << "Config source '" << name() << "' is empty";
    
    normalize_xml_attributes_impl(path(), raw_tree);
}

void config_source::impl::normalize_xml_attributes_impl(const path& current_path, tree& raw_tree) const
{
    const tree_iter end{raw_tree.end()};
    for(tree_iter iter = raw_tree.begin(); end != iter;)
    {
        if("<xmlattr>" == iter->first)
        {
            copy_unique_children(current_path, iter->second, raw_tree);
            iter = raw_tree.erase(iter);
        }
        else
        {
            normalize_xml_attributes_impl(current_path/path{iter->first}, iter->second);
            ++iter;
        }
    }
}

void config_source::impl::normalize_root_node(tree& raw_tree) const
{
    if(raw_tree.size() == 1 && ROOT_NODE_NAME == boost::to_lower_copy(raw_tree.front().first))
        return;//...this tree is already normalized
    for(const tree::value_type& child : raw_tree)
    {
        const std::string& child_name = child.first;
        if(ROOT_NODE_NAME == boost::to_lower_copy(child_name))
            JET_THROW_CFG()
                << "Invalid config source '" << name()
                << "'. '" ROOT_NODE_NAME "' must be root node";
    }
    tree newTree;
    newTree.push_back({ ROOT_NODE_NAME, tree{}});
    newTree.front().second.swap(raw_tree);
    newTree.swap(raw_tree);
}

namespace
{
inline tree_iter rename_node(tree& parent, const tree_iter& node_iter, const std::string& new_name)
{
    const tree_iter new_node_iter { parent.insert(node_iter, {new_name, tree{}}) };
    node_iter->second.swap(new_node_iter->second);
    parent.erase(node_iter);
    return new_node_iter;
}
}//anonymous namespace

void config_source::impl::normalize_keywords(
    tree& root, config_source::file_name_style fname_style) const
{
    const std::string& root_name { root.front().first };
    assert(ROOT_NODE_NAME == boost::to_lower_copy(root_name));
    if(ROOT_NODE_NAME != root_name)
        rename_node(root, root.begin(), ROOT_NODE_NAME);
    tree& config_node { root.front().second };
    for(tree_iter iter = config_node.begin(); config_node.end() != iter; ++iter)
    {
        iter = normalize_keywords_impl(config_node, iter, fname_style);
    }
}

tree_iter config_source::impl::normalize_keywords_impl(
    tree& parent, const tree_iter& child_iter, config_source::file_name_style fname_style) const
{
    if(DEFAULT_NODE_NAME == boost::to_lower_copy(child_iter->first))
    {
        if(DEFAULT_NODE_NAME != child_iter->first)
            return rename_node(parent, child_iter, DEFAULT_NODE_NAME);
    }
    else
    {
        tree& app_node { child_iter->second };
        for(tree_iter iter = app_node.begin(); app_node.end() != iter; ++iter)
        {
            const std::string& node_name { iter->first };
            if(INSTANCE_NODE_NAME == boost::to_lower_copy(node_name))
            {
                if(INSTANCE_NODE_NAME != node_name)
                    iter = rename_node(app_node, iter, INSTANCE_NODE_NAME);
            }
        }
        if(config_source::case_insensitive == fname_style)
        {
            const std::string low_case_name { boost::to_lower_copy(child_iter->first) };
            if(child_iter->first != low_case_name)
                return rename_node(parent, child_iter, low_case_name);
        }
    }
    return child_iter;
}

void config_source::impl::normalize_instance_delimiter(tree& root) const
{
    tree& config{ root.front().second };
    for(auto iter(config.begin()), end(config.end()); end != iter;)
    {
        iter = normalize_instance_delimiter_impl(config, iter);
    }
}

namespace
{
inline tree_iter find_or_insert_child(tree& parent, tree_iter insert_position, const std::string& key)
{
    const assoc_tree_iter assoc_iter { parent.find(key) };
    if(parent.not_found() != assoc_iter)
        return parent.to_iterator(assoc_iter);
    return parent.insert(insert_position, {key, tree{}});
}
}//anonymous namespace

tree_iter config_source::impl::normalize_instance_delimiter_impl(
    tree& parent,
    const tree_iter& child_iter) const
{
    const std::string& child_name { child_iter->first };
    const size_t pos { child_name.find(INSTANCE_DELIMITER) };
    if(std::string::npos == pos)
    {
        tree_iter next_iter { child_iter };
        return ++next_iter;
    }
    const std::string app_name{ child_name.substr(0, pos) };
    const std::string instance_name{ child_name.substr(pos + sizeof(INSTANCE_DELIMITER) - 1) };
    if(app_name.empty() || instance_name.empty())
        JET_THROW_CFG()
            << "Invalid '" INSTANCE_DELIMITER "' in element '" << child_name
            << "' in config source '" << name()
            << "'. Expected format 'app_name" INSTANCE_DELIMITER "instance_name'";
    if(boost::to_lower_copy(app_name) == DEFAULT_NODE_NAME)
        JET_THROW_CFG()
            << "Default node '" << child_name
            << "' can't have instance. Found in config source '" << name() << '\'';
    const tree_iter new_child_iter { find_or_insert_child(parent, child_iter, app_name) };
    const tree_iter grand_child_iter { find_or_insert_child(
        new_child_iter->second, new_child_iter->second.end(), INSTANCE_NODE_NAME) };
    const assoc_tree_iter grand_grand_child_iter { grand_child_iter->second.find(instance_name) };
    if(grand_grand_child_iter != grand_child_iter->second.not_found())
        JET_THROW_CFG()
            << "Duplicate node '" << child_name << "' in config source '" << name() << '\'';
    child_iter->second.swap(
        grand_child_iter->second.push_back({instance_name, tree{}})->second);
    
    return parent.erase(child_iter);
}

void config_source::impl::copy_unique_children(const path& current_path, const tree& from, tree& to) const
{
    for(const tree::value_type& node : boost::adaptors::reverse(from))
    {
        if(from.count(node.first) > 1)
            JET_THROW_CFG()
                << "Duplicate definition of attribute '" << (current_path/path(node.first)).dump()
                << "' in config '" << name() << '\'';
        to.push_front(node);
    }
}

config_source::config_source(std::unique_ptr<impl> impl): impl_(std::move(impl)) {}

config_source::~config_source() {}

std::string config_source::to_string(output_type type) const try
{
    return impl_->to_string(pretty == type);
}
catch(const std::exception& ex)
{
    JET_THROW_CFG()
        << "Couldn't stringify config '" << impl_->name()
        << "'. Reason: " << ex.what();
}

const std::string& config_source::name() const
{
    return impl_->name();
}

config_source::config_source(const config_source& other):
    impl_{new impl{*other.impl_}}
{}

config_source& config_source::operator=(const config_source& other)
{
    std::unique_ptr<impl> tmp{new impl{*other.impl_}};
    impl_.swap(tmp);
    return *this;
}

config_source::config_source(config_source&& other):
    impl_{std::move(other.impl_)}
{}

config_source& config_source::operator=(config_source&& other)
{
    impl_.swap(other.impl_);
    return *this;
}

config_source config_source::from_string::create() try
{
    std::stringstream strm { source_ };
    return std::unique_ptr<impl>{new impl{strm, name_, fmt_, name_style_}};
}
catch(const std::exception& ex)
{
    JET_THROW_CFG()
        << "Couldn't parse config '" << name_ << '\'';
}

config_source config_source::from_stream::create() try
{
    return std::unique_ptr<impl>{new impl{*strm_, name_, fmt_, name_style_}};
}
catch(const std::exception& ex)
{
    JET_THROW_CFG()
        << "Couldn't parse config '" << name_ << '\'';
}

config_source config_source::from_file::create() try
{
    return std::unique_ptr<impl>{new impl{filename_, fmt_, name_style_}};
}
catch(const std::exception& ex)
{
    JET_THROW_CFG()
        << "Couldn't parse config '" << filename_ << '\'';
}

config_source config_source::from_cmd_line::create() try
{
    tree root;
    if(app_name_.empty())
        JET_THROW_CFG()
            << "Empty application name. Couldn't create configuration from '" << source_ << '\'';
    {//...prepare configuration tree
        const tree_iter app_node_iter {
            root.insert(root.begin(), {app_name_, tree{}})};

        tree& config { instance_name_.empty()?
            app_node_iter->second:
            app_node_iter->second.put(
                INSTANCE_NODE_NAME NODE_DELIMITER + instance_name_, std::string{})};

        std::vector<std::string> properties;//TODO: define ':' and '=' as symbolic constants
        boost::split(properties, source_, boost::is_any_of(":"), boost::token_compress_on);
        for(std::string& property : properties)
        {
            boost::trim(property);
            if(property.empty())
                continue;
            std::vector<std::string> name_value;
            boost::split(name_value, property, boost::is_any_of("="));
            if(name_value.size() != 2)
                JET_THROW_CFG()
                    << "Invalid property '" << property << "' in config source '" << source_ << '\'';
            boost::trim(name_value[0]);
            boost::trim(name_value[1]);
            if(name_value[0].empty() || name_value[1].empty())
                JET_THROW_CFG()
                    << "Invalid property '" << property << "' in config source '" << source_ << '\'';
            config.add(name_value[0], name_value[1]);
        }
    }
    return std::unique_ptr<impl>{new impl{root, source_, config_source::case_sensitive}};
}
catch(const std::exception& ex)
{
    JET_THROW_CFG()
        << "Couldn't create configuration from '" << source_ << '\'';
}


}//namespace jet
