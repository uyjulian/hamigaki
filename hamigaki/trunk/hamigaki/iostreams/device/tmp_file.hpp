// tmp_file.hpp: temporary file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_TMP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_TMP_FILE_HPP

#include <hamigaki/iostreams/detail/config.hpp>
#include <hamigaki/iostreams/detail/auto_link.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

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
    explicit tmp_file(boost::uint32_t seed);

    std::streamsize read(char* s, std::streamsize n);
    std::streamsize write(const char* s, std::streamsize n);

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way);

    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::tmp_file, 0)

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_IOSTREAMS_DEVICE_TMP_FILE_HPP
