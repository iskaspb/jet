//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "gtest.hpp"
#include "config/config.hpp"
#include "config/config_error.hpp"
#include <iostream>

#define EXPECT_CONFIG_ERROR(EXPRESSION, MATCHER) \
    EXPECT_ERROR_EX(EXPRESSION, ::jet::config_error, MATCHER)

using std::cout;
using std::endl;

using jet::test::equal;
using jet::test::start_with;
using jet::config_source;
using jet::config;

TEST(config_source, simple_config_source)
{
    const config_source source{config_source::from_string{"<app> <attr> value</attr></app> "}};
    EXPECT_EQ(
        "<config><app><attr>value</attr></app></config>",
        source.to_string(config_source::one_line));
}

TEST(config_source, empty_config_source)
{
    {
        const config_source source{config_source::from_string{"<config></config> "}};
        EXPECT_EQ(
            "<config/>\n",
            source.to_string());
    }

    EXPECT_CONFIG_ERROR(
        config_source::from_string{"  "}.create(),
        equal
            ("Couldn't parse config 'unknown'")
            ("Config source 'unknown' is empty"));
}

TEST(config_source, invalid_config_source)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"  invalid "}.name("config.xml").create(),
        equal
            ("Couldn't parse config 'config.xml'")
            ("<unspecified file>(1): expected <"));
}

TEST(config_source, simple_config_source_pretty_print)
{
    const config_source source{config_source::from_string{" <app><attr>value  </attr></app>"}};
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
    const config_source source{config_source::from_string{"<app attr='value'/> "}};
    EXPECT_EQ(
        "<config>\n"
        "  <app>\n"
        "    <attr>value</attr>\n"
        "  </app>\n"
        "</config>\n",
        source.to_string());
}

TEST(config_source, normalize_instance_shortcut_config_source)
{
    const config_source source{config_source::from_string{"<config><app..i1 attr='value'/></config>"}};
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

TEST(config_source, error_instance_shortcut_config_source)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config><app.. attr='value'/></config>"}.name("s1.xml").create(),
        equal
            ("Couldn't parse config 's1.xml'")
            ("Invalid '..' in element 'app..' in config source 's1.xml'. Expected format 'app_name..instance_name'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config><..i1 attr='value'/></config>"}.name("s1.xml").create(),
        start_with
            ("Couldn't")
            ("Invalid '..' in element '..i1'"));
}

TEST(config_source, combine_abbriviated_instance_config_source1)
{
    const config_source source{config_source::from_string{
        "<config>"
        "   <app..i1 attr='value'/>"
        "   <app..i2 attr2='value2'/>"
        "</config>"}};
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
    const config_source source{config_source::from_string{
        "<config>"
        "   <app attr='value' attr1='value1'/>"
        "   <app..i2 attr1='value11' attr2='value2'/>"
        "</config>"}};
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
    const config_source source{config_source::from_string{"<app attr1='value1' attr2=\"value2\"><attr3 attr4='value4'><attr5>value5</attr5></attr3></app>"}};
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
    const config_source source{
        config_source::from_cmd_line{
            "attr1=value1:attr2=value2:attr3.attr4=value4:attr3.attr5=value5", "app"}};
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
    const config_source source{
        config_source::from_cmd_line{"attr1=value1", "app.exe"}
            .instance_name("i1")};
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
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("attr1", "app").create(),
        equal
            ("Couldn't create configuration from 'attr1'")
            ("Invalid property 'attr1' in config source 'attr1'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("attr1=", "app").create(),
        start_with
            ()
            ("Invalid property 'attr1=' in config source 'attr1='"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("=value", "app").create(),
        start_with
            ()
            ("Invalid property '=value' in config source '=value'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("=", "app").create(),
        start_with
            ()
            ("Invalid property '=' in config source '='"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("==", "app").create(),
        start_with
            ()
            ("Invalid property '==' in config source '=='"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("1=2=3", "app").create(),
        start_with
            ()
            ("Invalid property '1=2=3' in config source '1=2=3'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("range=2:3", "app").create(),
        start_with
            ()
            ("Invalid property '3' in config source 'range=2:3'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_cmd_line("attr=value", "").create(),
        equal
            ("Couldn't create configuration from 'attr=value'")
            ("Empty application name. Couldn't create configuration from 'attr=value'"));
}

TEST(config_source, system_config_invalid_data)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config>12345678901</config>"}.name("s1.xml").create(),
        start_with
            ()
            ("Invalid data node '1234567...' under 'config' node in config source 's1.xml'"));
}

TEST(config_source, app_config_invalid_data)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config><app_name>data</app_name></config>"}.name("s1.xml").create(),
        start_with
            ()
            ("Invalid data node 'data' under 'app_name' node in config source 's1.xml'"));
}

TEST(config_source, default_config_invalid_data)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config><default>data123</default></config>"}.name("s1.xml").create(),
        start_with
            ()
            ("Invalid data node 'data123' under 'default' node in config source 's1.xml'"));
}

