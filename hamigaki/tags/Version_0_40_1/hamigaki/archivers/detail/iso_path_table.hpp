// iso_path_table.hpp: ISO 9660 path table record and infomation

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_PATH_TABLE_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_PATH_TABLE_HPP

#include <hamigaki/archivers/detail/iso_directory_record.hpp>
#include <hamigaki/archivers/iso/directory_record.hpp>
#include <boost/filesystem/path.hpp>
#include <set>

namespace hamigaki { namespace archivers { namespace detail {

struct iso_path_table_record
{
    std::string dir_id;
    boost::uint32_t data_pos;
    boost::uint16_t parent_index;
    std::set<iso_directory_record> entries;
    boost::filesystem::path full_path;

    bool operator<(const iso_path_table_record& rhs) const
    {
        if (parent_index != rhs.parent_index)
            return parent_index < rhs.parent_index;
        else
            return dir_id < rhs.dir_id;
    }
};

struct iso_path_table_info
{
    iso::directory_record root_record;
    boost::uint32_t path_table_size;
    boost::uint32_t l_path_table_pos;
    boost::uint32_t m_path_table_pos;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_PATH_TABLE_HPP
