//  Copyright Alexey Tkachneko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "gtest.hpp"
#include "config/config.hpp"
#include "config/config_error.hpp"
#include <boost/foreach.hpp>
#include <iostream>

using std::cout;
using std::endl;


TEST(config_source, simple_config_source)
{
    const jet::config_source source("<app> <attr> value</attr></app> ");
    EXPECT_EQ(
        "<config><app><attr>value</attr></app></config>",
        source.to_string(jet::config_source::one_line));
}

TEST(config_source, empty_config_source)
{
    {
        const jet::config_source source("<config></config> ");
        EXPECT_EQ(
            "<config/>\n",
            source.to_string());
    }
    CONFIG_ERROR(jet::config_source("  "),
        "Couldn't parse config 'unknown'. Reason:");
}

TEST(config_source, invalid_config_source)
{
    CONFIG_ERROR(jet::config_source("  invalid ", "config.xml"),
        "Couldn't parse config 'config.xml'. Reason:");
}

TEST(config_source, simple_config_source_pretty_print)
{
    const jet::config_source source(" <app><attr>value  </attr></app>");
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <attr>value</attr>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, attr_config_source)
{
    const jet::config_source source("<app attr='value'/> ");
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <attr>value</attr>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, normalize_colon_config_source)
{
    const jet::config_source source("<config><app..i1 attr='value'/></config>");
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <instance>\n"
        "      <i1>\n"
        "        <attr>value</attr>\n"
        "      </i1>\n"
        "    </instance>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, error_colon_config_source)
{
    CONFIG_ERROR(
        jet::config_source("<config><app.. attr='value'/></config>", "s1.xml"),
        "Invalid '..' in element 'app..' in config source 's1.xml'. Expected format 'app_name..instance_name'");
    CONFIG_ERROR(
        jet::config_source("<config><..i1 attr='value'/></config>", "s1.xml"),
        "Invalid '..' in element '..i1' in config source 's1.xml'. Expected format 'app_name..instance_name'");
}

TEST(config_source, combine_abbriviated_instance_config_source1)
{
    const jet::config_source source(
        "<config>"
        "   <app..i1 attr='value'/>"
        "   <app..i2 attr2='value2'/>"
        "</config>");
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <instance>\n"
        "      <i1>\n"
        "        <attr>value</attr>\n"
        "      </i1>\n"
        "      <i2>\n"
        "        <attr2>value2</attr2>\n"
        "      </i2>\n"
        "    </instance>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, combine_abbriviated_instance_config_source2)
{
    const jet::config_source source(
        "<config>"
        "   <app attr='value' attr1='value1'/>"
        "   <app..i2 attr1='value11' attr2='value2'/>"
        "</config>");
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <attr>value</attr>\n"
        "    <attr1>value1</attr1>\n"
        "    <instance>\n"
        "      <i2>\n"
        "        <attr1>value11</attr1>\n"
        "        <attr2>value2</attr2>\n"
        "      </i2>\n"
        "    </instance>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, complex_config_source)
{
    const jet::config_source source("<app attr1='value1' attr2=\"value2\"><attr3 attr4='value4'><attr5>value5</attr5></attr3></app>");
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <attr1>value1</attr1>\n"
        "    <attr2>value2</attr2>\n"
        "    <attr3>\n"
        "      <attr4>value4</attr4>\n"
        "      <attr5>value5</attr5>\n"
        "    </attr3>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, naive_config_source)
{
    const jet::config_source source(
        jet::config_source::create_naive(
            "attr1=value1:attr2=value2:attr3.attr4=value4:attr3.attr5=value5", "app"));
    EXPECT_EQ("attr1=value1:attr2=value2:attr3.attr4=value4:attr3.attr5=value5", source.name());
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <attr1>value1</attr1>\n"
        "    <attr2>value2</attr2>\n"
        "    <attr3>\n"
        "      <attr4>value4</attr4>\n"
        "      <attr5>value5</attr5>\n"
        "    </attr3>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, naive_config_source_instance)
{
    const jet::config_source source(
        jet::config_source::create_naive(
            "attr1=value1", "app.exe", "i1"));
    EXPECT_EQ(
        "<config>\n"
        "  <app.exe>\n"
        "    <instance>\n"
        "      <i1>\n"
        "        <attr1>value1</attr1>\n"
        "      </i1>\n"
        "    </instance>\n"
        "  </app.exe>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, invalid_naive_instance_config)
{
    CONFIG_ERROR(
        jet::config_source::create_naive("attr1", "app"),
        "Invalid property 'attr1' in config source 'attr1'");
    CONFIG_ERROR(
        jet::config_source::create_naive("attr1=", "app"),
        "Invalid property 'attr1=' in config source 'attr1='");
    CONFIG_ERROR(
        jet::config_source::create_naive("=value", "app"),
        "Invalid property '=value' in config source '=value'");
    CONFIG_ERROR(
        jet::config_source::create_naive("=", "app"),
        "Invalid property '=' in config source '='");
    CONFIG_ERROR(
        jet::config_source::create_naive("==", "app"),
        "Invalid property '==' in config source '=='");
    CONFIG_ERROR(
        jet::config_source::create_naive("1=2=3", "app"),
        "Invalid property '1=2=3' in config source '1=2=3'");
    CONFIG_ERROR(
        jet::config_source::create_naive("range=2:3", "app"),
        "Invalid property '3' in config source 'range=2:3'");
    CONFIG_ERROR(
        jet::config_source::create_naive("attr=value", ""),
        "Empty application name. Couldn't create configuration from 'attr=value'");
}

TEST(config_source, system_config_invalid_data)
{
    CONFIG_ERROR(
        jet::config_source("<config>12345678901</config>", "s1.xml"),
        "Invalid data node '1234567...' under 'config' node in config source 's1.xml'");
}

TEST(config_source, app_config_invalid_data)
{
    CONFIG_ERROR(
        jet::config_source("<config><app_name>data</app_name></config>", "s1.xml"),
        "Invalid data node 'data' under 'app_name' node in config source 's1.xml'");
}

TEST(config_source, default_config_invalid_data)
{
    CONFIG_ERROR(
        jet::config_source("<config><default>data123</default></config>", "s1.xml"),
        "Invalid data node 'data123' under 'default' node in config source 's1.xml'");
}

TEST(config_source, instance_config_invalid_data)
{
    CONFIG_ERROR(
        jet::config_source("<config><app_name..i1>data</app_name..i1></config>", "s1.xml"),
        "Invalid data node 'data' under 'app_name..i1' node in config source 's1.xml'");
}

TEST(config_source, inconsistent_attribute_definition_config_source)
{
    CONFIG_ERROR(
        jet::config_source(
            "<app>"
            "   <env attr='value1'>"
            "       data"
            "   </env>"
            "</app>",
            "InconsistentAttributeDefinitionConfigSource"),
        "Invalid element 'config.app.env' in config source 'InconsistentAttributeDefinitionConfigSource' contains both value and child attributes");
}

TEST(config_source, duplicate_attr_config_source)
{//TODO: submit bugreport to boost comunity - two attributes with the same name is not well formed xml
    CONFIG_ERROR(jet::config_source("<config attr='value1' attr='value2'></config>"),
        "Duplicate definition of attribute 'config.attr' in config 'unknown'");
}

TEST(config_source, invalid_default_node_with_instance)
{
    CONFIG_ERROR(
        jet::config_source("<config><default..i1><env PATH='/usr/bin'/></default></config..i1>", "s1.xml"),
        "Default node 'default..i1' can't have instance. Found in config source 's1.xml'");
}


TEST(config_source, invalid_duplicates)
{
    CONFIG_ERROR(
        jet::config_source(
            "<config>"
            "   <default><lib attr='value'/></default>"
            "   <default><env PATH='/usr/bin'/></default>"
            "</config>",
            "s1.xml"),
        "Duplicate default node in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<config>"
            "   <default>"
            "       <lib attr='value'/>"
            "       <lib attr2='value2'/>"
            "   </default>"
            "</config>",
            "s1.xml"),
        "Duplicate default node 'lib' in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "   <default>"
            "       <lib attr='value'/>"
            "       <lib attr2='value2'/>"
            "   </default>",
            "s1.xml"),
        "Duplicate default node 'lib' in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<config>"
            "   <app_name attr='value'/>"
            "   <app_name><env PATH='/usr/bin'/></app_name>"
            "</config>", "s1.xml"),
        "Duplicate node 'app_name' in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<config>\n"
            "   <app_name..i1><env PATH='/usr/bin'/></app_name..i1>\n"
            "   <app_name..i1><env PATH='/usr../usr/bin'/></app_name..i1>\n"
            "</config>\n",
            "s1.xml"),
        "Duplicate node 'app_name..i1' in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<config>\n"
            "   <app_name><instance><i1 attr='value'/></instance></app_name>\n"
            "   <app_name..i1><env PATH='/usr/bin'/></app_name..i1>\n"
            "</config>\n",
            "s1.xml"),
        "Duplicate node 'app_name..i1' in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<config>\n"
            "   <app_name>\n"
            "       <instance><i1 attr='value'/></instance>\n"
            "       <instance><i2 attr='value'/></instance>\n"
            "   </app_name>\n"
            "</config>\n",
            "s1.xml"),
        "Duplicate instance node under 'app_name' node in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<app_name>\n"
            "   <instance><i1 attr='value'/></instance>\n"
            "   <instance><i2 attr='value'/></instance>\n"
            "</app_name>\n",
            "s1.xml"),
        "Duplicate instance node under 'app_name' node in config source 's1.xml'");
    CONFIG_ERROR(
        jet::config_source(
            "<config>\n"
            "   <app_name>"
            "       <instance>"
            "           <i1 attr='value'/>"
            "           <i1 attr2='value2'/>"
            "       </instance>"
            "   </app_name>\n"
            "</config>\n",
            "s1.xml"),
        "Duplicate node 'app_name..i1' in config source 's1.xml'");
}

TEST(config_source, normalize_keywords)
{
    EXPECT_EQ(jet::config_source("<config/>").to_string(), "<config/>\n");
    EXPECT_EQ(
        jet::config_source("<Default/>").to_string(jet::config_source::one_line),
        "<config><default/></config>");
    EXPECT_EQ(
        jet::config_source(
            "<config><Default/></config>").to_string(jet::config_source::one_line),
        "<config><default/></config>");
    EXPECT_EQ(
        jet::config_source(
            "<app><Instance><i1/></Instance></app>"
            ).to_string(jet::config_source::one_line),
        "<config><app><instance><i1/></instance></app></config>");
    EXPECT_EQ(
        jet::config_source(
            "<config><app><Instance><i1/></Instance></app></config>"
            ).to_string(jet::config_source::one_line),
        "<config><app><instance><i1/></instance></app></config>");
    EXPECT_EQ(
        jet::config_source("<APP/>").to_string(jet::config_source::one_line),
        "<config><APP/></config>");
    EXPECT_EQ(//...this is for Windows file name convention
        jet::config_source(
            "<APP/>",
            "s1.xml",
            jet::config_source::xml,
            jet::config_source::case_insensitive
            ).to_string(jet::config_source::one_line),
        "<config><app/></config>");
}

TEST(config_source, prohibited_simple_default_attributes)
{//...this is to prevent situation when default attributes are treated as default values for simple attributes in application config
    CONFIG_ERROR(
        jet::config_source(
            "<default attr1='value1'>\n"
            "   <attr2>value2</attr2>\n"
            "</default>",
            "s1.xml"),
        "config source 's1.xml' is invalid: 'default' node can not contain direct properties. See 'default.attr1' property");
}

