//  tgz_file.hpp: tar.gz file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_TGZ_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_TGZ_FILE_HPP

#include <hamigaki/iostreams/device/tar_file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace hamigaki { namespace iostreams {

template<class Source>
class basic_tgz_file_source
    : public basic_tar_file_source<
        boost::iostreams::composite<
            boost::iostreams::gzip_decompressor,
            Source
        >
    >
{
private:
    typedef boost::iostreams::composite<
        boost::iostreams::gzip_decompressor,
        Source
    > source_type;

    typedef basic_tar_file_source<source_type> base_type;

public:
    explicit basic_tgz_file_source(const Source& src)
        : base_type(source_type(boost::iostreams::gzip_decompressor(), src))
    {
    }
};

class tgz_file_source : public basic_tgz_file_source<file_source>
{
private:
    typedef basic_tgz_file_source<file_source> base_type;

public:
    explicit tgz_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};


template<class Sink>
class basic_tgz_file_sink
    : public basic_tar_file_sink<
        boost::iostreams::composite<
            boost::iostreams::gzip_compressor,
            Sink
        >
    >
{
private:
    typedef boost::iostreams::composite<
        boost::iostreams::gzip_compressor,
        Sink
    > sink_type;

    typedef basic_tar_file_sink<sink_type> base_type;

public:
    explicit basic_tgz_file_sink(const Sink& sink)
        : base_type(sink_type(boost::iostreams::gzip_compressor(), sink))
    {
    }
};

class tgz_file_sink : public basic_tgz_file_sink<file_sink>
{
private:
    typedef basic_tgz_file_sink<file_sink> base_type;

public:
    explicit tgz_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_TGZ_FILE_HPP
