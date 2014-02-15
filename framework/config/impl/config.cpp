// jet.config library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "config.hpp"
#include "config_source_impl.hpp"
#include "config_throw.hpp"
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <sstream>

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

inline std::string compose_name(
    const std::string& app_name,
    const std::string& instance_name = std::string{},
    const std::string& path = std::string{})
{
    std::string res{app_name};
    if(!instance_name.empty())
    {
        res += INSTANCE_DELIMITER;
        res += instance_name;
    }
    if(!path.empty())
    {
        res += NODE_DELIMITER;
        res += path;
    }
    return res;
}

inline std::string add_path(const std::string& lhs, const std::string& rhs)
{
    std::string res{lhs};
    if(!res.empty() && !rhs.empty())
        res += NODE_DELIMITER;
    res += rhs;
    return res;
}

}//anonymous namespace

const config_lock lock{};

class config_node::impl: boost::noncopyable
{
public:
    impl(const std::string& app_name, const std::string& instance_name):
        app_name_{app_name},
        instance_name_{instance_name},
        is_locked_{false},
        config_{}
    {
        if(app_name_.empty())
            JET_THROW_CFG() << "Empty config name";
        root_.push_back({ROOT_NODE_NAME, tree()});
        config_ = &root_.front().second;
        if(!instance_name_.empty())
            config_->push_back({instance_name_, tree{}});
        config_->push_back({app_name_, tree{}});
        config_->push_back({DEFAULT_NODE_NAME, tree{}});
    }
    const std::string& app_name() const { return app_name_; }
    const std::string& instance_name() const { return instance_name_; }
    void merge(const config_source::impl& source)
    {
        if(is_locked_)
            JET_THROW_CFG() << "config '" << name() << "' is locked";
        const tree& other_config(source.get_root().front().second);
        
        {//...merge default attributes (if found)
            const cassoc_tree_iter default_iter{ other_config.find(DEFAULT_NODE_NAME) };
            if(other_config.not_found() != default_iter)
                merge_processor(name(), source.name()).merge(get_default_node(), default_iter->second);
        }
        const cassoc_tree_iter appIter { other_config.find(app_name()) };
        if(appIter == other_config.not_found())
            return;
        merge_processor(name(), source.name()).merge(get_self_node(), appIter->second);
        if(!instance_name().empty())
        {//...merge instance
            const cassoc_tree_iter instance_root_iter { appIter->second.find(INSTANCE_NODE_NAME) };
            if(appIter->second.not_found() != instance_root_iter)
            {
                const cassoc_tree_iter instance_iter{ instance_root_iter->second.find(instance_name()) };
                if(instance_root_iter->second.not_found() != instance_iter)
                {
                    merge_processor(name(), source.name()).merge(
                        getInstanceNode(), instance_iter->second);
                }
            }
        }
    }
    void lock()
    {
        if(is_locked_)
            return;
        //...merge self node and (optionally) instance node into default node
        merge_processor(ROOT_NODE_NAME NODE_DELIMITER DEFAULT_NODE_NAME, app_name()).merge(
            get_default_node(), get_self_node());
        if(!instance_name().empty())
            merge_processor(ROOT_NODE_NAME NODE_DELIMITER DEFAULT_NODE_NAME, name()).merge(
                get_default_node(), getInstanceNode());
        //...swap self node and default node to move result into self node
        get_default_node().swap(get_self_node());
        //...erase default and (optionally) instance node
        config_->erase(DEFAULT_NODE_NAME);
        config_->erase(instance_name());
        is_locked_ = true;
    }
    const tree& get_config_node() const
    {
        if(!is_locked_)
            JET_THROW_CFG() << "Initialization of config '" << name() << "' is not finished";
        return *config_;
    }
    void print(std::ostream& os) const
    {
        PT::write_xml(os, root_, PT::xml_writer_make_settings(' ', 2));
    }
    std::string name() const { return compose_name(app_name(), instance_name()); }
private:
    class merge_processor
    {
    public:
        merge_processor(const std::string& config_name, const std::string& source_name):
            config_name_(config_name),
            source_name_(source_name)
        {}
        void merge(tree& to, const tree& from) const
        {
            tree new_children{};
            for(const value_type& node: from)
            {
                const std::string& merge_name{node.first};
                if(INSTANCE_NODE_NAME == merge_name)
                    continue;
                {//...check for ambiguous merge
                    const size_t from_count { from.count(merge_name) };
                    const size_t to_count { to.count(merge_name) };
                    if( (from_count > 0 && to_count > 1) ||
                        (to_count > 0 && from_count > 1) )
                        JET_THROW_CFG()
                            << "Can't do ambiguous merge of node '" << merge_name
                            << "' from config source '" << source_name_
                            << "' to config '" << config_name_<< '\'';
                }
                const tree& merge_tree { node.second };
                const assoc_tree_iter iter { to.find(merge_name) };
                if(iter == to.not_found())
                {
                    new_children.push_back(node);
                }
                else if(merge_tree.empty())
                {
                    iter->second = merge_tree;
                }
                else
                {
                    merge(iter->second, merge_tree);
                }
            }
            for(value_type& node: new_children)
            {
                const std::string& name{node.first};
                to.push_back({name, tree{}});
                node.second.swap(to.back().second);
            }

        }
    private:
        const std::string config_name_;
        const std::string& source_name_;
    };
    tree& getInstanceNode()
    {
        assert(!is_locked_);
        assert(!instance_name().empty());
        assert(config_->size() == 3);
        tree& instance{config_->front().second};
        return instance;
    }
    tree& get_self_node()
    {
        assert(!is_locked_);
        if(instance_name().empty())
        {
            assert(config_->size() == 2);
            tree& self(config_->front().second);
            return self;
        }
        else
        {
            assert(config_->size() == 3);
            tree_iter iter{config_->begin()};
            ++iter;
            tree& self{iter->second};
            return self;
        }
    }
    tree& get_default_node()
    {
        assert(!is_locked_);
        tree& default_node{config_->back().second};
        return default_node;
    }
    //...
    const std::string app_name_, instance_name_;
    bool is_locked_;
    tree root_;
    tree* config_;
};

