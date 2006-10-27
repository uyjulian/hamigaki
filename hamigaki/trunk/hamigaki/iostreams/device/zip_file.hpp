//  zip_file.hpp: Phil Katz Zip file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP

#include <hamigaki/iostreams/device/raw_zip_file.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/crc.hpp>
#include <boost/ref.hpp>

namespace hamigaki { namespace iostreams {

namespace zip
{

namespace detail
{

inline boost::iostreams::zlib_params make_zlib_params()
{
    boost::iostreams::zlib_params params;
    params.noheader = true;
    return params;
}

} // namespace detail

} // namespace zip

template<class Source>
class basic_zip_file_source_impl
{
private:
    typedef basic_raw_zip_file_source_impl<Source> raw_type;

public:
    explicit basic_zip_file_source_impl(const Source& src)
        : raw_(src)
        , zlib_(zip::detail::make_zlib_params())
    {
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        prepare_reading();
        return true;
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        raw_.select_entry(ph);
        prepare_reading();
    }

    zip::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = read_impl(s, n);
        if (amt != -1)
            crc32_.process_bytes(s, amt);
        else if (crc32_.checksum() != header_.crc32_checksum)
            throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
        return amt;
    }

private:
    raw_type raw_;
    zip::header header_;
    boost::crc_32_type crc32_;
    boost::iostreams::zlib_decompressor zlib_;

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (header_.method == 0)
            return raw_.read(s, n);
        else if (header_.method == 8)
            return boost::iostreams::read(zlib_, boost::ref(raw_), s, n);

        return -1;
    }

    void prepare_reading()
    {
        header_ = raw_.header();

        if ((header_.method != 0) && (header_.method != 8))
            throw std::runtime_error("unsupported ZIP format");

        crc32_.reset();
        boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::in);

        if ((header_.permission & 0170000) == 0120000)
        {
            using namespace boost::filesystem;

            boost::scoped_array<char> buf(new char[header_.file_size+1]);
            read(buf.get(), header_.file_size);
            buf[header_.file_size] = '\0';

            header_.link_path = path(buf.get(), no_check);
            header_.compressed_size = 0;
            header_.file_size = 0;
        }
    }
};

template<class Source>
class basic_zip_file_source
{
private:
    typedef basic_zip_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_zip_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        pimpl_->select_entry(ph);
    }

    zip::header header() const
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

class zip_file_source : public basic_zip_file_source<file_source>
{
    typedef basic_zip_file_source<file_source> base_type;

public:
    explicit zip_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};


template<class Sink>
class basic_zip_file_sink_impl
{
private:
    typedef basic_raw_zip_file_sink_impl<Sink> raw_zip_type;

public:
    explicit basic_zip_file_sink_impl(const Sink& sink)
        : raw_(sink), method_(0), size_(0)
        , zlib_(zip::detail::make_zlib_params())
    {
    }

    void create_entry(const zip::header& head)
    {
        zip::header header = head;

        const std::string link_path = header.link_path.string();

        if (!link_path.empty())
        {
            header.method = 0;
            header.file_size = link_path.size();
        }
        else if (header.file_size < 6)
            header.method = 0;

        method_ = header.method;
        raw_.create_entry(header);
        size_ = 0;

        if (!link_path.empty())
        {
            write(link_path.c_str(), link_path.size());
            close();
        }
    }

    void rewind_entry()
    {
        raw_.rewind_entry();
        method_ = 0;
        size_ = 0;
        crc32_.reset();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = write_impl(s, n);
        if (amt != 0)
        {
            crc32_.process_bytes(s, amt);
            size_ += amt;
        }
        return amt;
    }

    void close()
    {
        if (method_ == 8)
            boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::out);

        raw_.close(crc32_.checksum(), size_);
        crc32_.reset();
    }

    void write_end_mark()
    {
        raw_.write_end_mark();
    }

private:
    raw_zip_type raw_;
    boost::uint16_t method_;
    boost::crc_32_type crc32_;
    boost::uint32_t size_;
    boost::iostreams::zlib_compressor zlib_;

    std::streamsize write_impl(const char* s, std::streamsize n)
    {
        if (method_ == 0)
            return raw_.write(s, n);
        else if (method_ == 8)
            return boost::iostreams::write(zlib_, boost::ref(raw_), s, n);

        return n;
    }
};

template<class Sink>
class basic_zip_file_sink
{
private:
    typedef basic_zip_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_zip_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void create_entry(const zip::header& head)
    {
        pimpl_->create_entry(head);
    }

    void rewind_entry()
    {
        pimpl_->rewind_entry();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

    void write_end_mark()
    {
        pimpl_->write_end_mark();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class zip_file_sink : public basic_zip_file_sink<file_sink>
{
private:
    typedef basic_zip_file_sink<file_sink> base_type;

public:
    explicit zip_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP
