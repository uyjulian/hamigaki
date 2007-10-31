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


#include <boost/function.hpp>
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

namespace slope_type
{
    enum values
    {
        none,
        left_down,
        right_down
    };
};

struct game_system;
struct character;

typedef boost::function<
    void (game_system*, character*)
> routine_type;

typedef boost::function<
    void (game_system*, character*, character*)
> collision_event_type;

struct character
{
    std::bitset<char_attr::max_value> attrs;
    point position;
    float width;
    float height;
    float vx;
    float vy;
    slope_type::values slope;
    routine_type routine;
    collision_event_type on_collide_block_side;

    character()
        : width(0.0f), height(0.0f), vx(0.0f), vy(0.0f)
        , slope(slope_type::none)
    {
    }

    rectangle bounds() const
    {
        return make_bounds(position, width, height);
    }

    float slope_height(float dx) const
    {
        if (width == 0.0f)
            return 0.0f;

        if (slope == slope_type::left_down)
            return height * (0.5f + dx/width);
        else if (slope == slope_type::right_down)
            return height * (0.5f - dx/width);
        else
            return height;
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

inline bool on_characters(const character& c1, const character& c2)
{
    if (c2.slope == slope_type::none)
        return on_rects(c1.bounds(), c2.bounds());
    else
    {
        if (c1.vy != 0.0f)
            return false;

        const rectangle& r2 = c2.bounds();

        if ((r2.x <= c1.position.x) && (c1.position.x < r2.x + r2.lx))
        {
            float dx = c1.position.x - c2.position.x;
            float height = c2.slope_height(dx);

            return r2.y + height == c1.position.y;
        }
        else
            return false;
    }
}


#include <list>

typedef std::list<character> character_list;
typedef character_list::iterator character_iterator;

slope_type::values current_slope(const character& c, const character_list& ls)
{
    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (!i->attrs.test(char_attr::block) || (i->slope == slope_type::none))
            continue;

        const rectangle& r2 = i->bounds();
        if ((r2.x <= c.position.x) && (c.position.x <  r2.x + r2.lx) &&
            (r2.y <  c.position.y) && (c.position.y <= r2.y + r2.ly) )
        {
            return i->slope;
        }
    }

    return slope_type::none;
}

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

        if (i->slope != slope_type::none)
        {
            const rectangle& r2 = i->bounds();
            if ((r2.x > c.position.x) || (c.position.x >= r2.x + r2.lx))
                continue;

            float dx = c.position.x - i->position.x;
            float height = i->slope_height(dx);
            if (i->position.y + height == c.position.y)
                return true;
        }
        else if (on_rects(r, i->bounds()))
            return true;
    }

    return false;
}

struct game_system
{
    game_system()
        : screen_width(640), screen_height(480)
        , gravity(-0.6f), min_vy(-10.0f)
    {
    }

    int screen_width;
    int screen_height;
    float gravity;
    float min_vy;
    character_list characters;
};


void turn(game_system* game, character* c, character* target)
{
    c->vx = -c->vx;
}

void stop_routine(game_system* game, character* c)
{
}

