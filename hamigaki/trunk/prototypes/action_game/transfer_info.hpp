// transfer_info.hpp: transfer information

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef TRANSFER_INFO_HPP
#define TRANSFER_INFO_HPP

#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <map>
#include <string>

struct transfer_info
{
    enum direction
    {
        none,
        left,
        right,
        down,
        up
    };

    std::string map_file;
    int x;
    int y;
    direction enter_dir;
    direction leave_dir;

    transfer_info() : x(0), y(0), enter_dir(down), leave_dir(none)
    {
    }
};

BOOST_CLASS_VERSION(transfer_info, 1);

typedef std::map<std::pair<int,int>,transfer_info> transfer_info_table;

void save_transfer_infos(
    const char* filename, const transfer_info_table& table);

void load_transfer_infos(const char* filename, transfer_info_table& table);

namespace boost { namespace serialization {

template<class Archive>
inline void serialize(
    Archive& ar, transfer_info& t, const unsigned int file_version)
{
    ar & make_nvp("x", t.x);
    ar & make_nvp("y", t.y);
    ar & make_nvp("map-file", t.map_file);

    if (file_version > 0)
    {
        ar & make_nvp("enter-dir", t.enter_dir);
        ar & make_nvp("leave-dir", t.leave_dir);
    }
}

} } // End namespaces serialization, boost.

#endif // TRANSFER_INFO_HPP
