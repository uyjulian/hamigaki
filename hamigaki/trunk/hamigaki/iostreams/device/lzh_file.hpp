//  lzh_file.hpp: LZH file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP

#include <hamigaki/iostreams/device/raw_lzh_file.hpp>
#include <hamigaki/iostreams/filter/lzhuf.hpp>
#include <boost/ref.hpp>
#include <memory>

namespace hamigaki { namespace iostreams {

template<class Source>
class basic_lzh_file_source_impl
{
private:
    typedef basic_raw_lzh_file_source_impl<Source> raw_type;

public:
    explicit basic_lzh_file_source_impl(const Source& src)
        : raw_(src), pos_(0)
    {
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        const lha::header& head = raw_.header();
        if (head.method == "-lhd-")
            lzhuf_ptr_.reset();
        else if (head.method == "-lh0-")
            lzhuf_ptr_.reset();
        else if (head.method == "-lh4-")
            lzhuf_ptr_.reset(new lzhuf_decompressor(12));
        else if (head.method == "-lh5-")
            lzhuf_ptr_.reset(new lzhuf_decompressor(13));
        else if (head.method == "-lh6-")
            lzhuf_ptr_.reset(new lzhuf_decompressor(15));
        else if (head.method == "-lh7-")
            lzhuf_ptr_.reset(new lzhuf_decompressor(16));
        else
            throw std::runtime_error("unsupported LZH method");

        pos_ = 0;
        crc_.reset();
        return true;
    }

    lha::header header() const
    {
        return raw_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        boost::int64_t file_size = header().file_size;
        if ((pos_ >= file_size) || (n <= 0))
            return -1;

        boost::int64_t rest = file_size - pos_;
        std::streamsize amt =
            static_cast<std::streamsize>(
                (std::min)(static_cast<boost::int64_t>(n), rest));

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
    std::auto_ptr<lzhuf_decompressor> lzhuf_ptr_;

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (lzhuf_ptr_.get())
            return boost::iostreams::read(*lzhuf_ptr_, boost::ref(raw_), s, n);
        else
            return raw_.read(s, n);
    }
};


template<class Source>
class basic_lzh_file_source
{
private:
    typedef basic_lzh_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    typedef lha::header header_type;

    explicit basic_lzh_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    lha::header header() const
    {
        return pimpl_->header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class lzh_file_source : public basic_lzh_file_source<file_source>
{
    typedef basic_lzh_file_source<file_source> base_type;

public:
    explicit lzh_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};


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
        if (header.is_directory() || header.is_symbolic_link())
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
            lzhuf_ptr_.reset(new lzhuf_compressor(12));
        else if (header.method == "-lh5-")
            lzhuf_ptr_.reset(new lzhuf_compressor(13));
        else if (header.method == "-lh6-")
            lzhuf_ptr_.reset(new lzhuf_compressor(15));
        else if (header.method == "-lh7-")
            lzhuf_ptr_.reset(new lzhuf_compressor(16));
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
    std::auto_ptr<lzhuf_compressor> lzhuf_ptr_;

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

template<class Sink>
class basic_lzh_file_sink
{
private:
    typedef basic_lzh_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef lha::header header_type;

    explicit basic_lzh_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void default_method(const char* method)
    {
        pimpl_->default_method(method);
    }

    void create_entry(const lha::header& head)
    {
        pimpl_->create_entry(head);
    }

    void rewind_entry()
    {
        pimpl_->rewind_entry();
    }

    void close()
    {
        pimpl_->close();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void close_archive()
    {
        pimpl_->close_archive();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class lzh_file_sink : public basic_lzh_file_sink<file_sink>
{
    typedef basic_lzh_file_sink<file_sink> base_type;

public:
    explicit lzh_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
