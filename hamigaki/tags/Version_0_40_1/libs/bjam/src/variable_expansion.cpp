// variable_expansion.cpp: bjam variable expansion

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/cartesian_product.hpp>
#include <hamigaki/bjam/util/path.hpp>
#include <hamigaki/bjam/util/variable_expansion.hpp>
#include <hamigaki/integer/auto_max.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#if defined(__CYGWIN__)
    #include <sys/cygwin.h>
    #include <windef.h>
#endif

namespace hamigaki { namespace bjam {

namespace
{

typedef std::string::size_type size_type;

struct magic
{
    static const char colon     = '\x00';
    static const char lbracket  = '\x01';
    static const char rbracket  = '\x02';
};

struct convert_to_magic
{
    typedef char result_type;

    char operator()(char c) const
    {
        if (c == ':')
            return magic::colon;
        else if (c == '[')
            return magic::lbracket;
        else if (c == ']')
            return magic::rbracket;
        else
            return c;
    }
};

inline size_type find_lparen_nested(const std::string& s, size_type offset)
{
    const size_type size = s.size();
    std::size_t depth = 1;
    for ( ; offset < size; ++offset)
    {
        if (s[offset] == '(')
            ++depth;
        else if (s[offset] == ')')
        {
            if (--depth == 0)
                return offset;
        }
    }
    return std::string::npos;
}

int parse_int(const std::string& s, std::size_t& pos)
{
    bool minus = false;
    if (s[pos] == '-')
    {
        minus = true;
        ++pos;
    }

    int n = 0;
    while (std::isdigit(s[pos], std::locale::classic()))
        n = n*10 + (s[pos++] - '0');

    if (minus)
        return -n;
    else
        return n;
}

std::pair<int,int> parse_index_range(const std::string& s)
{
    std::size_t pos = 0;
    if ((s[pos] != '-') && !std::isdigit(s[pos], std::locale::classic()))
        return std::pair<int,int>(0,0);

    int lower = parse_int(s, pos);

    if (pos == s.size())
        return std::pair<int,int>(lower,lower);

    if (s[pos] != '-')
        return std::pair<int,int>(lower,0);

    if (++pos == s.size())
        return std::pair<int,int>(lower,-1);

    if ((s[pos] != '-') && !std::isdigit(s[pos], std::locale::classic()))
        return std::pair<int,int>(lower,0);

    int upper = parse_int(s, pos);

    if (pos == s.size())
        return std::pair<int,int>(lower,upper);
    else
        return std::pair<int,int>(lower,0);
}

struct modifiers
{
    static const unsigned lower     = 0x01;
    static const unsigned upper     = 0x02;
    static const unsigned parent    = 0x04;
    static const unsigned empty     = 0x08;
    static const unsigned join      = 0x10;
    static const unsigned slash     = 0x20;
    static const unsigned windows   = 0x40;

    modifiers() : flags(0)
    {
    }

    unsigned flags;
    std::string empty_value;
    std::string join_value;
    boost::optional<std::string> grist_value;
    boost::optional<std::string> root_value;
    boost::optional<std::string> dir_value;
    boost::optional<std::string> base_value;
    boost::optional<std::string> suffix_value;
    boost::optional<std::string> member_value;

