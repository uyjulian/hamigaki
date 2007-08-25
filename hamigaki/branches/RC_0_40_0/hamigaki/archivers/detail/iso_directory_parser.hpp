// iso_directory_parser.hpp: ISO 9660 directory parser base class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_PARSER_HPP

#include <hamigaki/archivers/detail/iso_directory_record.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

class iso_directory_parser : private boost::noncopyable
{
public:
    virtual ~iso_directory_parser(){}

    void fix_records(std::vector<iso_directory_record>& records)
    {
        this->do_fix_records(records);
    }

    iso::header make_header(const iso_directory_record& rec)
    {
        return this->do_make_header(rec);
    }

private:
    virtual void do_fix_records(std::vector<iso_directory_record>& records) = 0;
    virtual iso::header do_make_header(const iso_directory_record& rec) = 0;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_PARSER_HPP
