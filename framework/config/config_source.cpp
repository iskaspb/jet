//
//  config_source.cpp
//
//  Created by Alexey Tkachenko on 1/9/13.
//  This code belongs to public domain. You can do with it whatever you want without any guarantee.
//
#include "config_source_impl.hpp"
#include "config_error.hpp"
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

namespace PT = boost::property_tree;
typedef PT::ptree::value_type value_type;
typedef PT::ptree::iterator tree_iter;
typedef PT::ptree::assoc_iterator assoc_tree_iter;
typedef PT::ptree::const_assoc_iterator cassoc_tree_iter;
typedef PT::ptree tree;
typedef PT::path path;

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
        source_name_(source_name), root_(root)
    {
        assert(root_.front().first == ROOT_NODE_NAME);
    }
    void check_tree_does_not_have_data_and_attribute_nodes() const
    {
        const tree::value_type& config(root_.front());
        
        check_node_does_not_have_data_and_attribute(config.first, config.second);
    }
    void check_no_data_in_config_node() const
    {
        const tree::value_type& config(root_.front());
        if(!config.second.data().empty())
            throw config_error(str(
                boost::format("Invalid data node '%1%' under '" ROOT_NODE_NAME "' node in config source '%2%'") %
                prune_string(config.second.data()) %
                source_name_));
    }
    void check_o_data_in_shared_node() const
    {
        const tree::value_type& config(root_.front());
        const cassoc_tree_iter sharedIter = config.second.find(SHARED_NODE_NAME);
        if(config.second.not_found() == sharedIter)
            return;
        if(!sharedIter->second.data().empty())
            throw config_error(str(
                boost::format("Invalid data node '%1%' under '" SHARED_NODE_NAME "' node in config source '%2%'") %
                prune_string(sharedIter->second.data()) %
                source_name_));
    }
    void check_no_data_in_app_and_instance_node() const
    {
        const tree::value_type& config(root_.front());
        BOOST_FOREACH(const tree::value_type& app_node, config.second)
        {
            if(SHARED_NODE_NAME == app_node.first)
                continue;
            if(!app_node.second.data().empty())
                throw config_error(str(
                    boost::format("Invalid data node '%1%' under '%2%' node in config source '%3%'") %
                    prune_string(app_node.second.data()) %
                    app_node.first %
                    source_name_));
            const cassoc_tree_iter instance_iter = app_node.second.find(INSTANCE_NODE_NAME);
            if(app_node.second.not_found() == instance_iter)
                continue;
            BOOST_FOREACH(const tree::value_type& instanceNode, instance_iter->second)
            {
                if(!instanceNode.second.data().empty())
                {
                    std::string instance_name(app_node.first);
                    instance_name += INSTANCE_DELIMITER;
                    instance_name += instanceNode.first;
                    throw config_error(str(
                        boost::format("Invalid data node '%1%' under '%2%' node in config source '%3%'") %
                        prune_string(instanceNode.second.data()) %
                        instance_name %
                        source_name_));
                }
            }
        }
    }
    void check_no_shared_node_duplicates() const
    {
        const tree::value_type& config(root_.front());
        if(config.second.count(SHARED_NODE_NAME) > 1)
            throw config_error(str(
                boost::format("Duplicate shared node in config source '%1%'") % source_name_));
    }
    void check_no_shared_subnode_duplicates() const
    {
        const tree::value_type& config(root_.front());
        const cassoc_tree_iter sharedIter = config.second.find(SHARED_NODE_NAME);
        if(config.second.not_found() == sharedIter)
            return;

        BOOST_FOREACH(const tree::value_type& sharedChild, sharedIter->second)
        {
            if(sharedIter->second.count(sharedChild.first) > 1)
                throw config_error(str(
                    boost::format("Duplicate shared node '%1%' in config source '%2%'") %
                    sharedChild.first %
                    source_name_));
        }
    }
    void check_no_shared_instance_node() const
    {
        const tree::value_type& config(root_.front());
        const cassoc_tree_iter sharedIter = config.second.find(SHARED_NODE_NAME);
        if(config.second.not_found() == sharedIter)
            return;
        BOOST_FOREACH(const tree::value_type& node, sharedIter->second)
        {
            const std::string node_name = boost::to_lower_copy(node.first);
            if(node_name == INSTANCE_NODE_NAME)
                throw config_error(str(
                    boost::format("config source '%1%' is invalid: '" SHARED_NODE_NAME "' node can not contain '%2%' node") %
                    source_name_ %
                    node.first));
        }
    }
    void check_no_direct_shared_attributes() const
    {
        const tree::value_type& config(root_.front());
        const cassoc_tree_iter sharedIter = config.second.find(SHARED_NODE_NAME);
        if(config.second.not_found() == sharedIter)
            return;
        BOOST_FOREACH(const tree::value_type& node, sharedIter->second)
        {
            if(!node.second.data().empty())
                throw config_error(str(
                    boost::format("config source '%1%' is invalid: '" SHARED_NODE_NAME "' node can not contain direct properties. See '" SHARED_NODE_NAME ".%2%' property") %
                    source_name_ %
                    node.first));
        }
    }
    void check_no_app_node_duplicates() const
    {
        const tree::value_type& config(root_.front());
        BOOST_FOREACH(const tree::value_type& node, config.second)
        {
            if(config.second.count(node.first) > 1)
                throw config_error(str(
                    boost::format("Duplicate node '%1%' in config source '%2%'") %
                    node.first %
                    source_name_));
        }
    }
    void check_no_instance_node_duplicates() const
    {
        const tree::value_type& config(root_.front());
        BOOST_FOREACH(const tree::value_type& node, config.second)
        {
            check_no_instance_node_duplicates_impl(node.first, node.second);
        }
    }
    void check_no_instance_subnode_duplicates() const
    {
        const tree::value_type& config(root_.front());
        BOOST_FOREACH(const tree::value_type& node, config.second)
        {
            check_no_instance_subnode_duplicates_impl(node.first, node.second);
        }
    }
