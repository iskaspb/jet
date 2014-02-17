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
    config_source(std::unique_ptr<impl>);
public:
    enum input_format{ xml, json };
    enum output_type { pretty, one_line };
    enum file_name_style { case_sensitive, case_insensitive };
    //...
    struct from_string
    {
        from_string(const std::string& source): source_{source} {}
        from_string(std::string&& source): source_{source} {}
        from_string(const from_string&) = default;
        from_string& operator=(const from_string&) = default;
        from_string(from_string&&) = default;
        from_string& operator=(from_string&&) = default;
        //...
        from_string& name(const std::string& name)
        {
            name_ = name;
            return *this;
        }
        from_string& input_format(config_source::input_format fmt)
        {
            fmt_ = fmt;
            return *this;
        }
        from_string& file_name_style(config_source::file_name_style name_style)
        {
            name_style_ = name_style;
            return *this;
        }
        config_source create();
    private:
        std::string name_{"unknown"};
        std::string source_;
        config_source::input_format fmt_ { config_source::xml };
        config_source::file_name_style name_style_ { config_source::case_sensitive };
    };
    struct from_stream
    {
        from_stream(std::istream& strm): strm_(&strm) {}
        //...
        from_stream& name(const std::string& name)
        {
            name_ = name;
            return *this;
        }
        from_stream& input_format(config_source::input_format fmt)
        {
            fmt_ = fmt;
            return *this;
        }
        from_stream& file_name_style(config_source::file_name_style name_style)
        {
            name_style_ = name_style;
            return *this;
        }
        config_source create();
    private:
        std::string name_{"unknown"};
        std::istream* strm_;
        config_source::input_format fmt_ { config_source::xml };
        config_source::file_name_style name_style_ { config_source::case_sensitive };
    };
    struct from_file
    {
        from_file(const std::string& filename): filename_(filename) {}
        from_file& input_format(config_source::input_format fmt)
        {
            fmt_ = fmt;
            return *this;
        }
        from_file& file_name_style(config_source::file_name_style name_style)
        {
            name_style_ = name_style;
            return *this;
        }
        config_source create();
    private:
        std::string filename_;
        config_source::input_format fmt_ { config_source::xml };
        config_source::file_name_style name_style_ { config_source::case_sensitive };
    };
    struct from_cmd_line
    {
        from_cmd_line(
            const std::string& source,
            const std::string& app_name):
            source_(source),
            app_name_(app_name)
        {}
        from_cmd_line& instance_name(const std::string& name)
        {
            instance_name_ = name;
            return *this;
        }
        config_source create();
    private:
        std::string source_, app_name_, instance_name_;
    };
    //...
    template<typename factory_type>
    explicit config_source(factory_type factory):
        config_source(std::move(factory.create()))
    {}
    config_source(const config_source& other);
    config_source& operator=(const config_source& other);
    config_source(config_source&& other);
    config_source& operator=(config_source&& other);
    ~config_source();
    const std::string& name() const;
    std::string to_string(output_type type = pretty) const;
private:
    std::unique_ptr<impl> impl_;
    friend class config_node;
};

}//namespace jet

#endif /*JET_CONFIG_CONFIG_SOURCE_HEADER_GUARD*/
