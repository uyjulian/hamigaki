// iso9660_directory_parser.hpp: ISO 9660 directory parser

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_DIRECTORY_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_DIRECTORY_PARSER_HPP

#include <hamigaki/archivers/detail/iso_directory_parser.hpp>
#include <hamigaki/archivers/detail/iso_string.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Path>
class iso9660_directory_parser : public iso_directory_parser<Path>
{
private:
    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;
    typedef typename Path::string_type string_type;
    typedef typename Path::value_type char_type;

    void do_fix_records(std::vector<iso_directory_record>& records) // virtual
    {
    }

    header_type do_make_header(const iso_directory_record& rec) // virtual
    {
        header_type h;
        if (rec.file_id.size() == 1)
        {
            if (rec.file_id[0] == '\x00')
                h.path = iso_path_traits<char_type>::current_directory();
            else if (rec.file_id[0] == '\x01')
                h.path = iso_path_traits<char_type>::parent_directory();
            else
                h.path = detail::from_iso9660_string<string_type>(rec.file_id);
        }
        else
            h.path = detail::from_iso9660_string<string_type>(rec.file_id);

        h.data_pos = rec.data_pos;
        h.file_size = rec.data_size;
        h.recorded_time = rec.recorded_time;
        h.flags = rec.flags;
        h.system_use = rec.system_use;

        return h;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_DIRECTORY_PARSER_HPP
