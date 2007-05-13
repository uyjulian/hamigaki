// raw_header.hpp: cpio header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_CPIO_RAW_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_CPIO_RAW_HEADER_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>

namespace hamigaki { namespace archivers { namespace cpio {

struct raw_header
{
    char magic[6];
    char dev[6];
    char ino[6];
    char mode[6];
    char uid[6];
    char gid[6];
    char nlink[6];
    char rdev[6];
    char mtime[11];
    char namesize[6];
    char filesize[11];
};

} } } // End namespaces cpio, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::cpio::raw_header>
{
private:
    typedef archivers::cpio::raw_header self;

public:
    typedef boost::mpl::list<
        member<self, char[6], &self::magic>,
        member<self, char[6], &self::dev>,
        member<self, char[6], &self::ino>,
        member<self, char[6], &self::mode>,
        member<self, char[6], &self::uid>,
        member<self, char[6], &self::gid>,
        member<self, char[6], &self::nlink>,
        member<self, char[6], &self::rdev>,
        member<self, char[11], &self::mtime>,
        member<self, char[6], &self::namesize>,
        member<self, char[11], &self::filesize>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_CPIO_RAW_HEADER_HPP
