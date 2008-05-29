// node_val_data.hpp: node_val_data for bjam

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_NODE_VAL_DATA_HPP
#define HAMIGAKI_BJAM2_UTIL_NODE_VAL_DATA_HPP

#include <hamigaki/iterator/line_counting_iterator.hpp>
#include <boost/spirit/tree/common.hpp>

namespace hamigaki { namespace bjam2 {

template<class CharT = char, class ValueT = boost::spirit::nil_t>
struct node_val_data
{
    typedef CharT value_type;
    typedef std::allocator<CharT> allocator_type;

    typedef std::basic_string<
        CharT, std::char_traits<CharT>, allocator_type
    > container_t;

    typedef typename container_t::iterator iterator_t;
    typedef typename container_t::const_iterator const_iterator_t;

    node_val_data()
        : text_(), is_root_(false), parser_id_(), value_(), line_()
    {
    }

    template<class IteratorT>
    node_val_data(const IteratorT& first, const IteratorT& last)
        : text_(first, last), is_root_(false), parser_id_(), value_(), line_()
    {
    }

    template<class IteratorT>
    node_val_data(
        const hamigaki::line_counting_iterator<IteratorT>& first,
        const hamigaki::line_counting_iterator<IteratorT>& last
    )
        : text_(first.base(), last.base())
        , is_root_(false), parser_id_(), value_(), line_(first.line())
    {
    }

    void swap(node_val_data& x)
    {
        boost::spirit::impl::cp_swap(text_, x.text_);
        boost::spirit::impl::cp_swap(is_root_, x.is_root_);
        boost::spirit::impl::cp_swap(parser_id_, x.parser_id_);
        boost::spirit::impl::cp_swap(value_, x.value_);
        boost::spirit::impl::cp_swap(line_, x.line_);
    }

    typename container_t::iterator begin()
    {
        return text_.begin();
    }

    typename container_t::const_iterator begin() const
    {
        return text_.begin();
    }

    typename container_t::iterator end()
    {
        return text_.end();
    }

    typename container_t::const_iterator end() const
    {
        return text_.end();
    }

    bool is_root() const
    {
        return is_root_;
    }

    void is_root(bool b)
    {
        is_root_ = b;
    }

    boost::spirit::parser_id id() const
    {
        return parser_id_;
    }

    void id(const boost::spirit::parser_id& id)
    {
        parser_id_ = id;
    }

    ValueT const& value() const
    {
        return value_;
    }

    void value(const ValueT& v)
    {
        value_ = v;
    }

    int line() const
    {
        return line_;
    }

    void line(int n)
    {
        line_ = n;
    }

private:
    container_t text_;
    bool is_root_;
    boost::spirit::parser_id parser_id_;
    ValueT value_;
    int line_;
};

typedef boost::spirit::tree_node<
    hamigaki::bjam2::node_val_data<>
> tree_node;


template<class ValueT = boost::spirit::nil_t>
class node_val_data_factory
{
public:
    template<class IteratorT>
    class factory
    {
    public:
        typedef IteratorT iterator_t;

        typedef node_val_data<
            typename std::iterator_traits<IteratorT>::value_type,
            ValueT
        > node_t;

        template<class IteratorT2>
        static node_t create_node(
            const IteratorT2& first, const IteratorT2& last, bool is_leaf_node)
        {
            if (is_leaf_node)
                return node_t(first, last);
            else
                return node_t();
        }

        static node_t empty_node()
        {
            return node_t();
        }

        template<class ContainerT>
        static node_t group_nodes(const ContainerT& nodes)
        {
            typename node_t::container_t c;
            typedef typename ContainerT::const_iterator iter_t;

            for (iter_t i = nodes.begin(), end = nodes.end(); i != end; ++i)
            {
                assert(i->children.size() == 0);
                c.insert(c.end(), i->value.begin(), i->value.end());
            }

            return node_t(c.begin(), c.end());
        }
    };
};

template<
    class IteratorT = const char*,
    class NodeFactoryT = node_val_data_factory<boost::spirit::nil_t>,
    class T = boost::spirit::nil_t
>
struct tree_parse_info
{
    IteratorT stop;
    bool match;
    bool full;
    std::size_t length;
    int line;

    typename boost::spirit::tree_match<
        IteratorT,
        NodeFactoryT,
        T
    >::container_t trees;

    tree_parse_info()
        : stop() , match(false), full(false), length(0), line(0), trees()
    {
    }

    template<class IteratorT2>
    tree_parse_info(const tree_parse_info<IteratorT2>& pi)
        : stop(pi.stop), match(pi.match), full(pi.full)
        , length(pi.length), line(pi.line), trees()
    {
        trees.swap(pi.trees);
    }

    tree_parse_info(
        IteratorT stop,
        bool match,
        bool full,
        std::size_t length,
        int line,
        typename boost::spirit::tree_match<
            IteratorT,
            NodeFactoryT,
            T
        >::container_t trees
    )
        : stop(stop), match(match), full(full)
        , length(length), line(line), trees()
    {
        this->trees.swap(trees);
    }
};

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_NODE_VAL_DATA_HPP
