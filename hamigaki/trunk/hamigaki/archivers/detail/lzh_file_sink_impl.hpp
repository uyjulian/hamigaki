//  lzh_file_sink_impl.hpp: LZH file sink implementation

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_LZH_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_LZH_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/raw_lzh_file_sink_impl.hpp>
#include <hamigaki/iostreams/filter/lzhuf.hpp>
#include <boost/ref.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
class basic_lzh_file_sink_impl
{
private:
    typedef basic_raw_lzh_file_sink_impl<Sink> raw_type;

public:
    explicit basic_lzh_file_sink_impl(const Sink& sink)
        : raw_(sink), pos_(0), method_("-lh5-")
    {
    }

    void default_method(const char* method)
    {
        method_  = method;
    }

    void create_entry(const lha::header& head)
    {
        lha::header header = head;
        if (header.is_directory() || header.is_symlink())
            header.method = "-lhd-";
        else if ((header.file_size != -1) && (header.file_size < 3))
            header.method = "-lh0-";
        else if (header.method.empty())
            header.method = method_;

        raw_.create_entry(header);

        if (header.method == "-lhd-")
            lzhuf_ptr_.reset();
        else if (header.method == "-lh0-")
            lzhuf_ptr_.reset();
        else if (header.method == "-lh4-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_compressor(12));
        else if (header.method == "-lh5-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_compressor(13));
        else if (header.method == "-lh6-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_compressor(15));
        else if (header.method == "-lh7-")
            lzhuf_ptr_.reset(new iostreams::lzhuf_compressor(16));
        else
            throw std::runtime_error("unsupported LZH method");

        pos_ = 0;
    }

    void rewind_entry()
    {
        raw_.rewind_entry();
        lzhuf_ptr_.reset();
        crc_.reset();
        pos_ = 0;
    }

    void close()
    {
        if (lzhuf_ptr_.get())
        {
            boost::iostreams::close(
                *lzhuf_ptr_, boost::ref(raw_), BOOST_IOS::out);
        }

        raw_.close(crc_.checksum(), pos_);
        crc_.reset();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = write_impl(s, n);
        if (amt != 0)
        {
            crc_.process_bytes(s, amt);
            pos_ += amt;
        }
        return amt;
    }

    void close_archive()
    {
        raw_.close_archive();
    }

private:
    raw_type raw_;
    boost::int64_t pos_;
    boost::crc_16_type crc_;
    lha::compress_method method_;
    std::auto_ptr<iostreams::lzhuf_compressor> lzhuf_ptr_;

    std::streamsize write_impl(const char* s, std::streamsize n)
    {
        if (lzhuf_ptr_.get())
        {
            boost::reference_wrapper<raw_type> ref(raw_);
            return boost::iostreams::write(*lzhuf_ptr_, ref, s, n);
        }
        else
            return raw_.write(s, n);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_LZH_FILE_SINK_IMPL_HPP
