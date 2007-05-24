// path.cpp: path utilities

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/path.hpp>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    #define HAMIGAKI_BJAM_WINDOWS
#endif

namespace hamigaki { namespace bjam {

namespace
{

#if defined(HAMIGAKI_BJAM_WINDOWS)
const char path_delimiter = '\\';
#else
const char path_delimiter = '/';
#endif

inline bool is_path_delimiter(char c)
{
#if defined(HAMIGAKI_BJAM_WINDOWS)
    return (c == '/') || (c == '\\');
#else
    return c == '/';
#endif
}

inline std::string::size_type rfind_path_delimiter(const std::string& s)
{
#if defined(HAMIGAKI_BJAM_WINDOWS)
    return s.find_last_of("/\\");
#else
    return s.rfind('/');
#endif
}

inline bool is_rooted(const std::string& s)
{
    if (is_path_delimiter(s[0]))
        return true;
#if defined(HAMIGAKI_BJAM_WINDOWS)
    else if ((s.size() >= 2) && (s[1] == ';'))
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

} } // End namespaces bjam, hamigaki.
