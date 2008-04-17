// raw_lzh_file_source_impl.hpp: raw LZH file source implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/lzh_header_parser.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/skip.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source, class Path>
class basic_raw_lzh_file_source_impl
{
public:
    typedef char char_type;
    typedef Path path_type;
    typedef typename Path::string_type string_type;
    typedef lha::basic_header<Path> header_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_raw_lzh_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        header_.compressed_size = 0;
    }

    bool next_entry()
    {
        if (boost::int64_t rest = header_.compressed_size - pos_)
            iostreams::skip(src_, rest);
        pos_ = 0;

        lzh_detail::lzh_header_parser<Source,Path> parser(src_);
        if (!parser.parse())
            return false;

        header_ = parser.header();
        return true;
    }

    header_type header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((pos_ >= header_.compressed_size) || (n <= 0))
            return -1;

        boost::int64_t rest = header_.compressed_size - pos_;
        std::streamsize amt = auto_min(n, rest);

        iostreams::blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

private:
    Source src_;
    header_type header_;
    boost::int64_t pos_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SOURCE_IMPL_HPP
