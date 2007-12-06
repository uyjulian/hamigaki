// stage_map_load.cpp: load stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "stage_map_load.hpp"
#include "guid_io.hpp"
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/compose.hpp>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;

void load_map_from_binary(const char* filename, stage_map& m)
{
    io::composite<io::zlib_decompressor,io_ex::file_source> in(
        io::zlib_decompressor(),
        io_ex::file_source(filename, std::ios_base::binary)
    );

    int width = io_ex::read_int32<hamigaki::little>(in);
    int height = io_ex::read_int32<hamigaki::little>(in);

    map_elements tmp;

    boost::int32_t x;
    boost::int32_t y;
    ::GUID type;
    while (io_ex::binary_read(in, x, std::nothrow))
    {
        y = io_ex::read_int32<hamigaki::little>(in);
        io_ex::binary_read(in, type);
        tmp[std::make_pair(x, y)] =  hamigaki::uuid(type);
    }

    in.close();
    m.width = width;
    m.height = height;
    m.elements.swap(tmp);
}