TEST(config_source, instance_config_invalid_data)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config><app_name..i1>data</app_name..i1></config>"}
            .name("s1.xml").create(),
        start_with
            ()
            ("Invalid data node 'data' under 'app_name..i1' node in config source 's1.xml'"));
}

TEST(config_source, inconsistent_attribute_definition_config_source)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<app>"
            "   <env attr='value1'>"
            "       data"
            "   </env>"
            "</app>"}.name("InconsistentAttributeDefinitionConfigSource").create(),
        start_with
            ()
            ("Invalid element 'config.app.env' in config source 'InconsistentAttributeDefinitionConfigSource' contains both value and child attributes"));
}

TEST(config_source, duplicate_attr_config_source)
{//TODO: submit bugreport to boost comunity - two attributes with the same name is not well formed xml
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config attr='value1' attr='value2'></config>"}.create(),
        start_with
            ()
            ("Duplicate definition of attribute 'config.attr' in config 'unknown'"));
}

TEST(config_source, invalid_default_node_with_instance)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{"<config><default..i1><env PATH='/usr/bin'/></default></config..i1>"}
            .name("s1.xml").create(),
        start_with
            ()
            ("Default node 'default..i1' can't have instance. Found in config source 's1.xml'"));
}


TEST(config_source, invalid_duplicates)
{
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>"
            "   <default><lib attr='value'/></default>"
            "   <default><env PATH='/usr/bin'/></default>"
            "</config>"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate default node in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>"
            "   <default>"
            "       <lib attr='value'/>"
            "       <lib attr2='value2'/>"
            "   </default>"
            "</config>"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate default node 'lib' in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "   <default>"
            "       <lib attr='value'/>"
            "       <lib attr2='value2'/>"
            "   </default>"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate default node 'lib' in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>"
            "   <app_name attr='value'/>"
            "   <app_name><env PATH='/usr/bin'/></app_name>"
            "</config>"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate node 'app_name' in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>\n"
            "   <app_name..i1><env PATH='/usr/bin'/></app_name..i1>\n"
            "   <app_name..i1><env PATH='/usr../usr/bin'/></app_name..i1>\n"
            "</config>\n"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate node 'app_name..i1' in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>\n"
            "   <app_name><instance><i1 attr='value'/></instance></app_name>\n"
            "   <app_name..i1><env PATH='/usr/bin'/></app_name..i1>\n"
            "</config>\n"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate node 'app_name..i1' in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>\n"
            "   <app_name>\n"
            "       <instance><i1 attr='value'/></instance>\n"
            "       <instance><i2 attr='value'/></instance>\n"
            "   </app_name>\n"
            "</config>\n"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate instance node under 'app_name' node in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<app_name>\n"
            "   <instance><i1 attr='value'/></instance>\n"
            "   <instance><i2 attr='value'/></instance>\n"
            "</app_name>\n"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate instance node under 'app_name' node in config source 's1.xml'"));
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<config>\n"
            "   <app_name>"
            "       <instance>"
            "           <i1 attr='value'/>"
            "           <i1 attr2='value2'/>"
            "       </instance>"
            "   </app_name>\n"
            "</config>\n"}.name("s1.xml").create(),
        start_with
            ()
            ("Duplicate node 'app_name..i1' in config source 's1.xml'"));
}

