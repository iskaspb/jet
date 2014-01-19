//
//  config_source.hpp
//
//  Created by Alexey Tkachenko on 1/9/13.
//  This code belongs to public domain. You can do with it whatever you want without any guarantee.
//

#ifndef JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD
#define JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD

#include <boost/shared_ptr.hpp>
#include <string>
#include <iosfwd>

namespace jet
{

class config_source
{
    class impl;
    explicit config_source(const boost::shared_ptr<impl>& impl);
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
    ~config_source();
    static config_source create_from_file(
        const std::string& filename,
        input_format format = xml,
        file_name_style = case_sensitive);
    const std::string& name() const;
    std::string to_string(output_type type = pretty) const;
private:
    boost::shared_ptr<impl> impl_;
    friend class config_node;
};

}//namespace jet

#endif /*JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD*/
