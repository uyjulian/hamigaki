// bjam_preload.hpp: pre-loading for bjam configuration files

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_PRELOAD_HPP
#define IMPL_BJAM_PRELOAD_HPP

#include "./bjam_grammar.hpp"
#include <fstream>
#include <iterator>
#include <windows.h>

namespace impl
{

class environment_strings : private boost::noncopyable
{
public:
    environment_strings() : ptr_(::GetEnvironmentStringsA())
    {
    }

    ~environment_strings()
    {
        ::FreeEnvironmentStringsA(ptr_);
    }

    const char* get() const
    {
        return ptr_;
    }

private:
    char* ptr_;
};

inline void add_variable(
    variables& vars, const std::string& name, const std::string& value)
{
    std::vector<std::string> values;
    values.push_back(value);

    vars.add(name);
    vars.assign(name, values);
}

inline void import_environment_variables(variables& vars)
{
    impl::environment_strings env;
    const char* p = env.get();
    while (*p)
    {
        std::string s(p);
        p += (s.size() + 1);

        std::string::size_type eq = s.find('=', 1u);
        std::string name(s, 0, eq);
        std::string value(s, eq+1);

        impl::add_variable(vars, name, value);
    }
}

inline bool bjam_preload_impl(
    const std::string& filename,
    bjam_context& ctx, variables& vars, rule_table& rules)
{
    namespace fs = boost::filesystem;

    ctx.working_directory = fs::path(filename, fs::no_check).branch_path();

    std::ifstream is(filename.c_str(), std::ios_base::binary);
    if (!is)
        return false;

    std::string src(
        std::istreambuf_iterator<char>(is),
        (std::istreambuf_iterator<char>())
    );

    bjam_grammar g(ctx, vars, rules);

    return boost::spirit::parse(src.c_str(), g).full;
}

inline bool parse_project_root(
    const boost::filesystem::path& ph,
    bjam_context& ctx, variables& vars, rule_table& rules)
{
    const char* root_files[] =
    {
        "project-root.jam",
        "Jamroot",
        "jamroot.jam"
    };

    const std::size_t root_count = sizeof(root_files)/sizeof(root_files[0]);

    for (std::size_t i = 0; i < root_count; ++i)
    {
        std::string filename = (ph / root_files[i]).native_file_string();
        if (impl::bjam_preload_impl(filename, ctx, vars, rules))
            return true;
    }
    return false;
}

} // namespace impl

inline void bjam_preload(
    const boost::filesystem::path& work, variables& vars, rule_table& rules)
{
    namespace fs = boost::filesystem;
    typedef std::vector<std::string> list_type;

    bjam_context ctx;

    impl::import_environment_variables(vars);

    // Note: replace the environment variable "OS"
#if defined(__CYGWIN__)
    impl::add_variable(vars, "OS", "CYGWIN");
    impl::add_variable(vars, "UNIX", "true");
#else
    impl::add_variable(vars, "OS", "NT");
    impl::add_variable(vars, "NT", "true");
#endif

# if defined( __ia64__ ) || defined( __IA64__ )
    impl::add_variable(vars, "OSPLAT", "IA64");
#else
    impl::add_variable(vars, "OSPLAT", "X86");
# endif

    std::vector<std::string> user_dirs;
    if (const list_type* p1 = vars.find("HOMEDRIVE"))
    {
        if (const list_type* p2 = vars.find("HOMEPATH"))
            user_dirs.push_back(p1->front() + p2->front());
    }

    if (const list_type* p = vars.find("HOME"))
        user_dirs.push_back(p->front());
    if (const list_type* p = vars.find("USERPROFILE"))
        user_dirs.push_back(p->front());
    if (const list_type* p = vars.find("BOOST_BUILD_PATH"))
        user_dirs.push_back(p->front());

    std::vector<std::string> site_dirs;
    if (const list_type* p = vars.find("SystemRoot"))
        site_dirs.push_back(p->front());
    site_dirs.insert(site_dirs.end(), user_dirs.begin(), user_dirs.end());

    for (std::size_t i = 0; i < site_dirs.size(); ++i)
    {
        fs::path ph(site_dirs[i], fs::no_check);
        ph /= "site-config.jam";
        if (impl::bjam_preload_impl(ph.native_file_string(), ctx, vars, rules))
            break;
    }

    for (std::size_t i = 0; i < user_dirs.size(); ++i)
    {
        fs::path ph(user_dirs[i], fs::no_check);
        ph /= "user-config.jam";
        if (impl::bjam_preload_impl(ph.native_file_string(), ctx, vars, rules))
            break;
    }

    fs::path ph = work;
    while (!impl::parse_project_root(ph, ctx, vars, rules))
    {
        if (!ph.has_branch_path())
            break;

        ph = ph.branch_path();
    }
}

#endif // IMPL_BJAM_PRELOAD_HPP
