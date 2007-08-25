// svr4_header.hpp: cpio header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_CPIO_SVR4_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_CPIO_SVR4_HEADER_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>

namespace hamigaki { namespace archivers { namespace cpio {

struct svr4_header
{
    char magic[6];
    char ino[8];
    char mode[8];
    char uid[8];
    char gid[8];
    char nlink[8];
    char mtime[8];
    char filesize[8];
    char dev_major[8];
    char dev_minor[8];
    char rdev_major[8];
    char rdev_minor[8];
    char namesize[8];
    char checksum[8];
};

} } } // End namespaces cpio, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::cpio::svr4_header>
{
private:
    typedef archivers::cpio::svr4_header self;

public:
    typedef boost::mpl::list<
        member<self, char[6], &self::magic>,
        member<self, char[8], &self::ino>,
        member<self, char[8], &self::mode>,
        member<self, char[8], &self::uid>,
        member<self, char[8], &self::gid>,
        member<self, char[8], &self::nlink>,
        member<self, char[8], &self::mtime>,
        member<self, char[8], &self::filesize>,
        member<self, char[8], &self::dev_major>,
        member<self, char[8], &self::dev_minor>,
        member<self, char[8], &self::rdev_major>,
        member<self, char[8], &self::rdev_minor>,
        member<self, char[8], &self::namesize>,
        member<self, char[8], &self::checksum>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_CPIO_SVR4_HEADER_HPP
