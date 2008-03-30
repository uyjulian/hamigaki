// digital_signature.hpp: ZIP digital signature

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_DIGITAL_SIGNATURE_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_DIGITAL_SIGNATURE_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct digital_signature
{
    static const boost::uint32_t signature = 0x05054B50;

    boost::uint16_t size;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::digital_signature>
{
private:
    typedef archivers::zip::digital_signature self;

public:
    typedef boost::mpl::single_view<
        member<self, boost::uint16_t, &self::size, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_DIGITAL_SIGNATURE_HPP
