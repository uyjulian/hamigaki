// binary_header.hpp: cpio binary header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_CPIO_BINARY_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_CPIO_BINARY_HEADER_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace cpio {

struct binary_header
{
    boost::uint16_t magic;
    boost::uint16_t dev;
    boost::uint16_t ino;
    boost::uint16_t mode;
    boost::uint16_t uid;
    boost::uint16_t gid;
    boost::uint16_t nlink;
    boost::uint16_t rdev;
    boost::uint16_t mtime[2];
    boost::uint16_t namesize;
    boost::uint16_t filesize[2];
};

} } } // End namespaces cpio, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::cpio::binary_header>
{
private:
    typedef archivers::cpio::binary_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::magic, native>,
        member<self, boost::uint16_t, &self::dev, native>,
        member<self, boost::uint16_t, &self::ino, native>,
        member<self, boost::uint16_t, &self::mode, native>,
        member<self, boost::uint16_t, &self::uid, native>,
        member<self, boost::uint16_t, &self::gid, native>,
        member<self, boost::uint16_t, &self::nlink, native>,
        member<self, boost::uint16_t, &self::rdev, native>,
        member<self, boost::uint16_t[2], &self::mtime, native>,
        member<self, boost::uint16_t, &self::namesize, native>,
        member<self, boost::uint16_t[2], &self::filesize, native>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_CPIO_BINARY_HEADER_HPP
