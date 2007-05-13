// file_header.hpp: ZIP file header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_FILE_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_FILE_HEADER_HPP

#include <hamigaki/archivers/msdos/date_time.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct file_header
{
    static const boost::uint32_t signature = 0x02014B50;

    boost::uint16_t made_by;
    boost::uint16_t needed_to_extract;
    boost::uint16_t flags;
    boost::uint16_t method;
    msdos::date_time update_date_time;
    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::uint16_t file_name_length;
    boost::uint16_t extra_field_length;
    boost::uint16_t comment_length;
    boost::uint16_t disk_number_start;
    boost::uint16_t internal_attributes;
    boost::uint32_t external_attributes;
    boost::uint32_t offset;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::file_header>
{
private:
    typedef archivers::zip::file_header self;
    typedef archivers::msdos::date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::made_by, little>,
        member<self, boost::uint16_t, &self::needed_to_extract, little>,
        member<self, boost::uint16_t, &self::flags, little>,
        member<self, boost::uint16_t, &self::method, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint32_t, &self::crc32_checksum, little>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::uint16_t, &self::file_name_length, little>,
        member<self, boost::uint16_t, &self::extra_field_length, little>,
        member<self, boost::uint16_t, &self::comment_length, little>,
        member<self, boost::uint16_t, &self::disk_number_start, little>,
        member<self, boost::uint16_t, &self::internal_attributes, little>,
        member<self, boost::uint32_t, &self::external_attributes, little>,
        member<self, boost::uint32_t, &self::offset, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_FILE_HEADER_HPP
