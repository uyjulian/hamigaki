//  parser.hpp: IEEE P1281 System Use Field parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_SUSP_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_SUSP_PARSER_HPP

#include <hamigaki/archivers/iso/system_use_entry_header.hpp>
#include <hamigaki/binary/binary_io.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/transform_view.hpp>

namespace hamigaki { namespace archivers { namespace susp {

template<class T>
struct parser_field
{
    T field;
};

template<class T>
inline T& field(parser_field<T>& t)
{
    return t.field;
}

template<class State, class Header>
class call_parser
{
public:
    call_parser(State& state, const char* signature, bool& result,
            Header& head, const char* data, std::size_t size)
        : state_(state), signature_(signature), result_(result)
        , head_(head), data_(data), size_(size)
    {
    }

    template<class Signature, class Parser>
    void operator()(const boost::mpl::pair<Signature, Parser>&) const
    {
        if ((Signature::first_value  == signature_[0]) &&
            (Signature::second_value == signature_[1]) )
        {
            result_ = field<Parser>(state_)(head_, data_, size_);
        }
    }

private:
    State& state_;
    const char* signature_;
    bool& result_;
    Header& head_;
    const char* data_;
    const std::size_t size_;
};

template<class Map>
struct parser
{
private:
    typedef boost::mpl::transform_view<
        Map,
        boost::mpl::second<boost::mpl::_>
    > values_type;

    typedef typename boost::mpl::inherit_linearly<
        values_type,
        boost::mpl::inherit<
            boost::mpl::_1,
            parser_field<boost::mpl::_2>
        >
    >::type state_type;

public:
    template<class Header>
    bool operator()(Header& head,
        const char* signature, const char* data, std::size_t size)
    {
        bool result = true;
        call_parser<state_type,Header> f(
            state_, signature, result, head, data, size);
        boost::mpl::for_each<Map>(f);
        return result;
    }

private:
    state_type state_;
};

template<class Map, class Header>
inline bool parse_system_use_field(Header& head)
{
    susp::parser<Map> parser;

    const std::string& field = head.system_use;
    std::size_t field_size = field.size();
    const std::size_t head_size =
        hamigaki::struct_size<iso::system_use_entry_header>::value;

    std::size_t pos = 0;
    while (pos + head_size < field_size)
    {
        const char* s = field.c_str()+pos;

        iso::system_use_entry_header h;
        hamigaki::binary_read(s, h);

        if ((h.entry_size < head_size) || (pos + h.entry_size > field_size))
            break;

        if (!parser(head, h.signature, s+head_size, h.entry_size-head_size))
            return false;

        pos += h.entry_size;
    }

    return true;
}

} } } // End namespaces susp, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_SUSP_PARSER_HPP