config_node::config_node(const std::string& app_name, const std::string& instance_name):
    impl_{std::make_shared<impl>(boost::trim_copy(app_name), boost::trim_copy(instance_name))},
    tree_node_{}
{}

config_node::config_node(
    const std::string& path,
    const std::shared_ptr<impl>& impl,
    const void* tree_node):
    path_{path},
    impl_{impl},
    tree_node_{tree_node}
{
}

config_node::config_node(const config_node& other):
    path_{other.path_},
    impl_{other.impl_},
    tree_node_{other.tree_node_}
{
}

config_node& config_node::operator=(const config_node& other)
{
    if(&other == this)
        return *this;
    impl_ = other.impl_;
    tree_node_ = other.tree_node_;
    return *this;
}

config_node::~config_node() {}

std::string config_node::name() const { return compose_name(app_name(), instance_name(), path()); }

const std::string& config_node::app_name() const { return impl_->app_name(); }

const std::string& config_node::instance_name() const { return impl_->instance_name(); }

void config_node::merge(const config_source& source)
{
    impl_->merge(*source.impl_);
}

void config_node::lock()
{
    impl_->lock();
    tree_node_ = static_cast<const tree*>(&(impl_->get_config_node().front().second));
}

void config_node::print(std::ostream& os) const
{
    if(tree_node_)
    {
        os << '<' << name() << '>';
        if(!static_cast<const tree*>(tree_node_)->empty())
            os << '\n';
        {
            std::stringstream strm;
            PT::write_xml(
                strm,
                *static_cast<const tree*>(tree_node_),
                PT::xml_writer_make_settings(' ', 2));
            std::string firstString;
            std::getline(strm, firstString);
            if(firstString.size() > 2 && //...get rid of the first line with <?xml ...?>
                '<' == firstString[0] && '?' == firstString[1])
                os << strm.str().substr(firstString.size() + 1);
            else
                os << strm.str();
        }
        os << "</" << name() << ">\n";
    }
    else
        impl_->print(os);
}


