// lift_test.cpp: test codes for lifts

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

struct rectangle
{
    float x;
    float y;
    float lx;
    float ly;

    rectangle() : x(0.0f), y(0.0f), lx(0.0f), ly(0.0f)
    {
    }

    rectangle(float x, float y, float lx, float ly)
        : x(x), y(y), lx(lx), ly(ly)
    {
    }
};

inline bool intersect_rects(const rectangle& r1, const rectangle& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y < r2.y + r2.ly) && (r2.y < r1.y + r1.ly) ;
}

inline bool on_rects(const rectangle& r1, const rectangle& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y == r2.y + r2.ly) ;
}

inline rectangle sweep_x(const rectangle& r, float dx)
{
    if (dx < 0.0f)
        return rectangle(r.x+dx, r.y, r.lx-dx, r.ly);
    else
        return rectangle(r.x, r.y, r.lx+dx, r.ly);
}

inline rectangle sweep_y(const rectangle& r, float dy)
{
    if (dy < 0.0f)
        return rectangle(r.x, r.y+dy, r.lx, r.ly-dy);
    else
        return rectangle(r.x, r.y, r.lx, r.ly+dy);
}

struct point
{
    float x;
    float y;

    point() : x(0.0f), y(0.0f)
    {
    }

    point(float x, float y) : x(x), y(y)
    {
    }
};

inline rectangle make_bounds(const point& pos, float width, float height)
{
    rectangle r;
    r.x = pos.x - width * 0.5f;
    r.y = pos.y;
    r.lx = width;
    r.ly = height;
    return r;
}

struct velocity
{
    float vx;
    float vy;

    velocity() : vx(0.0f), vy(0.0f)
    {
    }

    velocity(float vx, float vy) : vx(vx), vy(vy)
    {
    }
};

struct acceleration
{
    float ax;
    float ay;

    acceleration() : ax(0.0f), ay(0.0f)
    {
    }

    acceleration(float ax, float ay) : ax(ax), ay(ay)
    {
    }
};


#include <hamigaki/coroutine/shared_coroutine.hpp>
#include <bitset>

namespace char_attr
{
    enum values
    {
        player,
        enemy,
        weapon,
        block,

        max_value
    };
};

struct game_system;
struct character;

typedef hamigaki::coroutines::shared_coroutine<
    void (game_system*, character*)
> routine_type;

struct character
{
    std::bitset<char_attr::max_value> attrs;
    point position;
    float width;
    float height;
    float vx;
    float vy;
    routine_type routine;

    character() : width(0.0f), height(0.0f), vx(0.0f), vy(0.0f)
    {
    }

    rectangle bounds() const
    {
        return make_bounds(position, width, height);
    }

    void move(game_system& game)
    {
        if (!routine.empty())
        {
            // Note:
            // This copy guarantees the lifetime until the call is completed.
            routine_type ro = routine;
            ro(&game, this);
        }
    }
};


#include <list>

typedef std::list<character> character_list;
typedef character_list::iterator character_iterator;

bool is_on_floor(const character& c, const character_list& ls)
{
    if (c.vy != 0.0f)
        return false;

    const rectangle& r = c.bounds();

    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        if (on_rects(r, i->bounds()))
            return true;
    }

    return false;
}

struct game_system
{
    game_system() : screen_width(640), screen_height(480)
    {
    }

    int screen_width;
    int screen_height;
    character_list characters;
};


void stop_routine(
    routine_type::self& self, game_system* game, character* c)
{
    while (true)
        boost::tie(game,c) = self.yield();
}

void gravity_routine(
    routine_type::self& self, game_system* game, character* c)
{
    const float gravity = -0.6f;
    const float min_vy = -10.0f;

    while (true)
    {
        c->vy += gravity;
        c->vy = (std::max)(c->vy, min_vy);

        float y = c->position.y;
        y += c->vy;

        rectangle r = sweep_y(c->bounds(), c->vy);

        character_list& ls = game->characters;
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            if (!i->attrs.test(char_attr::block))
                continue;

            const rectangle& r2 = i->bounds();

            if (intersect_rects(r, r2))
            {
                c->vy = 0.0f;
                y = r2.y + r2.ly;
                r.y = y;
                r.ly = c->height + c->position.y - r.y;
            }
        }

        c->position.y = y;

        boost::tie(game,c) = self.yield();
    }
}