private:
    void check_node_does_not_have_data_and_attribute(const path& current_path, const tree& tree) const
    {
        if(!tree.empty() && !tree.data().empty())
            throw config_error(str(
                boost::format("Invalid element '%1%' in config source '%2%' contains both value and child attributes") %
                current_path.dump() %
                source_name_));
        BOOST_FOREACH(const tree::value_type& node, tree)
        {
            const path node_path(current_path/path(node.first));
            check_node_does_not_have_data_and_attribute(node_path, node.second);
        }
    }
    void check_no_instance_node_duplicates_impl(const std::string& app_name, const tree& app_node) const
    {
        if(SHARED_NODE_NAME == app_name)//...in fact this is not an application node
            return;
        if(app_node.count(INSTANCE_NODE_NAME) > 1)
            throw config_error(str(
                boost::format("Duplicate " INSTANCE_NODE_NAME " node under '%1%' node in config source '%2%'") %
                app_name %
                source_name_));
    }
    void check_no_instance_subnode_duplicates_impl(const std::string& app_name, const tree& app_node) const
    {
        if(SHARED_NODE_NAME == app_name)//...in fact this is not an application node
            return;
        const cassoc_tree_iter instance_iter = app_node.find(INSTANCE_NODE_NAME);
        if(app_node.not_found() == instance_iter)
            return;
        BOOST_FOREACH(const tree::value_type& instance_subnode, instance_iter->second)
        {
            if(instance_iter->second.count(instance_subnode.first) > 1)
            {
                std::string instance_name(app_name);
                instance_name += INSTANCE_DELIMITER;
                instance_name += instance_subnode.first;
                throw config_error(str(
                    boost::format("Duplicate node '%1%' in config source '%2%'") %
                    instance_name %
                    source_name_));
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
    name_(name)
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
            config_error(
                boost::str(
                    boost::format("Parsing of config format %1% is not implemented") %
                    format));
    }
    process_raw_tree(fname_style);
}

config_source::impl::impl(
    const std::string& filename,
    config_source::input_format format,
    config_source::file_name_style fname_style):
    name_(filename)
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
            config_error(
                boost::str(
                    boost::format("Parsing of config format %1% is not implemented") %
                    format));
    }
    process_raw_tree(fname_style);
}