TEST(config_source, normalize_keywords)
{
    EXPECT_EQ(config_source::from_string{"<config/>"}.create().to_string(), "<config/>\n");
    EXPECT_EQ(
        config_source::from_string{"<Default/>"}.create().to_string(config_source::one_line),
        "<config><default/></config>");
    EXPECT_EQ(
        config_source::from_string{
            "<config><Default/></config>"}.create().to_string(config_source::one_line),
        "<config><default/></config>");
    EXPECT_EQ(
        config_source::from_string{
            "<app><Instance><i1/></Instance></app>"
            }.create().to_string(config_source::one_line),
        "<config><app><instance><i1/></instance></app></config>");
    EXPECT_EQ(
        config_source::from_string{
            "<config><app><Instance><i1/></Instance></app></config>"
            }.create().to_string(config_source::one_line),
        "<config><app><instance><i1/></instance></app></config>");
    EXPECT_EQ(
        config_source::from_string{"<APP/>"}.create().to_string(config_source::one_line),
        "<config><APP/></config>");
    EXPECT_EQ(//...this is for Windows file name convention
        config_source::from_string{"<APP/>"}
            .name("s1.xml")
            .input_format(config_source::xml)
            .file_name_style(config_source::case_insensitive)
            .create().to_string(config_source::one_line),
        "<config><app/></config>");
}

TEST(config_source, prohibited_simple_default_attributes)
{//...this is to prevent situation when default attributes are treated as default values for simple attributes in application config
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<default attr1='value1'>\n"
            "   <attr2>value2</attr2>\n"
            "</default>"}.name("s1.xml").create(),
        start_with
            ()
            ("config source 's1.xml' is invalid: 'default' node can not contain direct properties. See 'default.attr1' property"));
}

TEST(config_source, prohibited_instance_subsection_in_default_section)
{//...not allowed because 'instance' is keyword
    EXPECT_CONFIG_ERROR(
        config_source::from_string{
            "<default>\n"
            "   <Instance attr='value'/>\n"
            "</default>"}.name("s1.xml").create(),
        start_with
            ()
            ("config source 's1.xml' is invalid: 'default' node can not contain 'Instance' node"));
}

TEST(config, default_attr_config_without_root_config_element)
{
    const config_source default_source{config_source::from_string{
        "<default>\n"
        "   <libName strAttr='value' intAttr='12'>\n"
        "       <doubleAttr>13.2</doubleAttr>\n"
        "       <subKey attr='value'/>\n"
        "   </libName>\n"
        "</default>\n"}.name("default.xml")};
    config config{"app_name"};
    config << default_source << jet::lock;
    EXPECT_EQ(std::string{"app_name"}, config.name());
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
    const config_source s1{config_source::from_string{
        "<app_name attr1='10' attr2='something'></app_name>"}};
    const config_source s2{config_source::from_string{"<app_name attr2='20' attr3='value3'></app_name>"}};
    config config{"app_name"};
    config << s1 << s2 << jet::lock;
    EXPECT_EQ("app_name", config.name());
    EXPECT_EQ(10,        config.get<int>("attr1"));
    EXPECT_EQ(20,        config.get<int>("attr2"));
    EXPECT_EQ("value3",  config.get("attr3"));
}

TEST(config, subtree_config_source_merge)
{
    const config_source s1{config_source::from_string{
        "<app_name>\n"
        "  <attribs attr1='10' attr2='something'></attribs>\n"
        "</app_name>\n"}.name("s1")};
    const config_source s2{config_source::from_string{
        "<app_name>\n"
        "  <attribs attr2='20' attr3='value3'>\n"
        "    <subkey attr='data'></subkey>\n"
        "  </attribs>\n"
        "</app_name>\n"}.name("s2")};
    config config("app_name");
    config << s1 << s2 << jet::lock;
    EXPECT_EQ("app_name", config.name());
    EXPECT_EQ(10,        config.get<int>("attribs.attr1"));
    EXPECT_EQ(20,        config.get<int>("attribs.attr2"));
    EXPECT_EQ("value3",  config.get("attribs.attr3"));
    EXPECT_EQ("data",    config.get("attribs.subkey.attr"));
}

TEST(config, any_case_system_config_name)
{
    const config_source s1{config_source::from_string{
        "<conFIG><app_name attr='10'></app_name></conFIG>"}.name("s1.xml")};
    config config{"app_name"};
    config << s1 << jet::lock;
    EXPECT_EQ(10, config.get<int>("attr"));
}

TEST(config, instance_config_name)
{
    config config{"app_name ", "i1 "};
    EXPECT_EQ("app_name..i1", config.name());
    EXPECT_EQ("app_name", config.app_name());
    EXPECT_EQ("i1", config.instance_name());
}