TEST(config_source, prohibited_instance_subsection_in_default_section)
{//...not allowed because 'instance' is keyword
    CONFIG_ERROR(
        jet::config_source(
            "<default>\n"
            "   <Instance attr='value'/>\n"
            "</default>",
            "s1.xml"),
        "config source 's1.xml' is invalid: 'default' node can not contain 'Instance' node");
}

TEST(config, default_attr_config_without_root_config_element)
{
    const jet::config_source default_source(
        "<default>\n"
        "   <libName strAttr='value' intAttr='12'>\n"
        "       <doubleAttr>13.2</doubleAttr>\n"
        "       <subKey attr='value'/>\n"
        "   </libName>\n"
        "</default>\n",
        "default.xml");
    jet::config config("app_name");
    config << default_source << jet::lock;
    EXPECT_EQ(std::string("app_name"), config.name());
    EXPECT_EQ("value",   config.get("libName.strAttr "));
    EXPECT_EQ("value",   config.get<std::string>(" libName.strAttr"));
    EXPECT_EQ("12",      config.get("libName.intAttr"));
    EXPECT_EQ(12,        config.get<int>("libName.intAttr "));
    EXPECT_EQ("13.2",    config.get<std::string>(" libName.doubleAttr"));
    EXPECT_EQ(13.2,      config.get<double>("  libName.doubleAttr"));
    EXPECT_EQ("value",   config.get("libName.subKey.attr"));
}

