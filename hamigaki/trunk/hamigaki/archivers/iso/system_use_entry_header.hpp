// system_use_entry_header.hpp: IEEE P1281 System Use Entry

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_SYSTEM_USE_ENTRY_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_ISO_SYSTEM_USE_ENTRY_HEADER_HPP

namespace hamigaki { namespace archivers { namespace iso {

struct system_use_entry_header
{
    char signature[2];
    boost::uint8_t entry_size;
    boost::uint8_t version;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::system_use_entry_header>
{
private:
    typedef archivers::iso::system_use_entry_header self;

public:
    typedef boost::mpl::list<
        member<self, char[2], &self::signature>,
        member<self, boost::uint8_t, &self::entry_size>,
        member<self, boost::uint8_t, &self::version>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_SYSTEM_USE_ENTRY_HEADER_HPP
