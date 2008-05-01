// stage_map_load.cpp: load stage map

// Copyright Takeshi Mouri 2007, 2008.
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
#include <algorithm>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;

namespace
{

const ::GUID player_id =
{ 0xD5D26CC5, 0xD8BD, 0x40A4, {0x83,0x9E,0xE3,0xC7,0xFA,0x57,0x0F,0x66} };

bool guid_equal(const ::GUID& lhs, const ::GUID& rhs)
{
    return
        (lhs.Data1 == rhs.Data1) &&
        (lhs.Data2 == rhs.Data2) &&
        (lhs.Data3 == rhs.Data3) &&
        std::equal(lhs.Data4, lhs.Data4+8, rhs.Data4) ;
}

} // namespace

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
    std::pair<int,int> plyaer_pos;
    while (io_ex::binary_read(in, x, std::nothrow))
    {
        y = io_ex::read_int32<hamigaki::little>(in);
        io_ex::binary_read(in, type);

        if (guid_equal(type, player_id))
        {
            plyaer_pos.first = x;
            plyaer_pos.second = y;
        }

        tmp.insert(map_element(x, y, hamigaki::uuid(type)));
    }

    in.close();
    m.width = width;
    m.height = height;
    m.player_position = plyaer_pos;
    m.elements.swap(tmp);
}
