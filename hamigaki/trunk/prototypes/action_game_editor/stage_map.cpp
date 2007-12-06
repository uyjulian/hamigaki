// stage_map.cpp: stage map for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.


#include "stage_map.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <rpc.h>

namespace
{

const ::GUID guids[] =
{
    { 0xD5D26CC5, 0xD8BD, 0x40A4, {0x83,0x9E,0xE3,0xC7,0xFA,0x57,0x0F,0x66} },
    { 0xF19E0B84, 0x45A2, 0x4FC6, {0x96,0x8A,0xE4,0x49,0x80,0x9A,0xC6,0x92} },
    { 0xB2107203, 0x173F, 0x4AAA, {0x92,0x28,0xE9,0x54,0xED,0xBF,0x9C,0x2F} },
    { 0x6F642A22, 0x86B5, 0x41BF, {0x99,0x03,0x44,0xC0,0x0B,0x45,0x4D,0x6C} },
    { 0x1831D6D2, 0x0A6A, 0x4B73, {0x9F,0x92,0xE9,0x4B,0x56,0xEC,0xB9,0x70} },
    { 0x52B81339, 0x4AA0, 0x47D4, {0x9C,0xC8,0xE7,0x7A,0x02,0x3E,0xCA,0x7F} },
    { 0x5D55A5B6, 0x66B1, 0x4D4A, {0xAB,0xB4,0xAB,0xE3,0x38,0x94,0x6D,0x28} },
    { 0x8DC4CEDB, 0xDF9F, 0x4B9E, {0xB2,0xC3,0x0D,0x4A,0x6D,0x89,0x8D,0xF3} },
    { 0x9F211EEC, 0xAA1B, 0x4704, {0xAA,0x25,0x5C,0x40,0x19,0x24,0xDD,0xAD} },
    { 0xF311F6F5, 0x8B30, 0x48CE, {0xA8,0x43,0x10,0xDA,0xC4,0x26,0x84,0xDA} },
    { 0x7FB448BA, 0xDAE6, 0x45CB, {0x9E,0xD1,0xC1,0x6A,0x7C,0xFB,0x0C,0x97} },
    { 0x2FC40FC2, 0x841F, 0x48BC, {0xB9,0x10,0xFD,0x03,0xA6,0x13,0xE5,0x8A} },
    { 0x106AD0B7, 0x9A0A, 0x4351, {0xAD,0xB4,0xE2,0x1A,0x4C,0xFC,0xEC,0xD5} },
    { 0x2F1830C6, 0x6734, 0x4D14, {0x9A,0x15,0x6A,0xD5,0xD6,0x6D,0x4B,0xFA} },
    { 0x1047BBAC, 0x5FC8, 0x4435, {0xBD,0x26,0xAA,0x0A,0x11,0x87,0xBE,0x99} },
    { 0x639CE8F6, 0xDB5A, 0x45DA, {0xA9,0x63,0x9D,0x93,0xF7,0x11,0xD3,0xFE} }
};

::GUID char_to_guid(char c)
{
    switch (c)
    {
        default:
        case '@': return guids[0];
        case 'U': return guids[1];
        case 'D': return guids[2];
        case 'o': return guids[3];
        case 'a': return guids[4];
        case 'p': return guids[5];
        case 'w': return guids[6];
        case '=': return guids[7];
        case 'G': return guids[8];
        case 'I': return guids[9];
        case 'm': return guids[10];
        case '$': return guids[11];
        case '?': return guids[12];
        case 'S': return guids[13];
        case '/': return guids[14];
        case '\\': return guids[15];
    }
}

struct guid_equal_to : std::binary_function< ::GUID, ::GUID, bool>
{
    bool operator()(const ::GUID& lhs, const ::GUID& rhs) const
    {
        return
            (lhs.Data1 == rhs.Data1) &&
            (lhs.Data2 == rhs.Data2) &&
            (lhs.Data3 == rhs.Data3) &&
            std::equal(lhs.Data4, lhs.Data4+8, rhs.Data4) ;
    }
};

char guid_to_char(const ::GUID& type)
{
    const ::GUID* beg = guids;
    const ::GUID* end = beg + sizeof(guids)/sizeof(guids[0]);

    const ::GUID* pos =
        std::find_if(beg, end, std::bind2nd(guid_equal_to(), type));

    if (pos == end)
        return ' ';
    else
        return "@UDoapw=GIm$?S/\\"[pos-beg];
}

} // namespace

void load_map_from_text(const char* filename, stage_map& m)
{
    std::ifstream is(filename);
    if (!is)
    {
        m.elements.clear();
        return;
    }

    int width = 0;

    map_elements tmp;
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(is, line))
    {
        if (line.empty() || (line[0] == '#'))
            continue;

        lines.push_back(line);
        width = (std::max)(width, static_cast<int>(line.size()));
    }

    int height = static_cast<int>(lines.size());
    for (int y = 0; y < height; ++y)
    {
        const std::string& line = lines[height-1-y];
        for (std::size_t x = 0; x < line.size(); ++x)
        {
            char c = line[x];
            if (c != ' ')
                tmp[std::make_pair(x*32+16, y*32)] = char_to_guid(c);
        }
    }

    m.width = width*32;
    m.height = height*32;
    m.elements.swap(tmp);
}

void save_map_to_text(const char* filename, const stage_map& m)
{
    std::ofstream os(filename);
    if (!os)
    {
        std::string msg;
        msg = "cannot open file '";
        msg += filename;
        msg += "'";
        throw std::runtime_error(msg);
    }

    std::string line(static_cast<std::size_t>(m.width/32), ' ');
    std::vector<std::string> tmp(static_cast<std::size_t>(m.height/32), line);

    ::GUID buf;
    typedef map_elements::const_iterator iter_type;
    for (iter_type i=m.elements.begin(), end=m.elements.end(); i != end; ++i)
    {
        int x = i->first.first;
        int y = i->first.second;
        tmp[y/32][x/32] = guid_to_char(i->second.copy(buf));
    }

    std::copy(
        tmp.rbegin(), tmp.rend(), std::ostream_iterator<std::string>(os, "\n")
    );

    if (!os)
    {
        std::string msg;
        msg = "failed to write for '";
        msg += filename;
        msg += "'";
        throw std::runtime_error(msg);
    }
}
