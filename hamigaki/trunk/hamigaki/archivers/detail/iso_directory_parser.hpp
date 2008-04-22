// iso_directory_parser.hpp: ISO 9660 directory parser base class

// Copyright Takeshi Mouri 2007, 2008.
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

template<class CharT>
struct iso_path_traits;

template<>
struct iso_path_traits<char>
{
    static const char* current_directory()
    {
        return ".";
    }

    static const char* parent_directory()
    {
        return "..";
    }
};

template<>
struct iso_path_traits<wchar_t>
{
    static const wchar_t* current_directory()
    {
        return L".";
    }

    static const wchar_t* parent_directory()
    {
        return L"..";
    }
};

template<class Path>
class iso_directory_parser : private boost::noncopyable
{
public:
    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;

    virtual ~iso_directory_parser(){}

    void fix_records(std::vector<iso_directory_record>& records)
    {
        this->do_fix_records(records);
    }

    header_type make_header(const iso_directory_record& rec)
    {
        return this->do_make_header(rec);
    }

private:
    virtual void do_fix_records(std::vector<iso_directory_record>& records) = 0;
    virtual header_type do_make_header(const iso_directory_record& rec) = 0;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_PARSER_HPP
