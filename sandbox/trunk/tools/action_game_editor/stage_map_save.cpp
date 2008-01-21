// stage_map_save.cpp: save stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "stage_map_save.hpp"
#include "guid_io.hpp"
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/compose.hpp>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;

void save_map_to_binary(const char* filename, const stage_map& m)
{
    io::composite<io::zlib_compressor,io_ex::file_sink> out(
        io::zlib_compressor(),
        io_ex::file_sink(filename, std::ios_base::binary)
    );

    int width = m.width;
    int height = m.height;

    io_ex::write_int32<hamigaki::little>(out, width);
    io_ex::write_int32<hamigaki::little>(out, height);

    ::GUID buf;
    typedef map_elements::const_iterator iter_type;
    for (iter_type i=m.elements.begin(), end=m.elements.end(); i != end; ++i)
    {
        io_ex::write_int32<hamigaki::little>(out, i->first.first);
        io_ex::write_int32<hamigaki::little>(out, i->first.second);
        io_ex::binary_write(out, i->second.copy(buf));
    }

    out.close();
}
