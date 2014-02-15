// jet.config library
//
//  Copyright Alexey Tkachenko 2014. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD
#define JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD

#include <string>
#include <iosfwd>
#include <memory>

namespace jet
{

class config_source
{
    class impl;
    explicit config_source(std::unique_ptr<impl>);
public:
    enum input_format{ xml, json };
    enum output_type { pretty, one_line };
    enum file_name_style { case_sensitive, case_insensitive };
    //...
    explicit config_source(
        const std::string& source,
        const std::string& name = "unknown",
        input_format format = xml,
        file_name_style = case_sensitive);
    explicit config_source(
        std::istream& source,
        const std::string& name = "unknown",
        input_format format = xml,
        file_name_style = case_sensitive);
    config_source(const config_source& other);
    config_source& operator=(const config_source& other);
    config_source(config_source&& other);
    config_source& operator=(config_source&& other);
    ~config_source();
    //...
    static config_source create_from_file(
        const std::string& filename,
        input_format format = xml,
        file_name_style = case_sensitive);
    static config_source create_naive(
        const std::string& source,
        const std::string& app_name,
        const std::string& instance_name = std::string());
    const std::string& name() const;
    std::string to_string(output_type type = pretty) const;
private:
    std::unique_ptr<impl> impl_;
    friend class config_node;
};

}//namespace jet

#endif /*JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD*/
