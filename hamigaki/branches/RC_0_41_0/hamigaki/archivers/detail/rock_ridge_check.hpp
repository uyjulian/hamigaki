// rock_ridge_check.hpp: IEEE P1282 Rock Ridge checker

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_CHECKER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_CHECKER_HPP

#include <hamigaki/archivers/iso/rrip_type.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

inline iso::rrip_type rock_ridge_check(const std::string& su)
{
    if (su.empty())
        return iso::rrip_none;

    const std::size_t head_size =
        hamigaki::struct_size<iso::system_use_entry_header>::value;

    const std::size_t er_size =
        head_size +
        hamigaki::struct_size<iso::er_system_use_entry_data>::value;

    std::size_t pos = 0;
    while (pos + head_size <= su.size())
    {
        iso::system_use_entry_header head;
        hamigaki::binary_read(su.c_str()+pos, head);
        if (std::memcmp(head.signature, "ER", 2) == 0)
        {
            if ((head.entry_size >= er_size) &&
                (pos + head.entry_size <= su.size()) )
            {
                iso::er_system_use_entry_data er;
                hamigaki::binary_read(su.c_str()+pos+head_size, er);

                if (er.id_size == 10)
                {
                    const char* s = su.c_str()+pos+er_size;
                    if (std::memcmp(s, "RRIP_1991A", 10) == 0)
                        return iso::rrip_1991a;
                    else if (std::memcmp(s, "IEEE_P1282", 10) == 0)
                        return iso::ieee_p1282;
                }
            }
        }
        pos += head.entry_size;
    }

    return iso::rrip_none;
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_CHECKER_HPP
