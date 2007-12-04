// guid_io.hpp: binary I/O mapping for GUID

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GUID_IO_HPP
#define GUID_IO_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <rpc.h>

namespace hamigaki
{

template<>
struct struct_traits< ::GUID>
{
private:
    typedef ::GUID self;

public:
    typedef boost::mpl::list<
        member<self, unsigned long,  &self::Data1, little>,
        member<self, unsigned short, &self::Data2, little>,
        member<self, unsigned short, &self::Data3, little>,
        member<self, unsigned char[8],  &self::Data4, little>
    > members;
};

} // namespace hamigaki

#endif // GUID_IO_HPP