TEST(config, lock_config)
{
    const config_source s1{config_source::from_string{
        "<app_name..1 attr='value'/>"}.name("s1.xml")};
    const config_source s2{config_source::from_string{
        "<app_name..1 attr2='10'/>"}.name("s2.xml")};
    config config{"app_name ", " 1"};
    config << s1 << jet::lock;
    EXPECT_EQ("value", config.get("attr"));
    EXPECT_CONFIG_ERROR(
        config << s2,
        equal("config 'app_name..1' is locked"));
}

TEST(config, getters)
{
    const config_source s1{config_source::from_string{
        "<app..1 str='value' int='10'/>"}.name("s1.xml")};

    config config{"app", "1"};
    config << s1 << jet::lock;
    
    //...simple getter
    EXPECT_EQ("value", config.get("str"));
    EXPECT_EQ("value", config.get_node("str").get());
    EXPECT_EQ(10, config.get<int>("int"));
    EXPECT_CONFIG_ERROR(
        config.get("unknown"),
        equal("Can't find property 'unknown' in config 'app..1'"));

    EXPECT_CONFIG_ERROR(
        config.get<int>("str"),
        start_with
            ("Can't convert value 'value' of a property 'str' in config 'app..1'")
            ("bad lexical cast"));
    
    //...optional getter
    EXPECT_EQ("value", *config.get_optional("str"));
    EXPECT_EQ("value", *config.get_node_optional("str")->get_optional());
    EXPECT_EQ(10, *config.get_optional<int>("int"));
    EXPECT_EQ(boost::none, config.get_optional("unknown"));
    EXPECT_CONFIG_ERROR(
        config.get_optional<int>("str"),
        start_with
            ("Can't convert value 'value' of a property 'str' in config 'app..1'")
            ("bad lexical cast"));
    
    //...default getter
    EXPECT_EQ("value", config.get("str", "anotherValue"));
    EXPECT_EQ(10, config.get("int", 12));
    EXPECT_EQ("another value", config.get("unknown", "another value"));
    EXPECT_CONFIG_ERROR(
        config.get("str", 12),
        start_with
            ("Can't convert value 'value' of a property 'str' in config 'app..1'")
            ("bad lexical cast"));
    
}

TEST(config, initialization)
{
    const config_source s1{config_source::from_string{
        "<app str='value1' int='10'/><app..1><lib attr1='0s1'/></app..1><app2 attr='value'/><default><lib attr1='1s1' attr2='2s1'/></default>"}.name("s1.xml")};
    const config_source s2{config_source::from_string{
        "<config><app..1 str='value2' float='10.'/><default><lib attr1='1s2'/></default></config>"}
            .name("s2.xml")};

    config config{"app", "1"};
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
        EXPECT_CONFIG_ERROR(
            config.get("str"),
            equal("Initialization of config 'app..1' is not finished"));
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
    const config_source s1{config_source::from_string{
        "<app_name>\n"
        "  <attribs attr1='10' attr2='something'></attribs>\n"
        "</app_name>\n"}.name("s1")};
    const config_source s2{config_source::from_string{
        "<app_name..1>\n"
        "  <attribs attr2='20' attr3='value3'>\n"
        "    <subkey attr='data'></subkey>\n"
        "  </attribs>\n"
        "</app_name..1>\n"}.name("s2")};
    config config{"app_name", "1"};
    config << s1 << s2 << jet::lock;
    const jet::config_node attribs{config.get_node("attribs")};
    EXPECT_EQ("app_name..1.attribs", attribs.name());
    EXPECT_EQ(10,        attribs.get<int>("attr1"));
    EXPECT_EQ(20,        attribs.get<int>("attr2"));
    EXPECT_EQ("value3",  attribs.get("attr3"));
    EXPECT_EQ("data",    attribs.get("subkey.attr"));
    
    const jet::config_node subkey{attribs.get_node("subkey")};
    EXPECT_EQ("app_name..1.attribs.subkey", subkey.name());
    EXPECT_EQ("data", subkey.get("attr"));
    
    EXPECT_CONFIG_ERROR(
        attribs.get_node("UNKNOWN"),
        equal("config 'app_name..1.attribs' doesn't have child 'UNKNOWN'"));
    
}

