// game_project_io.hpp: game project I/O

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "game_project_io.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <fstream>
#include <stdexcept>
#include <string>

namespace io = boost::iostreams;

void save_game_project(const char* filename, const game_project& proj)
{
    std::ofstream file(filename, std::ios_base::binary);
    if (!file)
    {
        std::string msg;
        msg += "cannot open file '";
        msg += filename;
        msg += "'";
        throw std::runtime_error(msg);
    }

    io::filtering_stream<io::output> os;
    os.push(io::zlib_compressor());
    os.push(file);
    boost::archive::binary_oarchive oa(os);
    oa << proj;
}

game_project load_game_project(const char* filename)
{
    std::ifstream file(filename, std::ios_base::binary);
    if (!file)
    {
        std::string msg;
        msg += "cannot open file '";
        msg += filename;
        msg += "'";
        throw std::runtime_error(msg);
    }

    io::filtering_stream<io::input> is;
    is.push(io::zlib_decompressor());
    is.push(file);
    boost::archive::binary_iarchive ia(is);
    game_project proj;
    ia >> proj;
    return proj;
}