std::string config_node::get(const std::string& raw_attr_name) const
{
    impl_->get_config_node();//...just to check locked state
    const std::string attr_name{boost::trim_copy(raw_attr_name)};
    const boost::optional<const tree&> attr_node{
        static_cast<const tree*>(tree_node_)->get_child_optional(attr_name)};
    if(!attr_node)
        JET_THROW_CFG()
            << "Can't find property '" << attr_name
            << "' in config '" << name() << '\'';
    if(attr_node->empty())
        return attr_node->data();
    JET_THROW_CFG()
        << "Node '" << add_path(name(), attr_name) << "' is intermidiate node without value";
}

boost::optional<std::string> config_node::get_optional(const std::string& attr_name) const
{
    impl_->get_config_node();//...just to check locked state
    const boost::optional<const tree&> attr_node{
        static_cast<const tree*>(tree_node_)->get_child_optional(boost::trim_copy(attr_name))};

    if(attr_node && attr_node->empty())
        return attr_node->data();

    return boost::none;
}

std::string config_node::get(const std::string& attr_name, const std::string& default_value) const
{
    const boost::optional<std::string> value{get_optional(attr_name)};
    if(value)
        return *value;
    return default_value;
}

config_node config_node::get_node(const std::string& path) const
{
    boost::optional<config_node> optChild{get_node_optional(path)};
    if(optChild)
        return *optChild;
    JET_THROW_CFG() << "config '" << name() << "' doesn't have child '" << path << '\'';
}

boost::optional<config_node> config_node::get_node_optional(const std::string& raw_path) const
{
    impl_->get_config_node();//...just to check locked state
    const std::string path{boost::trim_copy(raw_path)};
    const boost::optional<const tree&> node{
        static_cast<const tree*>(tree_node_)->get_child_optional(path)};
    if(node)
    {
        return config_node{
            add_path(path_, path),
            impl_,
            &(*node)};
    }
    return boost::none;
}

namespace {
inline std::pair<std::string, std::string> cutoff_last_node(const std::string& path)
{
    const size_t pos { path.find_last_of(NODE_DELIMITER) };
    if(pos == std::string::npos)
        return {std::string{}, path};
    std::pair<std::string, std::string> res;
    res.first = path.substr(0, pos);
    res.second = path.substr(pos + sizeof(NODE_DELIMITER) - 1);
    return res;
}
}//anonymous namespace

const std::string config_node::node_name() const
{
    return cutoff_last_node(path()).second;
}

std::vector<config_node> config_node::get_children_of(const std::string& raw_parent_path) const
{
    impl_->get_config_node();//...just to check locked state
    
    const std::string parent_path{boost::trim_copy(raw_parent_path)};

    const std::string full_parent_path(add_path(path_, parent_path));

    const tree& parent_node = static_cast<const tree*>(tree_node_)->get_child(parent_path);
    std::vector<config_node> result;
    for(const auto& node: parent_node)
    {
        const std::string new_path{add_path(full_parent_path, node.first)};
        result.push_back(
            config_node{
                new_path,
                impl_,
                &node.second});
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const config_node& config)
{
    config.print(os);
    return os;
}

config& config::operator<<(const config_source& source)
{
    merge(source);
    return *this;
}

void config::operator<<(config_lock)
{
    lock();
}

void config_node::throw_value_conversion_error(const std::string& attr_name, const std::string& value) const
{
    JET_THROW_CFG()
        << "Can't convert value '" << value
        << "' of a property '" << attr_name
        << "' in config '" << name() << '\'';
}

config::config(const std::string& app_name, const std::string& instance_name):
    config_node(app_name, instance_name)
{
}

} //namespace jet
