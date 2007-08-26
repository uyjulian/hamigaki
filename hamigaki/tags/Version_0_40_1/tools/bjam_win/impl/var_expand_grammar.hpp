// var_expand_grammar.hpp: bjam variable expansion grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_VAR_EXPAND_GRAMMAR_HPP
#define IMPL_VAR_EXPAND_GRAMMAR_HPP

#include "./variables.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/phoenix/binders.hpp>
#include <algorithm>

struct vars_closure
    : boost::spirit::closure<
        vars_closure,
        variables::mapped_type
    >
{
    member1 val;
};

struct range_closure
    : boost::spirit::closure<
        range_closure,
        std::pair<int,int>,
        int,
        int
    >
{
    member1 val;
    member2 lower;
    member3 upper;
};

struct expand_closure
    : boost::spirit::closure<
        expand_closure,
        variables::mapped_type,
        variables::mapped_type
    >
{
    member1 val;
    member2 rep;
};

class vars_expand_actor
{
public:
    typedef std::vector<std::string> result_type;

    explicit vars_expand_actor(const variables& t)
        : table_(t)
    {
    }

    std::vector<std::string>
    operator()(const std::vector<std::string>& vs) const
    {
        std::vector<std::string> result;
        for (std::size_t i = 0; i < vs.size(); ++i)
        {
            const std::vector<std::string>* p = table_.find(vs[i]);
            if (p)
                result.insert(result.end(), p->begin(), p->end());
        }
        return result;
    }

private:
    const variables& table_;
};

struct slice_actor
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(
        const std::vector<std::string>& vs,
        const std::pair<int,int>& r) const
    {
        int n = r.first;
        int m = r.second;

        const int size = static_cast<int>(vs.size());

        if (n < 0)
            n += size;

        if (m == 0)
            m = size;
        else
            m += (size + 1);

        std::vector<std::string> result;
        for (int i = n; i <= m; ++i)
        {
            if (i <= size)
                result.push_back(vs[i-1]);
        }
        return result;
    }
};

struct upper_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
            boost::algorithm::to_upper(vs[i]);
    }
};

struct lower_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
            boost::algorithm::to_lower(vs[i]);
    }
};

struct convert_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
            boost::algorithm::replace_all(vs[i], "/", "\\");
    }
};

inline std::string::size_type find_grist_end(const std::string& s)
{
    if (s.empty() || (s[0] != '<'))
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
    std::string::size_type sl = s.rfind('/');
    std::string::size_type bs = s.rfind('\\');

    if (sl == std::string::npos)
        sl = start;
    if (bs == std::string::npos)
        bs = start;

    std::string::size_type pos = (std::max)((std::max)(sl, bs), start);
    if ((pos == start) && (pos < s.size()) &&
        ((s[pos] == '/') || (s[pos] == '\\')) )
    {
        ++pos;
    }
    else if ((pos > start) && (s[pos-1] == ':'))
        ++pos;
    return pos;
}

inline std::string::size_type find_basename(
    const std::string& s, std::string::size_type start)
{
    std::string::size_type sl = s.rfind('/');
    std::string::size_type bs = s.rfind('\\');

    if (sl == std::string::npos)
        sl = start;
    else
        sl = sl + 1;

    if (bs == std::string::npos)
        bs = start;
    else
        bs = bs + 1;

    return (std::max)((std::max)(sl, bs), start);
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

struct parent_dir_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
        {
            const std::string& s = vs[i];
            std::string::size_type start = ::find_grist_end(s);
            std::string::size_type end = ::find_dir_end(s, start);
            vs[i] = s.substr(start, end - start);
        }
    }
};

struct grist_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        if (!val.empty())
        {
            std::vector<std::string> result;
            for (std::size_t j = 0; j < val.size(); ++j)
            {
                std::string grist;
                grist += '<';
                grist += val[j];
                grist += '>';

                for (std::size_t i = 0; i < vs.size(); ++i)
                {
                    std::string s = vs[i];
                    s.replace(0, ::find_grist_end(s), grist);
                    result.push_back(s);
                }
            }

            val.clear();
            vs.swap(result);
        }
        else
        {
            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                const std::string& s = vs[i];
                vs[i] = s.substr(0, ::find_grist_end(s));
            }
        }
    }
};

struct directory_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        if (!val.empty())
        {
            std::vector<std::string> result;
            for (std::size_t j = 0; j < val.size(); ++j)
            {
                std::string dir = val[j];
                dir += '\\';

                for (std::size_t i = 0; i < vs.size(); ++i)
                {
                    std::string s = vs[i];
                    std::string::size_type start = ::find_grist_end(s);
                    std::string::size_type end = ::find_basename(s, start);
                    std::string::size_type member = ::find_member(s, end);
                    if (end != member)
                        s.replace(start, end-start, dir);
                    else
                        s.replace(start, end-start, val[j]);
                    result.push_back(s);
                }
            }

            val.clear();
            vs.swap(result);
        }
        else
        {
            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                const std::string& s = vs[i];
                std::string::size_type start = ::find_grist_end(s);
                std::string::size_type end = ::find_dir_end(s, start);
                vs[i] = s.substr(start, end-start);
            }
        }
    }
};

