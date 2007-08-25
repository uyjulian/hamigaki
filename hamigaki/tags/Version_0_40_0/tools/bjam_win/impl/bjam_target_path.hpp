// bjam_target_path.hpp: bjam feature.as-path

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_TARGET_PATH_HPP
#define IMPL_BJAM_TARGET_PATH_HPP

#include <boost/filesystem/path.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tokenizer.hpp>
#include <map>
#include <string>
#include <vector>

inline std::pair<std::string,std::string> grist_split(const std::string& s)
{
    if (s.empty() || (s[0] != '<'))
        return std::make_pair(std::string(), s);

    std::string::size_type grist_end = s.find('>');
    if (grist_end == std::string::npos)
        return std::make_pair(std::string(), s);

    const std::string name(s, 1u, grist_end-1);
    const std::string value(s, grist_end+1);

    return std::make_pair(name, value);
}

inline void merge_requirement_impl(
    std::map<std::string,std::string>& props, const std::string& req)
{
    std::string name;
    std::string value;
    boost::tie(name, value) = ::grist_split(req);

    if (!name.empty())
        props[name] = value;
}

inline void merge_requirement(
    std::map<std::string,std::string>& props, const std::string& req)
{
    if (req.empty())
        return;

    std::string::size_type colon = req.find(':');
    if (colon != std::string::npos)
    {
        typedef boost::char_separator<char> separator;
        typedef boost::tokenizer<separator> tokenizer;
        typedef tokenizer::iterator iter_type;

        const std::string conds(req, 0u, colon);
        separator sep(",");
        tokenizer tokens(conds, sep);
        for (iter_type i = tokens.begin(); i != tokens.end(); ++i)
        {
            std::string name;
            std::string value;
            boost::tie(name, value) = ::grist_split(*i);

            // error
            if (name.empty())
                return;

            std::map<std::string,std::string>::iterator p = props.find(name);
            if ((p == props.end()) || (p->second != value))
                return;
        }
        ::merge_requirement_impl(props, req.substr(colon+1u));
    }
    else
        ::merge_requirement_impl(props, req);
}

inline std::map<std::string,std::string>&
merge_requirements(
    std::map<std::string,std::string>& props,
    const std::vector<std::string>& reqs)
{
    for (std::size_t i = 0; i < reqs.size(); ++i)
        ::merge_requirement(props, reqs[i]);
    return props;
}

inline boost::filesystem::path&
add_directory_for_property(
    boost::filesystem::path& ph,
    const std::map<std::string,std::string>& props,
    const std::string& name, const std::string& def_value)
{
    typedef std::map<std::string,std::string>::const_iterator iter_type;
    iter_type i = props.find(name);
    if ((i != props.end()) && (i->second != def_value))
    {
        std::string tmp = name;
        tmp += '-';
        tmp += i->second;
        ph /= tmp;
    }
    return ph;
}

inline boost::filesystem::path
bjam_target_path(const std::map<std::string,std::string>& props)
{
    boost::filesystem::path ph;
    ph = props.find("toolset")->second;
    ph /= props.find("variant")->second;
    ::add_directory_for_property(ph, props, "asynch-exceptions", "off");
    ::add_directory_for_property(ph, props, "debug-symbols", "on");
    ::add_directory_for_property(ph, props, "inlining", "off");
    ::add_directory_for_property(ph, props, "link", "shared");
    ::add_directory_for_property(ph, props, "optimization", "off");
    ::add_directory_for_property(ph, props, "rtti", "on");
    ::add_directory_for_property(ph, props, "runtime-link", "shared");
    ::add_directory_for_property(ph, props, "stdlib", "native");
    ::add_directory_for_property(ph, props, "threading", "single");
    ::add_directory_for_property(ph, props, "user-interface", "console");
    return ph;
}

#endif // IMPL_BJAM_TARGET_PATH_HPP