void vx_routine(game_system* game, character* c)
{
    character_list& ls = game->characters;
    bool on_floor = is_on_floor(*c, ls);

    float x = c->position.x;
    x += c->vx;

    rectangle r = sweep_x(c->bounds(), c->vx);

    slope_type::values slope = current_slope(*c, ls);

    rectangle er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->position.x;
        er.y = c->position.y;
        er = sweep_x(er, c->vx);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->position.x - er.lx;
        er.y = c->position.y;
        er = sweep_x(er, c->vx);
    }

    character_iterator ti = ls.end();
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rectangle& r2 = i->bounds();

        if ((i->slope == slope_type::left_down)
            && (c->position.x < r2.x + r2.lx) )
        {
            ;
        }
        else if ((i->slope == slope_type::right_down) &&
            (r2.x < c->position.x) )
        {
            ;
        }
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            ti = i;
            if (c->vx < 0.0f)
            {
                r.x = r2.x + r2.lx;
                x = r.x + c->width * 0.5f;
                r.lx = c->width + c->position.x - x;
            }
            else
            {
                x = r2.x - c->width * 0.5f;
                r.lx = c->width + x - c->position.x;
            }
        }
    }

    if (on_floor)
    {
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            if (!i->attrs.test(char_attr::block) ||
                (i->slope == slope_type::none) )
                continue;

            const rectangle& r2 = i->bounds();
            if ((r2.x <= c->position.x) && (c->position.x <  r2.x + r2.lx) &&
                (r2.y <  c->position.y) && (c->position.y <= r2.y + r2.ly) )
            {
                float dx = x - i->position.x;
                float w = i->width*0.5f;
                if (dx < -w)
                    dx = -w;
                else if (dx > w)
                    dx = w;

                float height = i->slope_height(dx);
                c->position.y = i->position.y + height;
                break;
            }
        }

        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            if (!i->attrs.test(char_attr::block) ||
                (i->slope == slope_type::none) )
                continue;

            const rectangle& r2 = i->bounds();
            if ((r2.x <= x) && (x <  r2.x + r2.lx) &&
                (r2.y <  c->position.y) && (c->position.y <= r2.y + r2.ly) )
            {
                float dx = x - i->position.x;
                float height = i->slope_height(dx);
                c->position.y = i->position.y + height;
                break;
            }
        }
    }

    c->position.x = x;

    if ((ti != ls.end()) && c->on_collide_block_side)
        c->on_collide_block_side(game, c, &*ti);
}

void vy_routine(game_system* game, character* c)
{
    float y = c->position.y;
    y += c->vy;

    rectangle r = sweep_y(c->bounds(), c->vy);

    character_list& ls = game->characters;
    slope_type::values slope = current_slope(*c, ls);

    rectangle er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->position.x;
        er.y = c->position.y;
        er = sweep_y(er, c->vy);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->position.x - er.lx;
        er.y = c->position.y;
        er = sweep_y(er, c->vy);
    }

    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rectangle& r2 = i->bounds();

        if ((i->slope != slope_type::none) &&
            (r2.y <= c->position.y) && (r.y < r2.y + r2.ly) )
        {
            if ((r2.x <= c->position.x) && (c->position.x < r2.x + r2.lx))
            {
                float dx = c->position.x - i->position.x;
                float height = i->slope_height(dx);
                float y2 = r2.y + height;

                if (y < y2)
                {
                    c->vy = 0.0f;
                    y = y2;
                    r.y = y;
                    r.ly = c->height + c->position.y - r.y;
                }
            }
        }
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            if (c->vy < 0.0f)
            {
                c->vy = 0.0f;
                y = r2.y + r2.ly;
                r.y = y;
                r.ly = c->height + c->position.y - y;
            }
            else
            {
                c->vy = -c->vy * 0.5f;
                y = r2.y - c->height;
                r.ly = c->height + y - c->position.y;
            }
        }
    }

    c->position.y = y;
}

void velocity_routine(game_system* game, character* c)
{
    vx_routine(game, c);

    c->vy += game->gravity;
    c->vy = (std::max)(c->vy, game->min_vy);

    vy_routine(game, c);
}

void move_y(game_system* game, character* c, float vy)
{
    float old_y = c->position.y;
    float new_y = old_y + vy;

    rectangle br = c->bounds();
    rectangle r = sweep_y(br, vy);

    character_list& ls = game->characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if ((c->slope != slope_type::none) && (i->position.y > old_y))
        {
            const rectangle& r2 = i->bounds();

            if ((i->vy == 0.0f) && on_characters(*i, *c))
            {
                float dx = i->position.x - c->position.x;
                float height = c->slope_height(dx);

                i->position.y = new_y + height;
            }
            else if ((new_y <= r2.y) && (r2.y < new_y + br.ly) &&
                (r.x <= i->position.x) && (i->position.x < r.x + r.lx) )
            {
                float dx = i->position.x - c->position.x;
                float height = c->slope_height(dx);
                float y1 = new_y + height;

                if (r2.y < y1)
                {
                    i->vy = 0.0f;
                    i->position.y = y1;
                }
            }
        }
        else
        {
            const rectangle& r2 = i->bounds();

            if ((i->vy == 0.0f) && on_rects(r2, br))
            {
                if (i->attrs.test(char_attr::block))
                {
                    if (vy < 0.0f)
                        c->position.y = new_y;
                    move_y(game, &*i, new_y + br.ly - r2.y);
                    c->position.y = old_y;
                }
                else
                    i->position.y = new_y + br.ly;
            }
            else if (intersect_rects(r, r2))
            {
                if (vy < 0.0f)
                {
                    i->vy = 0.0f;
                    if (i->attrs.test(char_attr::block))
                    {
                        c->position.y = new_y;
                        move_y(game, &*i, new_y - r2.ly - r2.y);
                        c->position.y = old_y;
                    }
                    else
                        i->position.y = new_y - r2.ly;
                }
                else
                {
                    i->vy = 0.0f;
                    if (i->attrs.test(char_attr::block))
                        move_y(game, &*i, new_y + br.ly - r2.y);
                    else
                        i->position.y = new_y + br.ly;
                }
            }
        }
    }

    c->position.y = new_y;
}

