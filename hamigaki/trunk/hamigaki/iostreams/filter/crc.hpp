// crc.hpp: CRC filter

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_CRC_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_CRC_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/pipeline.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/crc.hpp>

namespace hamigaki { namespace iostreams {

template<class Crc>
class crc_filter
{
public:
    typedef char char_type;

    typedef Crc crc_type;
    typedef typename Crc::value_type value_type;

    struct category
        : boost::iostreams::dual_use
        , boost::iostreams::filter_tag
        , boost::iostreams::multichar_tag
        , boost::iostreams::optimally_buffered_tag
    {};

    crc_filter() {}

    explicit crc_filter(const Crc& crc) : crc_(crc) {}

    value_type checksum() const
    {
        return crc_.checksum();
    }

    std::streamsize optimal_buffer_size() const
    {
        return 0;
    }

    template<typename Source>
    std::streamsize read(Source& src, char* s, std::streamsize n)
    {
        std::streamsize result = boost::iostreams::read(src, s, n);
        if (result > 0)
            crc_.process_bytes(s, result);
        return result;
    }

    template<typename Sink>
    std::streamsize write(Sink& snk, const char* s, std::streamsize n)
    {
        std::streamsize result = boost::iostreams::write(snk, s, n);
        crc_.process_bytes(s, result);
        return result;
    }

private:
    Crc crc_;
};
BOOST_IOSTREAMS_PIPABLE(crc_filter, 1)

typedef crc_filter<boost::crc_16_type> crc_16_filter;
typedef crc_filter<boost::crc_ccitt_type> crc_ccitt_filter;
typedef crc_filter<boost::crc_xmodem_type> crc_xmodem_filter;
typedef crc_filter<boost::crc_32_type> crc_32_filter;

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_CRC_HPP
