// game_project.hpp: game project

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_PROJECT_HPP
#define GAME_PROJECT_HPP

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

struct game_project
{
    std::string title;
    int screen_width;
    int screen_height;
    unsigned long bg_color;
    std::string gravity;
    std::string min_vy;
    std::string start_map;

    // not serialize
    std::string dir;

    game_project()
        : screen_width(640), screen_height(480)
        , bg_color(0xFF7766DD), gravity("-0.6"), min_vy("-10.0")
    {
    }
};

BOOST_CLASS_VERSION(game_project, 1);

namespace boost { namespace serialization {

template<class Archive>
inline void serialize(
    Archive& ar, game_project& proj, const unsigned int file_version)
{
    ar & make_nvp("title", proj.title);
    ar & make_nvp("screen-width", proj.screen_width);
    ar & make_nvp("screen-height", proj.screen_height);
    ar & make_nvp("bg-color", proj.bg_color);
    if (file_version > 0)
    {
        ar & make_nvp("gravity", proj.gravity);
        ar & make_nvp("min-vy", proj.min_vy);
    }
    else
    {
        float gravity;
        float min_vy;

        if (Archive::is_saving::value)
        {
            gravity = boost::lexical_cast<float>(proj.gravity);
            min_vy = boost::lexical_cast<float>(proj.min_vy);
        }

        ar & make_nvp("gravity", gravity);
        ar & make_nvp("min-vy", min_vy);

        if (Archive::is_loading::value)
        {
            proj.gravity = boost::lexical_cast<std::string>(gravity);
            proj.min_vy = boost::lexical_cast<std::string>(min_vy);
        }
    }
    ar & make_nvp("start-map", proj.start_map);
}

} } // End namespaces serialization, boost.

#endif // GAME_PROJECT_HPP