void config_source::impl::process_raw_tree(config_source::file_name_style fname_style)
{
    if (root_.empty())
        throw config_source(str(
            boost::format("config source '%1%' is empty") % name()));

    normalize_root_node(root_);
    normalize_keywords(root_, fname_style);
    normalize_instance_delimiter(root_);
    
    const tree_validator validator(name(), root_);
    validator.check_no_data_in_config_node();
    validator.check_o_data_in_shared_node();
    validator.check_no_data_in_app_and_instance_node();
    validator.check_tree_does_not_have_data_and_attribute_nodes();
    validator.check_no_shared_node_duplicates();
    validator.check_no_shared_subnode_duplicates();
    validator.check_no_shared_instance_node();
    validator.check_no_direct_shared_attributes();
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
        throw config_source(str(
            boost::format("config source '%1%' is empty") % name()));
    
    normalize_xml_attributes_impl(path(), raw_tree);
}

void config_source::impl::normalize_xml_attributes_impl(const path& current_path, tree& raw_tree) const
{
    const tree_iter end(raw_tree.end());
    for(tree_iter iter = raw_tree.begin(); end != iter;)
    {
        if("<xmlattr>" == iter->first)
        {
            copy_unique_children(current_path, iter->second, raw_tree);
            iter = raw_tree.erase(iter);
        }
        else
        {
            normalize_xml_attributes_impl(current_path/path(iter->first), iter->second);
            ++iter;
        }
    }
}

void config_source::impl::normalize_root_node(tree& raw_tree) const
{
    if(raw_tree.size() == 1 && ROOT_NODE_NAME == boost::to_lower_copy(raw_tree.front().first))
        return;//...this tree is already normalized
    BOOST_FOREACH(const tree::value_type& child, raw_tree)
    {
        const std::string& child_name = child.first;
        if(ROOT_NODE_NAME == boost::to_lower_copy(child_name))
            throw config_error(str(
                boost::format("Invalid config source '%1%'. '" ROOT_NODE_NAME "' must be root node") %
                name()));
    }
    tree newTree;
    newTree.push_back(tree::value_type(ROOT_NODE_NAME, tree()));
    newTree.front().second.swap(raw_tree);
    newTree.swap(raw_tree);
}

namespace
{
inline tree_iter rename_node(tree& parent, const tree_iter& node_iter, const std::string& new_name)
{
    const tree_iter new_node_iter = parent.insert(node_iter, tree::value_type(new_name, tree()));
    node_iter->second.swap(new_node_iter->second);
    parent.erase(node_iter);
    return new_node_iter;
}
}//anonymous namespace

void config_source::impl::normalize_keywords(
    tree& root, config_source::file_name_style fname_style) const
{
    const std::string& root_name(root.front().first);
    assert(ROOT_NODE_NAME == boost::to_lower_copy(root_name));
    if(ROOT_NODE_NAME != root_name)
        rename_node(root, root.begin(), ROOT_NODE_NAME);
    tree& config_node = root.front().second;
    for(tree_iter iter = config_node.begin(); config_node.end() != iter; ++iter)
    {
        iter = normalize_keywords_impl(config_node, iter, fname_style);
    }
}

tree_iter config_source::impl::normalize_keywords_impl(
    tree& parent, const tree_iter& child_iter, config_source::file_name_style fname_style) const
{
    if(SHARED_NODE_NAME == boost::to_lower_copy(child_iter->first))
    {
        if(SHARED_NODE_NAME != child_iter->first)
            return rename_node(parent, child_iter, SHARED_NODE_NAME);
    }
    else
    {
        tree& app_node = child_iter->second;
        for(tree_iter iter = app_node.begin(); app_node.end() != iter; ++iter)
        {
            const std::string& node_name = iter->first;
            if(INSTANCE_NODE_NAME == boost::to_lower_copy(node_name))
            {
                if(INSTANCE_NODE_NAME != node_name)
                    iter = rename_node(app_node, iter, INSTANCE_NODE_NAME);
            }
        }
        if(config_source::case_insensitive == fname_style)
        {
            const std::string low_case_name = boost::to_lower_copy(child_iter->first);
            if(child_iter->first != low_case_name)
                return rename_node(parent, child_iter, low_case_name);
        }
    }
    return child_iter;
}

void config_source::impl::normalize_instance_delimiter(tree& root) const
{
    tree& config(root.front().second);
    const tree_iter end(config.end());
    for(tree_iter iter(config.begin()); end != iter;)
    {
        iter = normalize_instance_delimiter_impl(config, iter);
    }
}