TEST(config, attrib_config_source_merge)
{
    const jet::config_source s1("<app_name attr1='10' attr2='something'></app_name>");
    const jet::config_source s2("<app_name attr2='20' attr3='value3'></app_name>");
    jet::config config("app_name");
    config << s1 << s2 << jet::lock;
    EXPECT_EQ("app_name", config.name());
    EXPECT_EQ(10,        config.get<int>("attr1"));
    EXPECT_EQ(20,        config.get<int>("attr2"));
    EXPECT_EQ("value3",  config.get("attr3"));
}

TEST(config, subtree_config_source_merge)
{
    const jet::config_source s1(
        "<app_name>\n"
        "  <attribs attr1='10' attr2='something'></attribs>\n"
        "</app_name>\n",
        "s1");
    const jet::config_source s2(
        "<app_name>\n"
        "  <attribs attr2='20' attr3='value3'>\n"
        "    <subkey attr='data'></subkey>\n"
        "  </attribs>\n"
        "</app_name>\n",
        "s2");
    jet::config config("app_name");
    config << s1 << s2 << jet::lock;
    EXPECT_EQ("app_name", config.name());
    EXPECT_EQ(10,        config.get<int>("attribs.attr1"));
    EXPECT_EQ(20,        config.get<int>("attribs.attr2"));
    EXPECT_EQ("value3",  config.get("attribs.attr3"));
    EXPECT_EQ("data",    config.get("attribs.subkey.attr"));
}

TEST(config, any_case_system_config_name)
{
    const jet::config_source s1("<conFIG><app_name attr='10'></app_name></conFIG>", "s1.xml");
    jet::config config("app_name");
    config << s1 << jet::lock;
    EXPECT_EQ(10, config.get<int>("attr"));
}

TEST(config, instance_config_name)
{
    jet::config config("app_name ", "i1 ");
    EXPECT_EQ("app_name..i1", config.name());
    EXPECT_EQ("app_name", config.app_name());
    EXPECT_EQ("i1", config.instance_name());
}

TEST(config, lock_config)
{
    const jet::config_source s1(
        "<app_name..1 attr='value'/>",
        "s1.xml");
    const jet::config_source s2(
        "<app_name..1 attr2='10'/>",
        "s2.xml");
    jet::config config("app_name ", " 1");
    config << s1 << jet::lock;
    EXPECT_EQ("value", config.get("attr"));
    CONFIG_ERROR(
        config << s2,
        "config 'app_name..1' is locked");
}

