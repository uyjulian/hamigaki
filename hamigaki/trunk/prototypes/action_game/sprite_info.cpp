// sprite_info.cpp: sprite information for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "sprite_info.hpp"
#include "four_char_code.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace io = boost::iostreams;

namespace
{

bool get_next_line(std::istream& is, std::string& line)
{
    while (std::getline(is, line))
    {
        if (!line.empty() && (line[0] != '#'))
            return true;
    }
    return false;
}

void throw_invalid_format(const char* filename)
{
    std::string msg;
    msg = "invalid format: ";
    msg += filename;
    throw std::runtime_error(msg);
}

} // namespace

void save_sprite_info_set(const char* filename, const sprite_info_set& infos)
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
    oa << infos;
}

void load_sprite_info_set(const char* filename, sprite_info_set& infos)
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
    ia >> infos;
}

void
load_sprite_info_set_from_text(const char* filename, sprite_info_set& infos)
{
    std::ifstream is(filename);
    if (!is)
    {
        std::string msg;
        msg = "cannot open file: ";
        msg += filename;
        throw std::runtime_error(msg);
    }

    std::string line;
    if (!get_next_line(is, line))
        throw_invalid_format(filename);

    std::istringstream parser(line);

    sprite_info_set tmp;
    tmp.wait = 15;
    parser >> tmp.texture >> tmp.width >> tmp.height;
    if (!parser)
        throw_invalid_format(filename);

    while (get_next_line(is, line))
    {
        char fcc[5];
        int x;
        int y;
        rectangle<int> bounds;
        rectangle<int> attack;

        parser.str(line);
        parser.clear();
        parser
            >> std::setw(5) >> fcc >> x >> y
            >> bounds.x >> bounds.y
            >> bounds.lx >> bounds.ly;

        if (!parser)
            throw_invalid_format(filename);

        parser
            >> attack.x >> attack.y
            >> attack.lx >> attack.ly;

        sprite_group& grp = tmp.groups[four_char_code(fcc)];
        grp.bound_width = bounds.lx;
        grp.bound_height = bounds.ly;

        sprite_pattern ptn;
        ptn.x = x / tmp.width;
        ptn.y = y / tmp.height;

        if (parser)
        {
            ptn.attack_rect.x = attack.x - tmp.width/2;
            ptn.attack_rect.y = tmp.height - (attack.y + attack.ly);
            ptn.attack_rect.lx = attack.lx;
            ptn.attack_rect.ly = attack.ly;
        }

        ptn.stomp_rect.x = -grp.bound_width/2;
        ptn.stomp_rect.y = grp.bound_height/2;
        ptn.stomp_rect.lx = grp.bound_width;
        ptn.stomp_rect.ly = grp.bound_height/2;

        grp.patterns.push_back(ptn);
    }

    infos.swap(tmp);
}