struct basename_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        if (!val.empty())
        {
            std::vector<std::string> result;
            for (std::size_t j = 0; j < val.size(); ++j)
            {
                for (std::size_t i = 0; i < vs.size(); ++i)
                {
                    std::string s = vs[i];
                    std::string::size_type dir = ::find_grist_end(s);
                    std::string::size_type start = ::find_basename(s, dir);
                    std::string::size_type member = ::find_member(s, start);
                    std::string::size_type dot = ::find_dot(s, start, member);
                    s.replace(start, dot-start, val[j]);
                    result.push_back(s);
                }
            }

            val.clear();
            vs.swap(result);
        }
        else
        {
            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                const std::string& s = vs[i];
                std::string::size_type dir = ::find_grist_end(s);
                std::string::size_type start = ::find_basename(s, dir);
                std::string::size_type member = ::find_member(s, start);
                std::string::size_type dot = ::find_dot(s, start, member);
                vs[i] = s.substr(start, dot-start);
            }
        }
    }
};

struct suffix_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        if (!val.empty())
        {
            std::vector<std::string> result;
            for (std::size_t j = 0; j < val.size(); ++j)
            {
                for (std::size_t i = 0; i < vs.size(); ++i)
                {
                    std::string s = vs[i];
                    std::string::size_type dir = ::find_grist_end(s);
                    std::string::size_type base = ::find_basename(s, dir);
                    std::string::size_type member = ::find_member(s, base);
                    std::string::size_type dot = ::find_dot(s, base, member);
                    s.replace(dot, member-dot, val[j]);
                    result.push_back(s);
                }
            }

            val.clear();
            vs.swap(result);
        }
        else
        {
            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                const std::string& s = vs[i];
                std::string::size_type dir = ::find_grist_end(s);
                std::string::size_type base = ::find_basename(s, dir);
                std::string::size_type member = ::find_member(s, base);
                std::string::size_type dot = ::find_dot(s, base, member);
                vs[i] = s.substr(dot, member-dot);
            }
        }
    }
};

struct member_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        if (!val.empty())
        {
            std::vector<std::string> result;
            for (std::size_t j = 0; j < val.size(); ++j)
            {
                std::string strval;
                strval += '(';
                strval += val[j];
                strval += ')';

                for (std::size_t i = 0; i < vs.size(); ++i)
                {
                    std::string s = vs[i];
                    std::string::size_type dir = ::find_grist_end(s);
                    std::string::size_type base = ::find_basename(s, dir);
                    std::string::size_type member = ::find_member(s, base);
                    s.replace(member, s.size()-member, strval);
                    result.push_back(s);
                }
            }

            val.clear();
            vs.swap(result);
        }
        else
        {
            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                const std::string& s = vs[i];
                std::string::size_type dir = ::find_grist_end(s);
                std::string::size_type base = ::find_basename(s, dir);
                std::string::size_type member = ::find_member(s, base);
                vs[i] = s.substr(member, s.size()-member);
            }
        }
    }
};

struct root_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        std::vector<std::string> result;
        for (std::size_t j = 0; j < val.size(); ++j)
        {
            std::string root = val[j];
            if (!root.empty() &&
                (*root.rbegin()!='/') && (*root.rbegin()!='\\') )
            {
                root += '\\';
            }

            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                std::string s = vs[i];
                std::string::size_type start = ::find_grist_end(s);
                std::string::size_type end = ::find_basename(s, start);

                bool has_root = false;

                if (start != end)
                {
                    if ((s[start] == '/') || (s[start] == '\\'))
                        has_root = true;
                    else if ((end - start >= 3u) && (s[start+1] == ':') &&
                        ((s[start+2] == '/') || (s[start+2] == '\\')) )
                    {
                        has_root = true;
                    }
                }

                if (!has_root)
                    s.insert(start, root);
                result.push_back(s);
            }
        }

        val.clear();
        vs.swap(result);
    }
};

struct empty_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        if (vs.empty())
            vs = val;

        val.clear();
    }
};

struct join_actor
{
    typedef void result_type;

    void operator()(
        std::vector<std::string>& vs,
        std::vector<std::string>& val) const
    {
        std::string str;
        for (std::size_t i = 0; i < vs.size(); ++i)
        {
            if (i != 0)
                str += *val.rbegin();
            str += vs[i];
        }

        std::vector<std::string> tmp;
        if (!str.empty())
            tmp.push_back(str);
        vs.swap(tmp);

        val.clear();
    }
};

struct make_str_vec_actor
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(const char* first, const char* last) const
    {
        std::vector<std::string> tmp;
        tmp.push_back(std::string(first, last));
        return tmp;
    }
};