TEST(config, get_children_of)
{
    const config_source s1{config_source::from_string{
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
</deployment>\n"}.name("s1")};
    config deployment{"deployment"};
    deployment << s1 << jet::lock;
    const std::vector<jet::config_node> regions{deployment.get_children_of("regions")};
    EXPECT_EQ(2U, regions.size());
    for(const jet::config_node& region : regions)
    {
        const std::string& region_name = region.node_name();
        const std::vector<jet::config_node> boxes{region.get_children_of("boxes")};
        if("US" == region_name)
            EXPECT_EQ(2U, boxes.size());
        else
            EXPECT_EQ(3U, boxes.size());
        unsigned index = 1;
        for(const jet::config_node& box : boxes)
        {
            const std::string expected_hostname{region_name + boost::lexical_cast<std::string>(index)};
            EXPECT_EQ(expected_hostname, box.get<std::string>("hostname"));
            const std::string expected_box_name{"deployment.regions." + region_name + ".boxes.box"};
            EXPECT_EQ(expected_box_name, box.name());
            ++index;
        }
    }
}

TEST(config, get_children_of_root)
{
    const config_source s1{config_source::from_string{
"<deployment>\n\
    <UK attr='1'/>\n\
    <US attr='2'/>\n\
    <HK attr='23'/>\n\
</deployment>\n"}.name("s1")};
    config deployment{"deployment"};
    deployment << s1 << jet::lock;
    
    jet::config_nodes nodes{deployment.get_children_of()};
    ASSERT_EQ(3U, nodes.size());
    EXPECT_EQ("UK", nodes[0].node_name());
    EXPECT_EQ("US", nodes[1].node_name());
    EXPECT_EQ("HK", nodes[2].node_name());
}

TEST(config, erase_xml_comments)
{
    const config_source s1{config_source::from_string{
R"(<deployment>
    <!--UK attr='1'/-->
    <US attr='2'/>
    <HK attr='23'/>
</deployment>
)"}.name("s1")};
    config deployment("deployment");
    deployment << s1 << jet::lock;
    
    jet::config_nodes nodes(deployment.get_children_of());
    ASSERT_EQ(2U, nodes.size());
    EXPECT_EQ("US", nodes[0].node_name());
    EXPECT_EQ("HK", nodes[1].node_name());
}

TEST(config, get_value_of_intermidiate_node)
{
    const config_source s1{config_source::from_string{
"<deployment>\n\
    <UK attr='1'/>\n\
    <US attr='2'/>\n\
    <HK attr='23'/>\n\
</deployment>\n"}.name("s1")};
    config deployment{"deployment"};
    deployment << s1 << jet::lock;
    
    EXPECT_EQ("1", deployment.get_node("UK.attr").get());
    
    EXPECT_CONFIG_ERROR(
        deployment.get_node("UK").get(),
        equal("Node 'deployment.UK' is intermidiate node without value"));
}

TEST(config, repeating_node_merge_without_conflicts)
{
    const config_source s1{config_source::from_string{
"<app>\n\
    <key attr='1'/>\n\
    <key attr='2'/>\n\
</app>\n"}.name("s1")};
    
    const config_source s2{config_source::from_string{
"<app>\n\
    <key1 attr='1'/>\n\
    <key1 attr='2'/>\n\
</app>\n"}.name("s2")};
    
    config config{"app"};
    
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
    const config_source s1{config_source::from_string{
"<app>\n\
    <key attr='1'/>\n\
</app>\n"}.name("s1")};
    
    const config_source s2{config_source::from_string{
"<app>\n\
    <key attr='1'/>\n\
    <key attr='2'/>\n\
</app>\n"}.name("s2")};
    
    config config{"app"};
    
    config << s1;
    EXPECT_CONFIG_ERROR(
        config << s2,
        equal("Can't do ambiguous merge of node 'key' from config source 's2' to config 'app'"));
}

TEST(config, repeating_node_merge_with_conflicts2)
{
    const config_source s1{config_source::from_string{
"<app>\n\
    <key attr='1'/>\n\
    <key attr='2'/>\n\
</app>\n"}.name("s1")};
    
    const config_source s2{config_source::from_string{
"<app>\n\
    <key attr='1'/>\n\
</app>\n"}.name("s2")};
    
    config config{"app"};
    
    config << s1;
    EXPECT_CONFIG_ERROR(
        config << s2,
        equal("Can't do ambiguous merge of node 'key' from config source 's2' to config 'app'"));
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
