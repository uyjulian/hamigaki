// parse_tree.hpp: parse_tree() for bjam

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_PARSE_TREE_HPP
#define HAMIGAKI_BJAM2_UTIL_PARSE_TREE_HPP

#include <hamigaki/bjam2/util/node_val_data.hpp>
#include <boost/spirit/tree/parse_tree.hpp>

namespace hamigaki { namespace bjam2 {

template<
    class IteratorT,
    class NodeFactoryT = node_val_data_factory<boost::spirit::nil_t>,
    class T = boost::spirit::nil_t
>
struct tree_match_policy
    : public boost::spirit::common_tree_match_policy<
        tree_match_policy<IteratorT,NodeFactoryT,T>,
        IteratorT,
        NodeFactoryT,
        boost::spirit::pt_tree_policy<
            tree_match_policy<IteratorT,NodeFactoryT,T>,
            NodeFactoryT,
            T
        >,
        T
    >
{
    typedef boost::spirit::common_tree_match_policy<
        tree_match_policy<IteratorT,NodeFactoryT,T>,
        IteratorT,
        NodeFactoryT,
        boost::spirit::pt_tree_policy<
            tree_match_policy<IteratorT,NodeFactoryT,T>,
            NodeFactoryT,
            T
        >,
        T
    > common_tree_match_policy_;

    tree_match_policy()
    {
    }

    template<class PolicyT>
    tree_match_policy(const PolicyT& policies)
        : common_tree_match_policy_(policies)
    {
    }

    template<class AttrT, class Iterator1T, class Iterator2T>
    static boost::spirit::tree_match<IteratorT,NodeFactoryT,AttrT>
    create_match(
        std::size_t length,
        const AttrT& val,
        const Iterator1T& first,
        const Iterator2T& last)
    {
        typedef typename common_tree_match_policy_::tree_policy_t tree_policy_t;

        return boost::spirit::tree_match<IteratorT,NodeFactoryT,AttrT>(
            length, val,
            tree_policy_t::create_node(length, first, last, true)
        );
    }

    template<class Iterator1T, class Iterator2T>
    static boost::spirit::tree_match<
        IteratorT,
        NodeFactoryT,
        std::basic_string<
            typename std::iterator_traits<Iterator1T>::value_type
        >
    >
    create_match(
        std::size_t length,
        const std::basic_string<
            typename std::iterator_traits<Iterator1T>::value_type
        >& val,
        const Iterator1T&,
        const Iterator2T&)
    {
        typedef typename std::iterator_traits<Iterator1T>::value_type char_t;
        typedef std::basic_string<char_t> string_t;
        typedef typename common_tree_match_policy_::tree_policy_t tree_policy_t;

        return boost::spirit::tree_match<IteratorT,NodeFactoryT,string_t>(
            length, val,
            tree_policy_t::create_node(length, val.begin(), val.end(), true)
        );
    }

    template<class Iterator1T, class Iterator2T>
    static boost::spirit::tree_match<
        IteratorT,
        NodeFactoryT,
        std::basic_string<
            typename std::iterator_traits<Iterator1T>::value_type
        >
    >
    create_match(
        std::size_t length,
        const std::basic_string<
            typename std::iterator_traits<Iterator1T>::value_type
        >& val,
        const hamigaki::line_counting_iterator<Iterator1T>& first,
        const hamigaki::line_counting_iterator<Iterator2T>& last)
    {
        typedef typename std::iterator_traits<Iterator1T>::value_type char_t;
        typedef std::basic_string<char_t> string_t;
        typedef hamigaki::line_counting_iterator<const char_t*> iter_t;
        typedef typename common_tree_match_policy_::tree_policy_t tree_policy_t;

        const char_t* s = val.c_str();
        std::size_t n = val.size();

        return boost::spirit::tree_match<IteratorT,NodeFactoryT,string_t>(
            length, val,
            tree_policy_t::create_node(length,
                iter_t(s, first.line()),
                iter_t(s+n, last.line()),
                true
            )
        );
    }
};


template<
    class NodeFactoryT, class IteratorT, class ParserT, 
    class SkipT
>
inline boost::spirit::tree_parse_info<IteratorT, NodeFactoryT>
tree_parse(
    const IteratorT& first0,
    const IteratorT& last,
    const boost::spirit::parser<ParserT>& p,
    const SkipT& skip,
    const NodeFactoryT& /*dummy*/ = NodeFactoryT())
{
    typedef boost::spirit::skip_parser_iteration_policy<SkipT> iter_policy_t;
    typedef tree_match_policy<IteratorT, NodeFactoryT> tree_match_policy_t;

    typedef boost::spirit::scanner_policies<
        iter_policy_t,
        tree_match_policy_t
    > scanner_policies_t;

    typedef boost::spirit::scanner<IteratorT, scanner_policies_t> scanner_t;

    iter_policy_t iter_policy(skip);
    scanner_policies_t policies(iter_policy);
    IteratorT first = first0;
    scanner_t scan(first, last, policies);

    boost::spirit::tree_match<
        IteratorT, NodeFactoryT
    > hit = p.derived().parse(scan);

    return boost::spirit::tree_parse_info<IteratorT, NodeFactoryT>(
        first, hit, hit && (first == last), hit.length(), hit.trees);
}

template<class IteratorT, class ParserT, class SkipT>
inline boost::spirit::tree_parse_info<
    IteratorT,
    hamigaki::bjam2::node_val_data_factory<>
>
tree_parse(
    const IteratorT& first,
    const IteratorT& last,
    const boost::spirit::parser<ParserT>& p,
    const SkipT& skip)
{
    typedef hamigaki::bjam2::node_val_data_factory<> default_node_factory_t;

    return hamigaki::bjam2::tree_parse(
        first, last, p, skip, default_node_factory_t());
}

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_PARSE_TREE_HPP
