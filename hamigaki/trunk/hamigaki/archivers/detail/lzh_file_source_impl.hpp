// lzh_file_source_impl.hpp: LZH file source implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_LZH_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_LZH_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/raw_lzh_file_source_impl.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/filter/lzhuf.hpp>
#include <boost/ref.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source, class Path>
class basic_lzh_file_source_impl
{
private:
    typedef detail::basic_raw_lzh_file_source_impl<Source,Path> raw_type;

public:
    typedef Path path_type;
    typedef lha::basic_header<Path> header_type;

    explicit basic_lzh_file_source_impl(const Source& src)
        : raw_(src), pos_(0)
    {
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        const header_type& head = raw_.header();
        if (head.method == "-lhd-")
            lzhuf_ptr_.reset();
        else if (head.method == "-lh0-")
            lzhuf_ptr_.reset();
        else if (head.method == "-lh4-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_decompressor(12));
        else if (head.method == "-lh5-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_decompressor(13));
        else if (head.method == "-lh6-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_decompressor(15));
        else if (head.method == "-lh7-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_decompressor(16));
        else
            throw std::runtime_error("unsupported LZH method");

        pos_ = 0;
        crc_.reset();
        return true;
    }

    header_type header() const
    {
        return raw_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        boost::int64_t file_size = header().file_size;
        if ((pos_ >= file_size) || (n <= 0))
            return -1;

        boost::int64_t rest = file_size - pos_;
        std::streamsize amt = auto_min(n, rest);

        std::streamsize result = read_impl(s, amt);
        if (result > 0)
            pos_ += result;

        if (header().crc16_checksum)
        {
            if (result > 0)
                crc_.process_bytes(s, result);
            if (pos_ >= file_size)
            {
                if (crc_.checksum() != header().crc16_checksum.get())
                    throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
            }
        }
        return result;
    }

private:
    raw_type raw_;
    boost::int64_t pos_;
    boost::crc_16_type crc_;
    std::auto_ptr<iostreams::lzhuf_decompressor> lzhuf_ptr_;

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (lzhuf_ptr_.get())
            return boost::iostreams::read(*lzhuf_ptr_, boost::ref(raw_), s, n);
        else
            return raw_.read(s, n);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_LZH_FILE_SOURCE_IMPL_HPP