    bool has_path_components() const
    {
        return
            grist_value || root_value || dir_value ||
            base_value || suffix_value || member_value ||
            ((flags & parent) != 0);
    }
};

std::string parse_modifier_string(const std::string& s, std::size_t& pos)
{
    if (s[pos] != '=')
        return std::string();

    size_type start = ++pos;
    size_type colon = s.find(magic::colon, start);
    if (colon != std::string::npos)
    {
        pos = colon + 1;
        return s.substr(start, colon - start);
    }
    else
    {
        pos = s.size();
        return s.substr(start);
    }
}

boost::optional<std::string>
parse_modifier_filename(
    modifiers*& mods_ptr, const std::string& s, std::size_t& pos)
{
    if (s[pos] != '=')
    {
        if (mods_ptr)
        {
            mods_ptr->grist_value = std::string();
            mods_ptr->root_value = std::string();
            mods_ptr->dir_value = std::string();
            mods_ptr->base_value = std::string();
            mods_ptr->suffix_value = std::string();
            mods_ptr->member_value = std::string();
            mods_ptr = 0;
        }
        return boost::optional<std::string>();
    }

    return parse_modifier_string(s, pos);
}

void parse_modifiers(modifiers& mods, const std::string& s)
{
    modifiers* mods_ptr = &mods;
    for (std::size_t i = 0, size = s.size(); i < size;)
    {
        char c = s[i++];
        if (c == 'L')
            mods.flags |= modifiers::lower;
        else if (c == 'U')
            mods.flags |= modifiers::upper;
        else if (c == 'P')
            mods.flags |= modifiers::parent;
        else if (c == 'E')
        {
            mods.flags |= modifiers::empty;
            mods.empty_value = parse_modifier_string(s, i);
        }
        else if (c == 'J')
        {
            mods.flags |= modifiers::join;
            mods.join_value = parse_modifier_string(s, i);
        }
        else if (c == 'G')
            mods.grist_value = parse_modifier_filename(mods_ptr, s, i);
        else if (c == 'R')
            mods.root_value = parse_modifier_filename(mods_ptr, s, i);
        else if (c == 'D')
            mods.dir_value = parse_modifier_filename(mods_ptr, s, i);
        else if (c == 'B')
            mods.base_value = parse_modifier_filename(mods_ptr, s, i);
        else if (c == 'S')
            mods.suffix_value = parse_modifier_filename(mods_ptr, s, i);
        else if (c == 'M')
            mods.member_value = parse_modifier_filename(mods_ptr, s, i);
        else if (c == 'T')
            mods.flags |= modifiers::slash;
        else if (c == 'W')
            mods.flags |= modifiers::windows;
        else
            break;
    }
}

std::string apply_modifiers(const std::string& value, const modifiers& mods)
{
    std::string result = value;

#if defined(__CYGWIN__)
    if ((mods.flags & modifiers::windows) != 0)
    {
        char buf[MAX_PATH];
        if (::cygwin_conv_to_win32_path(result.c_str(), buf) != -1)
            result = buf;
    }
#endif

    if ((mods.flags & modifiers::upper) != 0)
        boost::algorithm::to_upper(result, std::locale::classic());
    else if ((mods.flags & modifiers::lower) != 0)
        boost::algorithm::to_lower(result, std::locale::classic());

    if ((mods.flags & modifiers::slash) != 0)
        boost::algorithm::replace_all(result, "\\", "/");

    if (mods.has_path_components())
    {
        path_components ph;
        bjam::split_path(ph, result);

        if (mods.grist_value)
            ph.grist = *mods.grist_value;
        if (mods.root_value)
            ph.root = *mods.root_value;
        if (mods.dir_value)
            ph.dir = *mods.dir_value;

        if ((mods.flags & modifiers::parent) != 0)
        {
            ph.base.clear();
            ph.suffix.clear();
            ph.member.clear();
        }
        else
        {
            if (mods.base_value)
                ph.base = *mods.base_value;
            if (mods.suffix_value)
                ph.suffix = *mods.suffix_value;
            if (mods.member_value)
                ph.member = *mods.member_value;
        }

        result = bjam::make_path(ph);
    }

    return result;
}

void expand_variable_impl(
    string_list& result, const std::string& prefix, const std::string& s,
    const variable_table& table, const list_of_list& args, bool is_last)
{
    const size_type colon = s.find(magic::colon);

    modifiers mods;
    if (colon != std::string::npos)
    {
        parse_modifiers(mods, s.substr(colon+1));

        // Note: This may be a bjam bug
        if (((mods.flags & modifiers::join) != 0) && !is_last)
            return;
    }

    size_type name_end = colon;

    size_type lbracket = s.find(magic::lbracket);
    std::pair<int,int> rng(0,-1);
    if (lbracket < colon)
    {
        size_type rbracket =
            (colon != std::string::npos) ? colon - 1 : s.size()-1;

        // Note: the change from the original bjam
        if (s[rbracket] != magic::rbracket)
            return;

        rng = parse_index_range(s.substr(lbracket+1, rbracket-(lbracket+1)));
        name_end = lbracket;
    }
    else
        lbracket = std::string::npos;


    string_list values_buf;
    const string_list& values =
        get_variable_values(values_buf, s.substr(0, name_end), table, args);

    if (rng.first < 0)
        rng.first += values.size();
    else
        --rng.first;

    if (rng.second < 0)
        rng.second += values.size() + 1;
    if (rng.second <= rng.first)
        return;

    const std::size_t start = hamigaki::auto_max(rng.first, 0u);
    const std::size_t last =
        (std::min)(static_cast<std::size_t>(rng.second), values.size());

    if (start < values.size())
    {
        if ((mods.flags & modifiers::join) != 0)
        {
            std::string tmp = prefix;
            bool need_separator = false;
            for (std::size_t i = start; i < last; ++i)
            {
                if (need_separator)
                    tmp += mods.join_value;
                else
                    need_separator = true;
                tmp += apply_modifiers(values[i], mods);
            }
            result.push_back(tmp);
        }
        else
        {
            for (std::size_t i = start; i < last; ++i)
                result.push_back(prefix + apply_modifiers(values[i], mods));
        }
    }
    else if ((mods.flags & modifiers::empty) != 0)
        result.push_back(prefix + apply_modifiers(mods.empty_value, mods));
}

} // namespace

HAMIGAKI_BJAM_DECL
const string_list& get_variable_values(
    string_list& buf, const std::string& name, const variable_table& table)
{
    if (name == "TMPDIR")
    {
        buf.push_back(tmp_directory());
        return buf;
    }
    else if (name == "TMPNAME")
    {
        buf.push_back(tmp_filename());
        return buf;
    }
    else if (name == "TMPFILE")
    {
        buf.push_back(tmp_file_path());
        return buf;
    }
    else if (name == "STDOUT")
    {
        buf.push_back("STDOUT");
        return buf;
    }
    else if (name == "STDERR")
    {
        buf.push_back("STDERR");
        return buf;
    }
    else
        return table.get_values(name);
}

HAMIGAKI_BJAM_DECL
const string_list& get_variable_values(
    string_list& buf, const std::string& name,
    const variable_table& table, const list_of_list& args)
{
    if (name.size() == 1)
    {
        char c = name[0];
        if (c == '<')
            return args[0];
        else if (c == '>')
            return args[1];
        else if ((c >= '1') && (c <= '9'))
            return args[static_cast<std::size_t>(c - '1')];
        else
            return table.get_values(name);
    }
    else
        return get_variable_values(buf, name, table);
}

HAMIGAKI_BJAM_DECL
string_list expand_variable(
    const std::string& s,
    const variable_table& table, const list_of_list& args)
{
    const size_type dol = s.find("$(");
    if (dol == std::string::npos)
        return string_list(s);

    const size_type name_start = dol + 2;
    const size_type name_end = find_lparen_nested(s, name_start);

    // Note: the original bjam crashes in this case
    if (name_end == std::string::npos)
        return string_list();

    std::string name(
        boost::make_transform_iterator<convert_to_magic>(s.begin()+name_start),
        boost::make_transform_iterator<convert_to_magic>(s.begin()+name_end)
    );

    string_list names = bjam::expand_variable(name, table, args);

    const std::string prefix(s, 0, dol);
    string_list values;
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        bool is_last = i == (names.size() - 1);
        expand_variable_impl(values, prefix, names[i], table, args, is_last);
    }

    string_list rests =
        bjam::expand_variable(s.substr(name_end+1), table, args);

    string_list result;
    bjam::append_production(result, values, rests);
    return result;
}

} } // End namespaces bjam, hamigaki.