TEST(config, getters)
{
    const jet::config_source s1(
        "<app..1 str='value' int='10'/>",
        "s1.xml");

    jet::config config("app", "1");
    config << s1 << jet::lock;
    
    //...simple getter
    EXPECT_EQ("value", config.get("str"));
    EXPECT_EQ("value", config.get_node("str").get());
    EXPECT_EQ(10, config.get<int>("int"));
    CONFIG_ERROR(
        config.get("unknown"),
        "Can't find property 'unknown' in config 'app..1'");
    CONFIG_ERROR(
        config.get<int>("str"),
        "Can't convert value 'value' of a property 'str' in config 'app..1'");
    
    //...optional getter
    EXPECT_EQ("value", *config.get_optional("str"));
    EXPECT_EQ("value", *config.get_node_optional("str")->get_optional());
    EXPECT_EQ(10, *config.get_optional<int>("int"));
    EXPECT_EQ(boost::none, config.get_optional("unknown"));
    CONFIG_ERROR(
        config.get_optional<int>("str"),
        "Can't convert value 'value' of a property 'str' in config 'app..1'");
    
    //...default getter
    EXPECT_EQ("value", config.get("str", "anotherValue"));
    EXPECT_EQ(10, config.get("int", 12));
    EXPECT_EQ("another value", config.get("unknown", "another value"));
    CONFIG_ERROR(
        config.get("str", 12),
        "Can't convert value 'value' of a property 'str' in config 'app..1'");
    
}

TEST(config, initialization)
{
    const jet::config_source s1(
        "<app str='value1' int='10'/><app..1><lib attr1='0s1'/></app..1><app2 attr='value'/><default><lib attr1='1s1' attr2='2s1'/></default>",
        "s1.xml");
    const jet::config_source s2(
        "<config><app..1 str='value2' float='10.'/><default><lib attr1='1s2'/></default></config>",
        "s2.xml");

    jet::config config("app", "1");
    config << s1 << s2;
    
    {//...check config before locking it
        const char* unlocked_config =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<config>\n\
  <1>\n\
    <lib>\n\
      <attr1>0s1</attr1>\n\
    </lib>\n\
    <str>value2</str>\n\
    <float>10.</float>\n\
  </1>\n\
  <app>\n\
    <str>value1</str>\n\
    <int>10</int>\n\
  </app>\n\
  <default>\n\
    <lib>\n\
      <attr1>1s2</attr1>\n\
      <attr2>2s1</attr2>\n\
    </lib>\n\
  </default>\n\
</config>\n";

        std::stringstream strm;
        strm << config;
        EXPECT_EQ(unlocked_config, strm.str());
        CONFIG_ERROR(config.get("str"), "Initialization of config 'app..1' is not finished");
    }
    
    config << jet::lock;
    
    {//...and after
        const char* locked_config =
"<app..1>\n\
<lib>\n\
  <attr1>0s1</attr1>\n\
  <attr2>2s1</attr2>\n\
</lib>\n\
<str>value2</str>\n\
<int>10</int>\n\
<float>10.</float>\n\
</app..1>\n";
        std::stringstream strm;
        strm << config;
        EXPECT_EQ(locked_config, strm.str());
    }
}

