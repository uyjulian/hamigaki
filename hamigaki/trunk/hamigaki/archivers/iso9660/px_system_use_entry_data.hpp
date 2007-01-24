//  px_system_use_entry_data.hpp: IEEE P1282 "PX" System Use Entry

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO9660_PX_SYSTEM_USE_ENTRY_DATA_HPP
#define HAMIGAKI_ARCHIVERS_ISO9660_PX_SYSTEM_USE_ENTRY_DATA_HPP

namespace hamigaki { namespace archivers { namespace iso9660 {

struct old_px_system_use_entry_data
{
    boost::uint32_t file_mode;
    boost::uint32_t links;
    boost::uint32_t uid;
    boost::uint32_t gid;
};

struct px_system_use_entry_data
{
    boost::uint32_t file_mode;
    boost::uint32_t links;
    boost::uint32_t uid;
    boost::uint32_t gid;
    boost::uint32_t serial_no;
};

} } } // End namespaces iso9660, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso9660::old_px_system_use_entry_data>
{
private:
    typedef archivers::iso9660::old_px_system_use_entry_data self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::file_mode, little>,
        member<self, boost::uint32_t, &self::file_mode, big>,
        member<self, boost::uint32_t, &self::links, little>,
        member<self, boost::uint32_t, &self::links, big>,
        member<self, boost::uint32_t, &self::uid, little>,
        member<self, boost::uint32_t, &self::uid, big>,
        member<self, boost::uint32_t, &self::gid, little>,
        member<self, boost::uint32_t, &self::gid, big>
    > members;
};

template<>
struct struct_traits<archivers::iso9660::px_system_use_entry_data>
{
private:
    typedef archivers::iso9660::px_system_use_entry_data self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::file_mode, little>,
        member<self, boost::uint32_t, &self::file_mode, big>,
        member<self, boost::uint32_t, &self::links, little>,
        member<self, boost::uint32_t, &self::links, big>,
        member<self, boost::uint32_t, &self::uid, little>,
        member<self, boost::uint32_t, &self::uid, big>,
        member<self, boost::uint32_t, &self::gid, little>,
        member<self, boost::uint32_t, &self::gid, big>,
        member<self, boost::uint32_t, &self::serial_no, little>,
        member<self, boost::uint32_t, &self::serial_no, big>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO9660_PX_SYSTEM_USE_ENTRY_DATA_HPP