namespace
{
inline tree_iter find_or_insert_child(tree& parent, tree_iter insert_position, const std::string& key)
{
    const assoc_tree_iter assoc_iter = parent.find(key);
    if(parent.not_found() != assoc_iter)
        return parent.to_iterator(assoc_iter);
    return parent.insert(insert_position, value_type(key, tree()));
}
}//anonymous namespace

tree_iter config_source::impl::normalize_instance_delimiter_impl(
    tree& parent,
    const tree_iter& child_iter) const
{
    const std::string& child_name(child_iter->first);
    const size_t pos = child_name.find(INSTANCE_DELIMITER);
    if(std::string::npos == pos)
    {
        tree_iter next_iter(child_iter);
        std::advance(next_iter, 1);
        return next_iter;
    }
    const std::string app_name(child_name.substr(0, pos));
    const std::string instance_name(child_name.substr(pos + sizeof(INSTANCE_DELIMITER) - 1));
    if(app_name.empty() || instance_name.empty())
        throw config_error(str(
            boost::format(
                "Invalid '"
                INSTANCE_DELIMITER
                "' in element '%1%' in config source '%2%'. Expected format 'app_name"
                INSTANCE_DELIMITER
                "instance_name'") %
                child_name %
                name()));
    if(boost::to_lower_copy(app_name) == SHARED_NODE_NAME)
        throw config_error(str(
            boost::format("Shared node '%1%' can't have instance. Found in config source '%2%'") %
            child_name %
            name()));
    const tree_iter new_child_iter = find_or_insert_child(parent, child_iter, app_name);
    const tree_iter grand_child_iter = find_or_insert_child(
        new_child_iter->second, new_child_iter->second.end(), INSTANCE_NODE_NAME);
    const assoc_tree_iter grand_grand_child_iter = grand_child_iter->second.find(instance_name);
    if(grand_grand_child_iter != grand_child_iter->second.not_found())
        throw config_error(str(
            boost::format("Duplicate node '%1%' in config source '%2%'") %
            child_name %
            name()));
    child_iter->second.swap(
        grand_child_iter->second.push_back(
            tree::value_type(instance_name, tree()))->second);
    
    return parent.erase(child_iter);
}

void config_source::impl::copy_unique_children(const path& current_path, const tree& from, tree& to) const
{
    BOOST_REVERSE_FOREACH(const tree::value_type& node, from)
    {
        if(from.count(node.first) > 1)
            throw config_error(str(
                boost::format("Duplicate definition of attribute '%1%' in config '%2%'") %
                    (current_path/path(node.first)).dump() %
                    name()));
        to.push_front(node);
    }
}

config_source::config_source(
    const std::string& source,
    const std::string& name,
    input_format format,
    file_name_style fname_style) try
{
    std::stringstream strm;
    strm << source;
    impl_.reset(new impl(strm, name, format, fname_style));
}
catch(const PT::ptree_error& ex)
{
    throw config_error(str(
        boost::format("Couldn't parse config '%1%'. Reason: %2%") %
        name %
        ex.what()));
}

config_source::config_source(
    std::istream& source,
    const std::string& name,
    input_format format,
    file_name_style fname_style) try :
    impl_(new impl(source, name, format, fname_style))
{
}
catch(const PT::ptree_error& ex)
{
    const std::string msg(
        std::string("Couldn't parse config. Reason: ") +
        ex.what());
    throw config_error(
        boost::str(
            boost::format("Couldn't parse config '%1%'. Reason: %2%") %
            name %
            ex.what()));
}

config_source::config_source(const boost::shared_ptr<impl>& impl): impl_(impl) {}


config_source::~config_source() {}

config_source config_source::create_from_file(
    const std::string& filename, input_format format, file_name_style fname_style) try
{
    const boost::shared_ptr<impl> impl(new class impl(filename, format, fname_style));
    return config_source(impl);
}
catch(const PT::ptree_error& ex)
{
    throw config_error(
        boost::str(
            boost::format(
                "Couldn't parse config '%1%'. Reason: %2%") %
                filename %
                ex.what()));
}

std::string config_source::to_string(output_type type) const try
{
    return impl_->to_string(pretty == type);
}
catch(const PT::ptree_error& ex)
{
    throw config_source(
        boost::str(
            boost::format(
                "Couldn't stringify config '%1%'. Reason: %2%") %
                impl_->name() %
                ex.what()));
}

}//namespace jet