struct append_str_vec_actor
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(
        const std::vector<std::string>& lhs,
        const std::vector<std::string>& rhs) const
    {
        std::vector<std::string> result;
        for (std::size_t i = 0; i < lhs.size(); ++i)
        {
            for (std::size_t j = 0; j < rhs.size(); ++j)
                result.push_back(lhs[i] + rhs[j]);
        }
        return result;
    }
};

struct var_expand_grammar
    : boost::spirit::grammar<var_expand_grammar,vars_closure::context_t>
{
    explicit var_expand_grammar(const variables& t)
        : table(t)
    {
    }

    const variables& table;

    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        typedef boost::spirit::rule<
            ScannerT, typename vars_closure::context_t> vars_rule_t;

        typedef boost::spirit::rule<
            ScannerT, typename range_closure::context_t> range_rule_t;

        typedef boost::spirit::rule<
            ScannerT, typename expand_closure::context_t> expand_rule_t;

        vars_rule_t top, expr;
        range_rule_t index_range;
        expand_rule_t variable;

        definition(const var_expand_grammar& self)
        {
            using namespace boost::spirit;
            using namespace phoenix;

            index_range
                =   '['
                    >> int_p
                    [
                        index_range.lower = index_range.upper = arg1
                    ]
                    >> !(
                        ch_p('-')[index_range.upper = 0]
                        >> !int_p[index_range.upper = arg1]
                        )
                    >> ch_p(']')
                        [
                            index_range.val =
                                construct_<std::pair<int,int> >(
                                    index_range.lower, index_range.upper
                                )
                        ]
                ;

            variable
                =   "$("
                    >> expr
                        [
                            variable.val =
                                bind(vars_expand_actor(self.table))(arg1)
                        ]
                    >> !index_range
                        [
                            variable.val =
                                bind(slice_actor())(variable.val, arg1)
                        ]
                    >> *(
                            ':'
                            >> +(   ch_p('P')
                                    [
                                        bind(parent_dir_actor())(variable.val)
                                    ]
                                |   ch_p('U')
                                    [
                                        bind(upper_actor())(variable.val)
                                    ]
                                |   ch_p('L')
                                    [
                                        bind(lower_actor())(variable.val)
                                    ]
                                |   ch_p('T')
                                    [
                                        bind(convert_actor())(variable.val)
                                    ]
                                |   ch_p('G')
                                    >> !('=' >> expr[variable.rep = arg1])
                                    >> eps_p
                                    [
                                        bind(grist_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('D')
                                    >> !('=' >> expr[variable.rep = arg1]
                                        )
                                    >> eps_p
                                    [
                                        bind(directory_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('B')
                                    >> !('=' >> expr[variable.rep = arg1]
                                        )
                                    >> eps_p
                                    [
                                        bind(basename_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('S')
                                    >> !('=' >> expr[variable.rep = arg1]
                                        )
                                    >> eps_p
                                    [
                                        bind(suffix_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('M')
                                    >> !('=' >> expr[variable.rep = arg1]
                                        )
                                    >> eps_p
                                    [
                                        bind(member_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('R')
                                    >> '=' >> expr[variable.rep = arg1]
                                    >> eps_p
                                    [
                                        bind(root_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('E')
                                    >> '=' >> expr[variable.rep = arg1]
                                    >> eps_p
                                    [
                                        bind(empty_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   ch_p('J')
                                    >> '=' >> expr[variable.rep = arg1]
                                    >> eps_p
                                    [
                                        bind(join_actor())
                                            (variable.val, variable.rep)
                                    ]
                                |   alpha_p
                                    >> !( '=' >> expr )
                                )
                        )
                    >> ')'
                ;

            expr
                =   eps_p[expr.val = bind(make_str_vec_actor())(arg1, arg2)]
                    >>
                   *(   variable
                        [
                            expr.val =
                                bind(append_str_vec_actor())(expr.val, arg1)
                        ]
                    |   (+(anychar_p - '$' - ')' - '[' - ':'))
                        [
                            expr.val =
                                bind(append_str_vec_actor())(
                                    expr.val, 
                                    bind(make_str_vec_actor())(arg1, arg2)
                                )
                        ]
                    )
                ;

            top
                =   eps_p[top.val = bind(make_str_vec_actor())(arg1, arg2)]
                    >>
                   *(   variable
                        [
                            top.val =
                                bind(append_str_vec_actor())(top.val, arg1)
                        ]
                    |   (+(anychar_p - '$'))
                        [
                            top.val =
                                bind(append_str_vec_actor())(
                                    top.val, 
                                    bind(make_str_vec_actor())(arg1, arg2)
                                )
                        ]
                    )
                    >> eps_p[self.val = top.val]
                ;
        }

        const vars_rule_t& start() const
        {
            return top;
        }
    };
};

#endif // IMPL_VAR_EXPAND_GRAMMAR_HPP