void loop_lift_routine(
    routine_type::self& self, game_system* game, character* c)
{
    const float max_y = static_cast<float>(game->screen_height);
    const float min_y = -64.0f;

    while (true)
    {
        float y = c->position.y;
        y += c->vy;
        if (y > max_y)
            y -= (max_y - min_y);
        else if (y < min_y)
            y += (max_y - min_y);

        rectangle br = c->bounds();
        rectangle r = sweep_y(br, y - c->position.y);

        character_list& ls = game->characters;
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            const rectangle& r2 = i->bounds();

            if ((i->vy == 0.0f) && on_rects(r2, br))
                i->position.y = y + br.ly;
            else if (intersect_rects(r, r2))
            {
                i->vy = 0.0f;
                if (c->vy < 0.0f)
                    i->position.y = y - i->height;
                else
                    i->position.y = y;
            }
        }

        c->position.y = y;

        boost::tie(game,c) = self.yield();
    }
}


#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <exception>
#include <iostream>

namespace ut = boost::unit_test;

void lift_check(game_system& game, const float y1[], const float y2[])
{
    character_list& ls = game.characters;
    for (std::size_t i = 0; i < 20; ++i)
    {
        std::for_each(
            ls.begin(), ls.end(),
            boost::bind(&character::move, _1, boost::ref(game))
        );

        std::cout
            << ls.front().position.y
            << '\t'
            << ls.back().position.y
            << '\n';

        BOOST_REQUIRE_CLOSE(ls.front().position.y, y1[i], 0.001f);
        BOOST_REQUIRE_CLOSE(ls.back() .position.y, y2[i], 0.001f);
    }
    std::cout << '\n';
}

void up_lift_test()
{
    character c1;
    c1.attrs.set(char_attr::block);
    c1.position = point(48.0f, 0.0f);
    c1.width = 96.0f;
    c1.height = 16.0f;
    c1.vy = 2.0f;
    c1.routine = routine_type(&loop_lift_routine);

    character c2;
    c1.attrs.set(char_attr::player);
    c2.position = point(16.0f, 64.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.routine = routine_type(&gravity_routine);

    const float y1[] =
    {
         2.0f,  4.0f,  6.0f,  8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f,
        22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f, 38.0f, 40.0f
    };

    const float y2[] =
    {
        63.4f, 62.2f, 60.4f, 58.0f, 55.0f, 51.4f, 47.2f, 42.4f, 37.0f, 36.0f,
        38.0f, 40.0f, 42.0f, 44.0f, 46.0f, 48.0f, 50.0f, 52.0f, 54.0f, 56.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        lift_check(game, y1, y2);
    }

    c1.routine.restart();
    c2.routine.restart();

    {
        game_system game;
        game.characters.push_back(c2);
        game.characters.push_back(c1);
        lift_check(game, y2, y1);
    }
}

void down_lift_test()
{
    character c1;
    c1.attrs.set(char_attr::block);
    c1.position = point(48.0f, 64.0f);
    c1.width = 96.0f;
    c1.height = 16.0f;
    c1.vy = -2.0f;
    c1.routine = routine_type(&loop_lift_routine);

    character c2;
    c1.attrs.set(char_attr::player);
    c2.position = point(16.0f, 96.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.routine = routine_type(&gravity_routine);

    const float y1[] =
    {
        62.0f, 60.0f, 58.0f, 56.0f, 54.0f, 52.0f, 50.0f, 48.0f, 46.0f, 44.0f,
        42.0f, 40.0f, 38.0f, 36.0f, 34.0f, 32.0f, 30.0f, 28.0f, 26.0f, 24.0f,
    };

    const float y2[] =
    {
        95.4f, 94.2f, 92.4f, 90.0f, 87.0f, 83.4f, 79.2f, 74.4f, 69.0f, 63.0f,
        58.0f, 56.0f, 54.0f, 52.0f, 50.0f, 48.0f, 46.0f, 44.0f, 42.0f, 40.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        lift_check(game, y1, y2);
    }

    c1.routine.restart();
    c2.routine.restart();

    {
        game_system game;
        game.characters.push_back(c2);
        game.characters.push_back(c1);
        lift_check(game, y2, y1);
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("lift test");
    test->add(BOOST_TEST_CASE(&up_lift_test));
    test->add(BOOST_TEST_CASE(&down_lift_test));
    return test;
}
