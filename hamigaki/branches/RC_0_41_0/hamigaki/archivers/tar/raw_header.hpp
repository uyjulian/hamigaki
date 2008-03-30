// raw_header.hpp: tar header

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TAR_RAW_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_TAR_RAW_HEADER_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace tar {

struct raw_header
{
    static const std::size_t block_size = 512;
    static const std::size_t name_size = 100;
    static const std::size_t prefix_size = 155;

    static const boost::intmax_t max_uid = 07777777;
    static const boost::intmax_t max_gid = 07777777;
    static const boost::uintmax_t max_size = 077777777777ull;

    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[prefix_size];

    static boost::uint64_t round_up_block_size(boost::uint64_t size)
    {
        return ((size + block_size - 1) / block_size) * block_size;
    }
};

} } } // End namespaces tar, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::tar::raw_header>
{
private:
    typedef archivers::tar::raw_header self;

public:
    typedef boost::mpl::list<
        member<self, char[100], &self::name>,
        member<self, char[8], &self::mode>,
        member<self, char[8], &self::uid>,
        member<self, char[8], &self::gid>,
        member<self, char[12], &self::size>,
        member<self, char[12], &self::mtime>,
        member<self, char[8], &self::chksum>,
        member<self, char, &self::typeflag>,
        member<self, char[100], &self::linkname>,
        member<self, char[6], &self::magic>,
        member<self, char[2], &self::version>,
        member<self, char[32], &self::uname>,
        member<self, char[32], &self::gname>,
        member<self, char[8], &self::devmajor>,
        member<self, char[8], &self::devminor>,
        member<self, char[155], &self::prefix>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_TAR_RAW_HEADER_HPP