void loop_lift_routine(game_system* game, character* c)
{
    const float max_y = static_cast<float>(game->screen_height);
    const float min_y = -64.0f;

    float y = c->position.y;

    y += c->vy;

    if (y > max_y)
    {
        move_y(game, c, max_y - c->position.y);
        c->position.y = min_y;
        move_y(game, c, y - max_y);
    }
    else if (y < min_y)
    {
        move_y(game, c, min_y - c->position.y);
        c->position.y = max_y;
        move_y(game, c, y - min_y);
    }
    else
        move_y(game, c, c->vy);
}


#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace ut = boost::unit_test;

void move_check_x1(game_system& game, const float x1[])
{
    character_list& ls = game.characters;
    for (std::size_t i = 0; i < 20; ++i)
    {
        std::for_each(
            ls.begin(), ls.end(),
            boost::bind(&character::move, _1, boost::ref(game))
        );

        std::cout
            << ls.front().position.x
            << ", "
            << ls.front().position.y
            << '\n';

        BOOST_REQUIRE_CLOSE(ls.front().position.x, x1[i], 0.001f);
    }
    std::cout << '\n';
}

void move_check_y1(game_system& game, const float y1[])
{
    character_list& ls = game.characters;
    for (std::size_t i = 0; i < 20; ++i)
    {
        std::for_each(
            ls.begin(), ls.end(),
            boost::bind(&character::move, _1, boost::ref(game))
        );

        std::cout
            << ls.front().position.x
            << ", "
            << ls.front().position.y
            << '\n';

        BOOST_REQUIRE_CLOSE(ls.front().position.y, y1[i], 0.001f);
    }
    std::cout << '\n';
}

void move_check_y1_y2(game_system& game, const float y1[], const float y2[])
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
    c1.routine = &loop_lift_routine;

    character c2;
    c2.attrs.set(char_attr::player);
    c2.position = point(16.0f, 64.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.routine = &velocity_routine;

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
        move_check_y1_y2(game, y1, y2);
    }

    {
        game_system game;
        game.characters.push_back(c2);
        game.characters.push_back(c1);
        move_check_y1_y2(game, y2, y1);
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
    c1.routine = &loop_lift_routine;

    character c2;
    c2.attrs.set(char_attr::player);
    c2.position = point(16.0f, 96.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.routine = &velocity_routine;

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
        move_check_y1_y2(game, y1, y2);
    }

    {
        game_system game;
        game.characters.push_back(c2);
        game.characters.push_back(c1);
        move_check_y1_y2(game, y2, y1);
    }
}

