// timestamp.hpp: Windows file size extended header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_WINDOWS_FILE_SIZE_HPP
#define HAMIGAKI_ARCHIVERS_LHA_WINDOWS_FILE_SIZE_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace lha { namespace windows {

struct file_size
{
    boost::int64_t compressed_size;
    boost::int64_t original_size;

    file_size(boost::int64_t compressed_size, boost::int64_t original_size)
        : compressed_size(compressed_size), original_size(original_size)
    {
    }
};

} } } } // End namespaces windows, lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::windows::file_size>
{
private:
    typedef archivers::lha::windows::file_size self;

public:
    typedef boost::mpl::list<
        member<self, boost::int64_t, &self::compressed_size, little>,
        member<self, boost::int64_t, &self::original_size, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_WINDOWS_FILE_SIZE_HPP