TEST(config, get_node)
{
    const jet::config_source s1(
        "<app_name>\n"
        "  <attribs attr1='10' attr2='something'></attribs>\n"
        "</app_name>\n",
        "s1");
    const jet::config_source s2(
        "<app_name..1>\n"
        "  <attribs attr2='20' attr3='value3'>\n"
        "    <subkey attr='data'></subkey>\n"
        "  </attribs>\n"
        "</app_name..1>\n",
        "s2");
    jet::config config("app_name", "1");
    config << s1 << s2 << jet::lock;
    const jet::config_node attribs(config.get_node("attribs"));
    EXPECT_EQ("app_name..1.attribs", attribs.name());
    EXPECT_EQ(10,        attribs.get<int>("attr1"));
    EXPECT_EQ(20,        attribs.get<int>("attr2"));
    EXPECT_EQ("value3",  attribs.get("attr3"));
    EXPECT_EQ("data",    attribs.get("subkey.attr"));
    
    const jet::config_node subkey(attribs.get_node("subkey"));
    EXPECT_EQ("app_name..1.attribs.subkey", subkey.name());
    EXPECT_EQ("data", subkey.get("attr"));
    
    CONFIG_ERROR(attribs.get_node("UNKNOWN"), "config 'app_name..1.attribs' doesn't have child 'UNKNOWN'");
    
}

TEST(config, get_children_of)
{
    const jet::config_source s1(
"<deployment>\n\
    <regions>\n\
        <UK>\n\
            <boxes>\n\
                <box hostname='UK1'/>\n\
                <box hostname='UK2'/>\n\
                <box hostname='UK3'/>\n\
            </boxes>\n\
        </UK>\n\
        <US>\n\
            <boxes>\n\
                <box hostname='US1'/>\n\
                <box hostname='US2'/>\n\
            </boxes>\n\
        </US>\n\
    </regions>\n\
</deployment>\n",
        "s1");
    jet::config deployment("deployment");
    deployment << s1 << jet::lock;
    const std::vector<jet::config_node> regions(deployment.get_children_of("regions"));
    EXPECT_EQ(2U, regions.size());
    BOOST_FOREACH(const jet::config_node& region, regions)
    {
        const std::string& region_name = region.node_name();
        const std::vector<jet::config_node> boxes(region.get_children_of("boxes"));
        if("US" == region_name)
            EXPECT_EQ(2U, boxes.size());
        else
            EXPECT_EQ(3U, boxes.size());
        unsigned index = 1;
        BOOST_FOREACH(const jet::config_node& box, boxes)
        {
            const std::string expected_hostname(region_name + boost::lexical_cast<std::string>(index));
            EXPECT_EQ(expected_hostname, box.get<std::string>("hostname"));
            const std::string expected_box_name("deployment.regions." + region_name + ".boxes.box");
            EXPECT_EQ(expected_box_name, box.name());
            ++index;
        }
    }
}

TEST(config, get_children_of_root)
{
    const jet::config_source s1(
"<deployment>\n\
    <UK attr='1'/>\n\
    <US attr='2'/>\n\
    <HK attr='23'/>\n\
</deployment>\n",
        "s1");
    jet::config deployment("deployment");
    deployment << s1 << jet::lock;
    
    jet::config_nodes nodes(deployment.get_children_of());
    ASSERT_EQ(3U, nodes.size());
    EXPECT_EQ("UK", nodes[0].node_name());
    EXPECT_EQ("US", nodes[1].node_name());
    EXPECT_EQ("HK", nodes[2].node_name());
}

TEST(config, erase_xml_comments)
{
    const jet::config_source s1(
"<deployment>\n\
    <!--UK attr='1'/-->\n\
    <US attr='2'/>\n\
    <HK attr='23'/>\n\
</deployment>\n",
        "s1");
    jet::config deployment("deployment");
    deployment << s1 << jet::lock;
    
    jet::config_nodes nodes(deployment.get_children_of());
    ASSERT_EQ(2U, nodes.size());
    EXPECT_EQ("US", nodes[0].node_name());
    EXPECT_EQ("HK", nodes[1].node_name());
}

TEST(config, get_value_of_intermidiate_node)
{
    const jet::config_source s1(
"<deployment>\n\
    <UK attr='1'/>\n\
    <US attr='2'/>\n\
    <HK attr='23'/>\n\
</deployment>\n",
        "s1");
    jet::config deployment("deployment");
    deployment << s1 << jet::lock;
    
    EXPECT_EQ("1", deployment.get_node("UK.attr").get());
    
    CONFIG_ERROR(deployment.get_node("UK").get(), "Node 'deployment.UK' is intermidiate node without value");
}

