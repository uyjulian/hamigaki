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
    velocity speed;
    routine_type routine;

    character() : width(0.0f), height(0.0f)
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

struct game_system
{
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
        c->speed.vy += gravity;
        c->speed.vy = (std::max)(c->speed.vy, min_vy);

        float y = c->position.y;
        y += c->speed.vy;

        rectangle r = sweep_y(c->bounds(), c->speed.vy);

        character_list& ls = game->characters;
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            const rectangle& r2 = i->bounds();

            if (intersect_rects(r, r2))
            {
                c->speed.vy = 0.0f;
                y = r2.y + r2.ly;
                r.y = y;
                r.ly = c->height + c->position.y - r.y;
            }
        }

        c->position.y = y;

        boost::tie(game,c) = self.yield();
    }
}

void lift_routine(
    routine_type::self& self, game_system* game, character* c)
{
    const float max_y = 480.0f + 32.0f;
    const float min_y = -32.0f;

    while (true)
    {
        float y = c->position.y;
        y += c->speed.vy;
        if (y > max_y)
            y -= max_y;

        rectangle r = sweep_y(c->bounds(), y - c->position.y);

        character_list& ls = game->characters;
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            const rectangle& r2 = i->bounds();

            if (intersect_rects(r, r2))
            {
                i->speed.vy = 0.0f;
                i->position.y = r.y + r.ly;
            }
        }

        c->position.y = y;

        boost::tie(game,c) = self.yield();
    }
}


#include <boost/bind.hpp>
#include <exception>
#include <iostream>

int main()
{
    try
    {
        game_system game;

        character c1;
        c1.attrs.set(char_attr::block);
        c1.position = point(48.0f, 0.0f);
        c1.width = 96.0f;
        c1.height = 16.0f;
        c1.speed.vy = 2.0f;
        c1.routine = routine_type(&lift_routine);
        game.characters.push_back(c1);

        character c2;
        c1.attrs.set(char_attr::player);
        c2.position = point(16.0f, 64.0f);
        c2.width = 32.0f;
        c2.height = 32.0f;
        c2.routine = routine_type(&gravity_routine);
        game.characters.push_back(c2);

        character_list& ls = game.characters;
        for (int i = 0; i < 20; ++i)
        {
            std::for_each(
                ls.begin(), ls.end(),
                boost::bind(&character::move, _1, boost::ref(game))
            );

            std::cout
                << ls.front().position.y
                << '\t'
                << ls.back().position.y
                << std::endl;
        }

    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