void walk_on_lift_test()
{
    character c1;
    c1.attrs.set(char_attr::block);
    c1.position = point(32.0f, 0.0f);
    c1.width = 64.0f;
    c1.height = 16.0f;
    c1.vy = 2.0f;
    c1.routine = &loop_lift_routine;

    character c2;
    c2.attrs.set(char_attr::player);
    c2.position = point(16.0f, 16.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.vx = 4.0f;
    c2.routine = &velocity_routine;

    const float y1[] =
    {
         2.0f,  4.0f,  6.0f,  8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f,
        22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f, 38.0f, 40.0f
    };

    const float y2[] =
    {
        18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f,
        38.0f, 40.0f, 42.0f, 44.0f, 46.0f, 47.4f, 46.2f, 44.4f, 42.0f, 39.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        move_check_y1_y2(game, y1, y2);
    }
}

void right_walk_test()
{
    character c1;
    c1.attrs.set(char_attr::player);
    c1.position = point(16.0f, 32.0f);
    c1.width = 32.0f;
    c1.height = 64.0f;
    c1.vx = 2.0f;
    c1.routine = &velocity_routine;
    c1.on_collide_block_side = &turn;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(64.0f, 0.0f);
    c2.width = 128.0f;
    c2.height = 32.0f;

    character c3;
    c3.attrs.set(char_attr::block);
    c3.position = point(81.0f, 64.0f);
    c3.width = 32.0f;
    c3.height = 32.0f;

    character c4;
    c4.attrs.set(char_attr::block);
    c4.position = point(80.0f, 32.0f);
    c4.width = 32.0f;
    c4.height = 32.0f;

    const float x1[] =
    {
        18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f,
        38.0f, 40.0f, 42.0f, 44.0f, 46.0f, 48.0f, 48.0f, 46.0f, 44.0f, 42.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        game.characters.push_back(c3);
        game.characters.push_back(c4);
        move_check_x1(game, x1);
    }
}

void left_walk_test()
{
    character c1;
    c1.attrs.set(char_attr::player);
    c1.position = point(80.0f, 32.0f);
    c1.width = 32.0f;
    c1.height = 64.0f;
    c1.vx = -2.0f;
    c1.routine = &velocity_routine;
    c1.on_collide_block_side = &turn;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(64.0f, 0.0f);
    c2.width = 128.0f;
    c2.height = 32.0f;

    character c3;
    c3.attrs.set(char_attr::block);
    c3.position = point(15.0f, 64.0f);
    c3.width = 32.0f;
    c3.height = 32.0f;

    character c4;
    c4.attrs.set(char_attr::block);
    c4.position = point(16.0f, 32.0f);
    c4.width = 32.0f;
    c4.height = 32.0f;

    const float x1[] =
    {
        78.0f, 76.0f, 74.0f, 72.0f, 70.0f, 68.0f, 66.0f, 64.0f, 62.0f, 60.0f,
        58.0f, 56.0f, 54.0f, 52.0f, 50.0f, 48.0f, 48.0f, 50.0f, 52.0f, 54.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        game.characters.push_back(c3);
        game.characters.push_back(c4);
        move_check_x1(game, x1);
    }
}

void fall_on_slope_test()
{
    character c1;
    c1.attrs.set(char_attr::player);
    c1.position = point(16.0f, 64.0f);
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.routine = &velocity_routine;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(16.0f, 0.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.slope = slope_type::left_down;

    const float y1[] =
    {
        63.4f, 62.2f, 60.4f, 58.0f, 55.0f, 51.4f, 47.2f, 42.4f, 37.0f, 31.0f,
        24.4f, 17.2f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        move_check_y1(game, y1);
    }
}

void jump_test()
{
    character c1;
    c1.attrs.set(char_attr::player);
    c1.position = point(16.0f, 64.0f);
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = 8.0f;
    c1.routine = &velocity_routine;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(16.0f, 128.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;

    const float y1[] =
    {
        71.4f, 78.2f, 84.4f, 90.0f, 95.0f, 96.0f, 93.2f, 89.8f, 85.8f, 81.2f,
        76.0f, 70.2f, 63.8f, 56.8f, 49.2f, 41.0f, 32.2f, 22.8f, 12.8f,  2.8f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        move_check_y1(game, y1);
    }

    c2.slope = slope_type::left_down;

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        move_check_y1(game, y1);
    }
}

void go_up_slope_test()
{
    character c1;
    c1.attrs.set(char_attr::player);
    c1.position = point(16.0f, 32.0f);
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vx = 4.0f;
    c1.routine = &velocity_routine;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(64.0f, 0.0f);
    c2.width = 128.0f;
    c2.height = 32.0f;

    character c3;
    c3.attrs.set(char_attr::block);
    c3.position = point(48.0f, 32.0f);
    c3.width = 32.0f;
    c3.height = 32.0f;
    c3.slope = slope_type::left_down;

    character c4;
    c4.attrs.set(char_attr::block);
    c4.position = point(80.0f, 32.0f);
    c4.width = 32.0f;
    c4.height = 32.0f;

    const float y1[] =
    {
        32.0f, 32.0f, 32.0f, 32.0f, 36.0f, 40.0f, 44.0f, 48.0f, 52.0f, 56.0f,
        60.0f, 64.0f, 64.0f, 64.0f, 64.0f, 64.0f, 64.0f, 64.0f, 64.0f, 64.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        game.characters.push_back(c3);
        game.characters.push_back(c4);
        move_check_y1(game, y1);
    }
}

void go_down_slope_test()
{
    character c1;
    c1.attrs.set(char_attr::player);
    c1.position = point(16.0f, 64.0f);
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vx = 4.0f;
    c1.routine = &velocity_routine;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(16.0f, 32.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;

    character c3;
    c3.attrs.set(char_attr::block);
    c3.position = point(48.0f, 32.0f);
    c3.width = 32.0f;
    c3.height = 32.0f;
    c3.slope = slope_type::right_down;

    character c4;
    c4.attrs.set(char_attr::block);
    c4.position = point(48.0f, 0.0f);
    c4.width = 96.0f;
    c4.height = 32.0f;

    const float y1[] =
    {
        64.0f, 64.0f, 64.0f, 64.0f, 60.0f, 56.0f, 52.0f, 48.0f, 44.0f, 40.0f,
        36.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        game.characters.push_back(c3);
        game.characters.push_back(c4);
        move_check_y1(game, y1);
    }
}

void fall_on_down_slope_lift_test()
{
    character c1;
    c1.attrs.set(char_attr::block);
    c1.position = point(16.0f, 64.0f);
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = -2.0f;
    c1.routine = &loop_lift_routine;
    c1.slope = slope_type::left_down;

    character c2;
    c2.attrs.set(char_attr::player);
    c2.position = point(16.0f, 96.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;
    c2.routine = &velocity_routine;

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
        move_check_y1_y2(game, y1, y2);
    }

    {
        game_system game;
        game.characters.push_back(c2);
        game.characters.push_back(c1);
        move_check_y1_y2(game, y2, y1);
    }
}

void up_lift_chain_test()
{
    character c1;
    c1.attrs.set(char_attr::block);
    c1.position = point(16.0f, 0.0f);
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = 2.0f;
    c1.routine = &loop_lift_routine;

    character c2;
    c2.attrs.set(char_attr::block);
    c2.position = point(16.0f, 32.0f);
    c2.width = 32.0f;
    c2.height = 32.0f;

    character c3;
    c3.attrs.set(char_attr::player);
    c3.position = point(16.0f, 64.0f);
    c3.width = 32.0f;
    c3.height = 32.0f;

    const float y1[] =
    {
         2.0f,  4.0f,  6.0f,  8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f,
        22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f, 34.0f, 36.0f, 38.0f, 40.0f
    };

    const float y3[] =
    {
        66.0f, 68.0f, 70.0f, 72.0f, 74.0f, 76.0f, 78.0f, 80.0f, 82.0f, 84.0f,
        86.0f, 88.0f, 90.0f, 92.0f, 94.0f, 96.0f, 98.0f,100.0f,102.0f,104.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        game.characters.push_back(c3);
        move_check_y1_y2(game, y1, y3);
    }

    {
        game_system game;
        game.characters.push_back(c3);
        game.characters.push_back(c2);
        game.characters.push_back(c1);
        move_check_y1_y2(game, y3, y1);
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("lift test");
    test->add(BOOST_TEST_CASE(&up_lift_test));
    test->add(BOOST_TEST_CASE(&down_lift_test));
    test->add(BOOST_TEST_CASE(&walk_on_lift_test));
    test->add(BOOST_TEST_CASE(&right_walk_test));
    test->add(BOOST_TEST_CASE(&left_walk_test));
    test->add(BOOST_TEST_CASE(&fall_on_slope_test));
    test->add(BOOST_TEST_CASE(&jump_test));
    test->add(BOOST_TEST_CASE(&go_up_slope_test));
    test->add(BOOST_TEST_CASE(&go_down_slope_test));
    test->add(BOOST_TEST_CASE(&fall_on_down_slope_lift_test));
    test->add(BOOST_TEST_CASE(&up_lift_chain_test));
    return test;
}