TEST(config, repeating_node_merge_without_conflicts)
{
    const jet::config_source s1(
"<app>\n\
    <key attr='1'/>\n\
    <key attr='2'/>\n\
</app>\n",
        "s1");
    
    const jet::config_source s2(
"<app>\n\
    <key1 attr='1'/>\n\
    <key1 attr='2'/>\n\
</app>\n",
        "s2");
    
    jet::config config("app");
    
    config << s1 << s2 << jet::lock;
    
    std::stringstream strm;
    strm << config;

    const char* merged_config =
"<app>\n\
<key>\n\
  <attr>1</attr>\n\
</key>\n\
<key>\n\
  <attr>2</attr>\n\
</key>\n\
<key1>\n\
  <attr>1</attr>\n\
</key1>\n\
<key1>\n\
  <attr>2</attr>\n\
</key1>\n\
</app>\n";

    EXPECT_EQ(merged_config, strm.str());
}

TEST(config, repeating_node_merge_with_conflicts1)
{
    const jet::config_source s1(
"<app>\n\
    <key attr='1'/>\n\
</app>\n",
        "s1");
    
    const jet::config_source s2(
"<app>\n\
    <key attr='1'/>\n\
    <key attr='2'/>\n\
</app>\n",
        "s2");
    
    jet::config config("app");
    
    config << s1;
    CONFIG_ERROR(config << s2, "Can't do ambiguous merge of node 'key' from config source 's2' to config 'app'");
}

TEST(config, repeating_node_merge_with_conflicts2)
{
    const jet::config_source s1(
"<app>\n\
    <key attr='1'/>\n\
    <key attr='2'/>\n\
</app>\n",
        "s1");
    
    const jet::config_source s2(
"<app>\n\
    <key attr='1'/>\n\
</app>\n",
        "s2");
    
    jet::config config("app");
    
    config << s1;
    CONFIG_ERROR(config << s2, "Can't do ambiguous merge of node 'key' from config source 's2' to config 'app'");
}

//TODO: introduce proper boolean property
//TODO: introduce "sequence of properties/elements" for repeating data
//TODO: (SourceConfig) prohibit '.' separator everywhere except application name
//TODO: add environment config source
//TODO: Is there any good alternative to '..' instance shortcut separator?
/*
    Currently JetConfig support merge of different config source with some inconvenient limitations.
    TODO list which addresses this issue:
1. Default handling
Default shared configuration is under <default>:
<default>
</default>

All other 'shared' configuration require explicit inclusion into target configuration:
<shared>
    <rfa host='10.10.10.30' port='23456' dacs_id='dev1'/>
    <db>
        <connection1 host='10.10.10.31' port='56789' user='system'/>
        <connection2 host='10.10.10.32' port='56789' user='system'/>
    </db>
    <display mode='vga'/>
</shared>
<myApp>
    <merge:shared.rfa/>
</myApp>

Some parameters of merged configuration can be overriden
<myApp>
    <merge:shared.rfa port='34567'></merge:shared.rfa>
</myApp>

Aggregate systax can be used to merge several items from shared config
<myApp>
    <merge:shared>
        <rfa port='34567'/>
        <display/>
    </merge:shared>
</myApp>

One line <merge:shared...> allows to include subelements:
<myApp>
    <merge:shared.db.connecton2/>
</myApp>

Both <shared> and <default> may cantain only unique elements of the first level (because otherwise we would have merge problems)

2. Merge modes: shared, override, add
<merge:shared> is already explained above.

Everything under this tag will override elements with the same name during merge
<merge:override>
	...
</merge:override>

or

<merge:override.element_name attr='value' attr2='value2'/>

Add sequence of elements:
<merge:add>
	...
</merge:add>

or

<merge:add.element_name attr='value' attr2='value2'/>
*/
