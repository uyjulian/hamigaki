//  tmp_file.hpp: temporary file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_TMP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_TMP_FILE_HPP

#include <hamigaki/iostreams/detail/config.hpp>
#include <hamigaki/iostreams/detail/auto_link.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include <boost/config/abi_prefix.hpp>

namespace hamigaki { namespace iostreams {

class HAMIGAKI_IOSTREAMS_DECL tmp_file
{
public:
    typedef char char_type;

    struct category
        : public boost::iostreams::seekable_device_tag
        , public boost::iostreams::closable_tag
    {};

    tmp_file();

    std::streamsize read(char* s, std::streamsize n);
    std::streamsize write(const char* s, std::streamsize n);

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way);

    void close();

private:
    struct impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::tmp_file, 0)

#include <boost/config/abi_suffix.hpp>

#endif // HAMIGAKI_IOSTREAMS_DEVICE_TMP_FILE_HPP
