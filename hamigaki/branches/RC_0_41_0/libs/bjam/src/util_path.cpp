// util_path.cpp: path utilities

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#define NOMINMAX
#include <hamigaki/bjam/util/path.hpp>
#include <hamigaki/detail/random.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

#if BOOST_VERSION >= 103500
    #include <boost/range/as_literal.hpp>
#endif
#include <boost/algorithm/string/join.hpp>

#if defined(BOOST_WINDOWS)
    #include <boost/scoped_array.hpp>
    #include <boost/assert.hpp>
    #include <windows.h>
#else
    #include <cstdlib>
#endif

namespace hamigaki { namespace bjam {

namespace
{

#if defined(BOOST_WINDOWS)
const char path_delimiter = '\\';
#else
const char path_delimiter = '/';
#endif

inline bool is_path_delimiter(char c)
{
#if defined(BOOST_WINDOWS)
    return (c == '/') || (c == '\\');
#else
    return c == '/';
#endif
}

inline std::string::size_type rfind_path_delimiter(const std::string& s)
{
#if defined(BOOST_WINDOWS)
    return s.find_last_of("/\\");
#else
    return s.rfind('/');
#endif
}

inline bool is_rooted(const std::string& s)
{
    if (is_path_delimiter(s[0]))
        return true;
#if defined(BOOST_WINDOWS)
    else if ((s.size() >= 2) && (s[1] == ':'))
        return true;
#endif
    return false;
}

inline bool ends_with_delimiter(const std::string& s)
{
    return !s.empty() && is_path_delimiter(*s.rbegin());
}

inline std::string::size_type find_grist_end(const std::string& s)
{
    if (s[0] != '<')
        return 0;
    else
    {
        std::string::size_type pos = s.find('>');
        if (pos == std::string::npos)
            return 0;
        else
            return pos+1;
    }
}

inline std::string::size_type find_dir_end(
    const std::string& s, std::string::size_type start)
{
    std::string::size_type pos = rfind_path_delimiter(s);
    if ((pos == std::string::npos) || (pos < start))
        return start;

    return pos + 1;
}

inline std::string::size_type find_member(
    const std::string& s, std::string::size_type start)
{
    if (s.empty() || (*s.rbegin() != ')'))
        return s.size();

    std::string::size_type pos = s.find('(', start);
    if (pos != std::string::npos)
        return pos;
    else
        return s.size();
}

inline std::string::size_type find_dot(
    const std::string& s,
    std::string::size_type start, std::string::size_type finish)
{
    std::string::size_type pos = s.find('.', start);
    if (pos < finish)
        return pos;
    else
        return finish;
}

} // namespace

HAMIGAKI_BJAM_DECL void split_path(path_components& ph, const std::string& s)
{
    typedef std::string::size_type size_type;

    size_type grist_end = find_grist_end(s);
    size_type dir_end = find_dir_end(s, grist_end);
    size_type member_start = find_member(s, dir_end);
    size_type dot = find_dot(s, dir_end, member_start);

    ph.grist = s.substr(0u, grist_end);
    ph.root.clear();
    ph.dir = s.substr(grist_end, dir_end-grist_end);
    ph.base = s.substr(dir_end, dot-dir_end);
    ph.suffix = s.substr(dot, member_start-dot);

    if (member_start != s.size())
        ph.member = s.substr(member_start+1, s.size() - member_start - 2);
    else
        ph.member.clear();
}

HAMIGAKI_BJAM_DECL std::string make_path(const path_components& ph)
{
    std::string buf;

    if (!ph.grist.empty())
    {
        if (ph.grist[0] != '<')
            buf += '<';
        buf += ph.grist;
        if (*ph.grist.rbegin() != '>')
            buf += '>';
    }

    if (!ph.root.empty() && !is_rooted(ph.dir))
    {
        buf += ph.root;
        if (!ends_with_delimiter(ph.root))
            buf += path_delimiter;
    }

    if (!ph.dir.empty())
    {
        buf += ph.dir;
        if (ph.base.empty() && ph.suffix.empty())
        {
            if (ends_with_delimiter(ph.dir))
                buf.resize(buf.size()-1);
        }
        else
        {
            if (!ends_with_delimiter(ph.dir))
                buf += path_delimiter;
        }
    }

    buf += ph.base;
    buf += ph.suffix;

    if (!ph.member.empty())
    {
        buf += '(';
        buf += ph.member;
        buf += ')';
    }

    return buf;
}

HAMIGAKI_BJAM_DECL std::string tmp_directory()
{
#if defined(BOOST_WINDOWS)
    ::DWORD size = ::GetTempPathA(0, 0);
    BOOST_ASSERT(size != 0);

    boost::scoped_array<char> buf(new char[size]);
    size = ::GetTempPathA(size, buf.get());

    // Note: "C:\\" -> "C:" !!!
    if ((size != 0) && (buf[size-1] == '\\'))
        --size;

    return std::string(buf.get(), size);
#else
    // Note: environment variables have potentially race conditions
    if (const char* s = std::getenv("TMPDIR"))
        return s;
    else
        return "/tmp";
#endif
}

HAMIGAKI_BJAM_DECL std::string tmp_filename()
{
    boost::uint32_t rnd = hamigaki::detail::random_ui32();
    return (boost::format("hamigaki_bjam_%1$08x.tmp") % rnd).str();
}

HAMIGAKI_BJAM_DECL std::string tmp_file_path()
{
    std::string s = tmp_directory();
    s += path_delimiter;
    s += tmp_filename();
    return s;
}

HAMIGAKI_BJAM_DECL std::string normalize_path(const string_list& parts)
{
    std::string result;
    if (!parts.empty() && (parts[0][0] == '/'))
        result += '/';

    namespace algo = boost::algorithm;
    std::string cat = algo::join(parts, "/");

    typedef boost::char_separator<char> sep_type;
    typedef boost::tokenizer<sep_type> tokenizer;

    sep_type sep("/");
    tokenizer tokens(cat, sep);

    std::vector<std::string> tmp;
    for (tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        const std::string& part = *i;
        if (part == ".")
            continue;
        else if (part == "..")
        {
            if (tmp.empty() || (tmp.back() == ".."))
                tmp.push_back("..");
            else
                tmp.pop_back();
        }
        else
            tmp.push_back(part);
    }

    result += algo::join(tmp, "/");

    if (result.empty())
        result = ".";

    return result;
}

} } // End namespaces bjam, hamigaki.
