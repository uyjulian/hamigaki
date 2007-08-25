// iso_directory_record.hpp: ISO 9660 directory record

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_RECORD_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_RECORD_HPP

#include <hamigaki/archivers/iso/date_time.hpp>
#include <hamigaki/archivers/iso/file_flags.hpp>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

inline int iso9660_id_compare(const std::string& lhs, const std::string& rhs)
{
    bool lhs_current = (lhs.size() == 1) && (lhs[0] == '\x00');
    bool rhs_current = (rhs.size() == 1) && (rhs[0] == '\x00');

    if (lhs_current && !rhs_current)
        return -1;
    else if (lhs_current && rhs_current)
        return 0;
    else if (!lhs_current && rhs_current)
        return 1;

    bool lhs_parent = (lhs.size() == 1) && (lhs[0] == '\x01');
    bool rhs_parent = (rhs.size() == 1) && (rhs[0] == '\x01');

    if (lhs_parent && !rhs_parent)
        return -1;
    else if (lhs_parent && rhs_parent)
        return 0;
    else if (!lhs_parent && rhs_parent)
        return 1;

    return lhs.compare(rhs);
}

struct iso_directory_record : boost::totally_ordered<iso_directory_record>
{
    boost::uint32_t data_pos;
    boost::uint32_t data_size;
    iso::binary_date_time recorded_time;
    boost::uint8_t flags;
    std::string file_id;
    boost::uint16_t version;
    std::string system_use;

    iso_directory_record() : data_pos(0), data_size(0), flags(0), version(0)
    {
    }

    bool is_directory() const
    {
        return (flags & iso::file_flags::directory) != 0;
    }

    int compare(const iso_directory_record& rhs) const
    {
        if (int cmp = detail::iso9660_id_compare(file_id, rhs.file_id))
            return cmp;

        // descending order
        if (version > rhs.version)
            return -1;
        else if (version < rhs.version)
            return 1;

        bool lhs_assoc = (flags & iso::file_flags::associated) != 0;
        bool rhs_assoc = (rhs.flags & iso::file_flags::associated) != 0;

        if (lhs_assoc && !rhs_assoc)
            return -1;
        else if (!lhs_assoc && rhs_assoc)
            return 1;

        if (data_pos < rhs.data_pos)
            return -1;
        else if (data_pos > rhs.data_pos)
            return 1;
        else
            return 0;
    }

    bool operator==(const iso_directory_record& rhs) const
    {
        return compare(rhs) == 0;
    }

    bool operator<(const iso_directory_record& rhs) const
    {
        return compare(rhs) < 0;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_RECORD_HPP
