//  joliet_directory_parser.hpp: Joliet directory parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_DIRECTORY_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_DIRECTORY_PARSER_HPP

#include <hamigaki/archivers/detail/iso_directory_parser.hpp>
#include <hamigaki/archivers/detail/ucs2.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class joliet_directory_parser : public iso_directory_parser
{
private:
    void do_fix_records(std::vector<iso_directory_record>& records) // virtual
    {
    }

    iso::header do_make_header(const iso_directory_record& rec) // virtual
    {
        using namespace boost::filesystem;

        iso::header h;
        if (rec.file_id.size() == 1)
        {
            if (rec.file_id[0] == '\0')
                h.path = ".";
            else
                h.path = "..";
        }
        else
            h.path = path(detail::ucs2be_to_narrow(rec.file_id), no_check);

        h.data_pos = rec.data_pos;
        h.file_size = rec.data_size;
        h.recorded_time = rec.recorded_time;
        h.flags = rec.flags;
        h.system_use = rec.system_use;

        return h;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_DIRECTORY_PARSER_HPP
