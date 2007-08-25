// extra_field_header.hpp: ZIP extra field header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_EXTRA_FIELD_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_EXTRA_FIELD_HEADER_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct extra_field_header
{
    boost::uint16_t id;
    boost::uint16_t size;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::extra_field_header>
{
private:
    typedef archivers::zip::extra_field_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::id, little>,
        member<self, boost::uint16_t, &self::size, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_EXTRA_FIELD_HEADER_HPP
