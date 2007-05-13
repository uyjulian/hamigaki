// reparse_point.cpp: the types for the reparse points

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_DETAIL_REPARSE_POINT_HPP
#define HAMIGAKI_FILESYSTEM_DETAIL_REPARSE_POINT_HPP

#include <hamigaki/binary/binary_io.hpp>
#include <boost/mpl/list.hpp>

namespace hamigaki { namespace filesystem { namespace detail {

struct symlink_flags
{
    static const boost::uint32_t relative = 0x00000001;
};


struct reparse_data_header
{
    boost::uint32_t tag;
    boost::uint16_t length;
    boost::uint16_t reserved;
};

struct mount_point_header
{
    static const boost::uint32_t tag = 0xA0000003;

    boost::uint16_t sub_name_offset;
    boost::uint16_t sub_name_length;
    boost::uint16_t print_name_offset;
    boost::uint16_t print_name_length;
};

struct symlink_header
{
    static const boost::uint32_t tag = 0xA000000C;

    boost::uint16_t sub_name_offset;
    boost::uint16_t sub_name_length;
    boost::uint16_t print_name_offset;
    boost::uint16_t print_name_length;
    boost::uint32_t flags;
};

} } } // End namespaces detail, filesystem, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<filesystem::detail::reparse_data_header>
{
private:
    typedef filesystem::detail::reparse_data_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::tag, native>,
        member<self, boost::uint16_t, &self::length, native>,
        member<self, boost::uint16_t, &self::reserved, native>
    > members;
};

template<>
struct struct_traits<filesystem::detail::mount_point_header>
{
private:
    typedef filesystem::detail::mount_point_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::sub_name_offset, native>,
        member<self, boost::uint16_t, &self::sub_name_length, native>,
        member<self, boost::uint16_t, &self::print_name_offset, native>,
        member<self, boost::uint16_t, &self::print_name_length, native>
    > members;
};

template<>
struct struct_traits<filesystem::detail::symlink_header>
{
private:
    typedef filesystem::detail::symlink_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::sub_name_offset, native>,
        member<self, boost::uint16_t, &self::sub_name_length, native>,
        member<self, boost::uint16_t, &self::print_name_offset, native>,
        member<self, boost::uint16_t, &self::print_name_length, native>,
        member<self, boost::uint32_t, &self::flags, native>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_FILESYSTEM_DETAIL_REPARSE_POINT_HPP
