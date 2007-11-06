// lift_test.cpp: test codes for lifts

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

struct rect
{
    float x;
    float y;
    float lx;
    float ly;

    rect() : x(0.0f), y(0.0f), lx(0.0f), ly(0.0f)
    {
    }

    rect(float x, float y, float lx, float ly)
        : x(x), y(y), lx(lx), ly(ly)
    {
    }
};

inline bool intersect_rects(const rect& r1, const rect& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y < r2.y + r2.ly) && (r2.y < r1.y + r1.ly) ;
}

inline bool on_rects(const rect& r1, const rect& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y == r2.y + r2.ly) ;
}

inline rect sweep_x(const rect& r, float dx)
{
    if (dx < 0.0f)
        return rect(r.x+dx, r.y, r.lx-dx, r.ly);
    else
        return rect(r.x, r.y, r.lx+dx, r.ly);
}

inline rect sweep_y(const rect& r, float dy)
{
    if (dy < 0.0f)
        return rect(r.x, r.y+dy, r.lx, r.ly-dy);
    else
        return rect(r.x, r.y, r.lx, r.ly+dy);
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
struct game_character;

typedef boost::function<
    void (game_system*, game_character*)
> routine_type;

typedef boost::function<
    void (game_system*, game_character*, game_character*)
> collision_event_type;

struct game_character
{
    std::bitset<char_attr::max_value> attrs;
    float x;
    float y;
    float width;
    float height;
    float vx;
    float vy;
    slope_type::values slope;
    routine_type routine;
    collision_event_type on_collide_block_side;

    game_character()
        : x(0.0f), y(0.0f), width(0.0f), height(0.0f), vx(0.0f), vy(0.0f)
        , slope(slope_type::none)
    {
    }

    rect bounds() const
    {
        rect r;
        r.x = x - width * 0.5f;
        r.y = y;
        r.lx = width;
        r.ly = height;
        return r;
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

inline bool on_characters(const game_character& c1, const game_character& c2)
{
    if (c2.slope == slope_type::none)
        return on_rects(c1.bounds(), c2.bounds());
    else
    {
        if (c1.vy != 0.0f)
            return false;

        const rect& r2 = c2.bounds();

        if ((r2.x <= c1.x) && (c1.x < r2.x + r2.lx))
        {
            float dx = c1.x - c2.x;
            float height = c2.slope_height(dx);

            return r2.y + height == c1.y;
        }
        else
            return false;
    }
}


#include <list>

typedef std::list<game_character> character_list;
typedef character_list::iterator character_iterator;

slope_type::values
current_slope(const game_character& c, const character_list& ls)
{
    const rect& r = c.bounds();

    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (!i->attrs.test(char_attr::block) || (i->slope == slope_type::none))
            continue;

        const rect& r2 = i->bounds();
        if ((r2.x <= c.x) && (c.x <  r2.x + r2.lx) && (r2.y <  c.y))
        {
            if (c.y < r2.y + r2.ly)
                return i->slope;
            else if (c.y == r2.y + r2.ly)
            {
                if (r.x <= r2.x)
                {
                    if (i->slope == slope_type::right_down)
                        return i->slope;
                }
                else if (r2.x + r2.lx <= r.x + r.lx)
                {
                    if (i->slope == slope_type::left_down)
                        return i->slope;
                }
            }
        }
    }

    return slope_type::none;
}

bool is_on_floor(const game_character& c, const character_list& ls)
{
    if (c.vy != 0.0f)
        return false;

    const rect& r = c.bounds();

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
            const rect& r2 = i->bounds();
            if ((r2.x > c.x) || (c.x >= r2.x + r2.lx))
                continue;

            float dx = c.x - i->x;
            float height = i->slope_height(dx);
            if (i->y + height == c.y)
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


void turn(game_system* game, game_character* c, game_character* target)
{
    c->vx = -c->vx;
}

void stop_routine(game_system* game, game_character* c)
{
}

void vx_routine(game_system* game, game_character* c)
{
    character_list& ls = game->characters;
    bool on_floor = is_on_floor(*c, ls);

    float x = c->x;
    x += c->vx;

    rect r = sweep_x(c->bounds(), c->vx);

    slope_type::values slope = current_slope(*c, ls);

    rect er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x;
        er.y = c->y;
        er = sweep_x(er, c->vx);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x - er.lx;
        er.y = c->y;
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

        const rect& r2 = i->bounds();

        if ((i->slope == slope_type::left_down)
            && (c->x < r2.x + r2.lx) )
        {
            ;
        }
        else if ((i->slope == slope_type::right_down) &&
            (r2.x < c->x) )
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
                r.lx = c->width + c->x - x;
            }
            else
            {
                x = r2.x - c->width * 0.5f;
                r.lx = c->width + x - c->x;
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

            const rect& r2 = i->bounds();
            if ((r2.x <= c->x) && (c->x <  r2.x + r2.lx) &&
                (r2.y <  c->y) && (c->y <= r2.y + r2.ly) )
            {
                float dx = x - i->x;
                float w = i->width*0.5f;
                if (dx < -w)
                    dx = -w;
                else if (dx > w)
                    dx = w;

                float height = i->slope_height(dx);
                c->y = i->y + height;
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

            const rect& r2 = i->bounds();
            if ((r2.x <= x) && (x <  r2.x + r2.lx) &&
                (r2.y <  c->y) && (c->y <= r2.y + r2.ly) )
            {
                float dx = x - i->x;
                float height = i->slope_height(dx);
                c->y = i->y + height;
                break;
            }
        }
    }

    c->x = x;

    if ((ti != ls.end()) && c->on_collide_block_side)
        c->on_collide_block_side(game, c, &*ti);
}

void vy_down_routine(game_system* game, game_character* c)
{
    float y = c->y;
    y += c->vy;

    rect r = sweep_y(c->bounds(), c->vy);

    character_list& ls = game->characters;
    slope_type::values slope = current_slope(*c, ls);

    rect er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x;
        er.y = c->y;
        er = sweep_y(er, c->vy);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x - er.lx;
        er.y = c->y;
        er = sweep_y(er, c->vy);
    }

    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rect& r2 = i->bounds();

        if ((i->slope != slope_type::none) &&
            (r2.y <= c->y) && (r.y < r2.y + r2.ly) )
        {
            if ((r2.x <= c->x) && (c->x < r2.x + r2.lx))
            {
                float dx = c->x - i->x;
                float height = i->slope_height(dx);
                float y2 = r2.y + height;

                if (y < y2)
                {
                    c->vy = 0.0f;
                    y = y2;
                    r.y = y;
                    r.ly = c->height + c->y - r.y;
                }
            }
        }
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            float dx = c->x - i->x;
            float height = i->slope_height(dx);
            if (height == r2.ly)
            {
                c->vy = 0.0f;
                y = r2.y + height;
                r.y = y;
                r.ly = c->height + c->y - y;
            }
        }
    }

    c->y = y;
}

void vy_up_routine(game_system* game, game_character* c)
{
    float y = c->y;
    y += c->vy;

    bool collision = false;

    character_list& ls = game->characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rect& r2 = i->bounds();

        if ((r2.x <= c->x) && (c->x < r2.x + r2.lx) &&
            (c->y + c->height <= i->y) && (i->y < y + c->height) )
        {
            collision = true;
            c->vy = -c->vy * 0.5f;
            y = r2.y - c->height;
        }
    }

    if (collision)
    {
        c->y = y;
        return;
    }

    rect r = sweep_y(c->bounds(), c->vy);

    slope_type::values slope = current_slope(*c, ls);

    rect er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x;
        er.y = c->y;
        er = sweep_y(er, c->vy);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x - er.lx;
        er.y = c->y;
        er = sweep_y(er, c->vy);
    }

    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rect& r2 = i->bounds();

        if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            if (c->x < i->x)
                c->x = r2.x - c->width*0.5f;
            else
                c->x = r2.x + r2.lx + c->width*0.5f;
        }
    }

    c->y = y;
}

void vy_routine(game_system* game, game_character* c)
{
    if (c->vy <= 0.0f)
        vy_down_routine(game, c);
    else
        vy_up_routine(game, c);
}

void velocity_routine(game_system* game, game_character* c)
{
    bool on_floor = is_on_floor(*c, game->characters);

    vx_routine(game, c);

    if (!on_floor)
    {
        c->vy += game->gravity;
        c->vy = (std::max)(c->vy, game->min_vy);
    }

    vy_routine(game, c);
}

void move_y(game_system* game, game_character* c, float vy)
{
    float old_y = c->y;
    float new_y = old_y + vy;

    rect br = c->bounds();
    rect r = sweep_y(br, vy);

    character_list& ls = game->characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if ((c->slope != slope_type::none) && (i->y > old_y))
        {
            const rect& r2 = i->bounds();

            if ((i->vy == 0.0f) && on_characters(*i, *c))
            {
                float dx = i->x - c->x;
                float height = c->slope_height(dx);

                i->y = new_y + height;
            }
            else if ((new_y <= r2.y) && (r2.y < new_y + br.ly) &&
                (r.x <= i->x) && (i->x < r.x + r.lx) )
            {
                float dx = i->x - c->x;
                float height = c->slope_height(dx);
                float y1 = new_y + height;

                if (r2.y < y1)
                {
                    i->vy = 0.0f;
                    i->y = y1;
                }
            }
        }
        else
        {
            const rect& r2 = i->bounds();

            if ((i->vy == 0.0f) && on_rects(r2, br))
            {
                if (i->attrs.test(char_attr::block))
                {
                    if (vy < 0.0f)
                        c->y = new_y;
                    move_y(game, &*i, new_y + br.ly - r2.y);
                    c->y = old_y;
                }
                else
                    i->y = new_y + br.ly;
            }
            else if (intersect_rects(r, r2))
            {
                if (vy < 0.0f)
                {
                    i->vy = 0.0f;
                    if (i->attrs.test(char_attr::block))
                    {
                        c->y = new_y;
                        move_y(game, &*i, new_y - r2.ly - r2.y);
                        c->y = old_y;
                    }
                    else
                        i->y = new_y - r2.ly;
                }
                else
                {
                    i->vy = 0.0f;
                    if (i->attrs.test(char_attr::block))
                        move_y(game, &*i, new_y + br.ly - r2.y);
                    else
                        i->y = new_y + br.ly;
                }
            }
        }
    }

    c->y = new_y;
}

void loop_lift_routine(game_system* game, game_character* c)
{
    const float max_y = static_cast<float>(game->screen_height);
    const float min_y = -64.0f;

    float y = c->y;

    y += c->vy;

    if (y > max_y)
    {
        move_y(game, c, max_y - c->y);
        c->y = min_y;
        move_y(game, c, y - max_y);
    }
    else if (y < min_y)
    {
        move_y(game, c, min_y - c->y);
        c->y = max_y;
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
            boost::bind(&game_character::move, _1, boost::ref(game))
        );

        std::cout
            << ls.front().x
            << ", "
            << ls.front().y
            << '\n';

        BOOST_REQUIRE_CLOSE(ls.front().x, x1[i], 0.001f);
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
            boost::bind(&game_character::move, _1, boost::ref(game))
        );

        std::cout
            << ls.front().x
            << ", "
            << ls.front().y
            << '\n';

        BOOST_REQUIRE_CLOSE(ls.front().y, y1[i], 0.001f);
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
            boost::bind(&game_character::move, _1, boost::ref(game))
        );

        std::cout
            << ls.front().y
            << '\t'
            << ls.back().y
            << '\n';

        BOOST_REQUIRE_CLOSE(ls.front().y, y1[i], 0.001f);
        BOOST_REQUIRE_CLOSE(ls.back() .y, y2[i], 0.001f);
    }
    std::cout << '\n';
}

void up_lift_test()
{
    game_character c1;
    c1.attrs.set(char_attr::block);
    c1.x = 48.0f;
    c1.y = 0.0f;
    c1.width = 96.0f;
    c1.height = 16.0f;
    c1.vy = 2.0f;
    c1.routine = &loop_lift_routine;

    game_character c2;
    c2.attrs.set(char_attr::player);
    c2.x = 16.0f;
    c2.y = 64.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::block);
    c1.x = 48.0f;
    c1.y = 64.0f;
    c1.width = 96.0f;
    c1.height = 16.0f;
    c1.vy = -2.0f;
    c1.routine = &loop_lift_routine;

    game_character c2;
    c2.attrs.set(char_attr::player);
    c2.x = 16.0f;
    c2.y = 96.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::block);
    c1.x = 32.0f;
    c1.y = 0.0f;
    c1.width = 64.0f;
    c1.height = 16.0f;
    c1.vy = 2.0f;
    c1.routine = &loop_lift_routine;

    game_character c2;
    c2.attrs.set(char_attr::player);
    c2.x = 16.0f;
    c2.y = 16.0f;
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
        38.0f, 40.0f, 42.0f, 44.0f, 46.0f, 48.0f, 47.4f, 46.2f, 44.4f, 42.0f
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
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 16.0f;
    c1.y = 32.0f;
    c1.width = 32.0f;
    c1.height = 64.0f;
    c1.vx = 2.0f;
    c1.routine = &velocity_routine;
    c1.on_collide_block_side = &turn;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 64.0f;
    c2.y = 0.0f;
    c2.width = 128.0f;
    c2.height = 32.0f;

    game_character c3;
    c3.attrs.set(char_attr::block);
    c3.x = 81.0f;
    c3.y = 64.0f;
    c3.width = 32.0f;
    c3.height = 32.0f;

    game_character c4;
    c4.attrs.set(char_attr::block);
    c4.x = 80.0f;
    c4.y = 32.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 80.0f;
    c1.y = 32.0f;
    c1.width = 32.0f;
    c1.height = 64.0f;
    c1.vx = -2.0f;
    c1.routine = &velocity_routine;
    c1.on_collide_block_side = &turn;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 64.0f;
    c2.y = 0.0f;
    c2.width = 128.0f;
    c2.height = 32.0f;

    game_character c3;
    c3.attrs.set(char_attr::block);
    c3.x = 15.0f;
    c3.y = 64.0f;
    c3.width = 32.0f;
    c3.height = 32.0f;

    game_character c4;
    c4.attrs.set(char_attr::block);
    c4.x = 16.0f;
    c4.y = 32.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 16.0f;
    c1.y = 64.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.routine = &velocity_routine;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 16.0f;
    c2.y = 0.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 16.0f;
    c1.y = 64.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = 8.0f;
    c1.routine = &velocity_routine;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 16.0f;
    c2.y = 128.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 16.0f;
    c1.y = 32.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vx = 4.0f;
    c1.routine = &velocity_routine;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 64.0f;
    c2.y = 0.0f;
    c2.width = 128.0f;
    c2.height = 32.0f;

    game_character c3;
    c3.attrs.set(char_attr::block);
    c3.x = 48.0f;
    c3.y = 32.0f;
    c3.width = 32.0f;
    c3.height = 32.0f;
    c3.slope = slope_type::left_down;

    game_character c4;
    c4.attrs.set(char_attr::block);
    c4.x = 80.0f;
    c4.y = 32.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 16.0f;
    c1.y = 64.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vx = 4.0f;
    c1.routine = &velocity_routine;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 16.0f;
    c2.y = 32.0f;
    c2.width = 32.0f;
    c2.height = 32.0f;

    game_character c3;
    c3.attrs.set(char_attr::block);
    c3.x = 48.0f;
    c3.y = 32.0f;
    c3.width = 32.0f;
    c3.height = 32.0f;
    c3.slope = slope_type::right_down;

    game_character c4;
    c4.attrs.set(char_attr::block);
    c4.x = 48.0f;
    c4.y = 0.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::block);
    c1.x = 16.0f;
    c1.y = 64.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = -2.0f;
    c1.routine = &loop_lift_routine;
    c1.slope = slope_type::left_down;

    game_character c2;
    c2.attrs.set(char_attr::player);
    c2.x = 16.0f;
    c2.y = 96.0f;
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
    game_character c1;
    c1.attrs.set(char_attr::block);
    c1.x = 16.0f;
    c1.y = 0.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = 2.0f;
    c1.routine = &loop_lift_routine;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 16.0f;
    c2.y = 32.0f;
    c2.width = 32.0f;
    c2.height = 32.0f;

    game_character c3;
    c3.attrs.set(char_attr::player);
    c3.x = 16.0f;
    c3.y = 64.0f;
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

void dash_test()
{
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 16.0f;
    c1.y = 32.0f;
    c1.width = 28.0f;
    c1.height = 32.0f;
    c1.vx = 5.0f;
    c1.routine = &velocity_routine;

    const float x1[] =
    {
        21.0f, 26.0f, 31.0f, 36.0f, 41.0f, 46.0f, 51.0f, 56.0f, 61.0f, 66.0f,
        71.0f, 76.0f, 81.0f, 86.0f, 91.0f, 96.0f,101.0f,106.0f,111.0f,116.0f
    };

    game_system game;
    game.characters.push_back(c1);
    for (int i = 0; i < 4; ++i)
    {
        game_character c;
        c.attrs.set(char_attr::block);
        c.x = i*64+16.0f;
        c.y = 0.0f;
        c.width = 32.0f;
        c.height = 32.0f;
        game.characters.push_back(c);
    }
    move_check_x1(game, x1);
}

void dodge_block_test()
{
    game_character c1;
    c1.attrs.set(char_attr::player);
    c1.x = 40.0f;
    c1.y = 32.0f;
    c1.width = 32.0f;
    c1.height = 32.0f;
    c1.vy = 8.0f;
    c1.routine = &velocity_routine;

    game_character c2;
    c2.attrs.set(char_attr::block);
    c2.x = 32.0f;
    c2.y = 0.0f;
    c2.width = 64.0f;
    c2.height = 32.0f;

    game_character c3;
    c3.attrs.set(char_attr::block);
    c3.x = 16.0f;
    c3.y = 64.0f;
    c3.width = 32.0f;
    c3.height = 32.0f;

    const float x1[] =
    {
        48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f,
        48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f, 48.0f
    };

    {
        game_system game;
        game.characters.push_back(c1);
        game.characters.push_back(c2);
        game.characters.push_back(c3);
        move_check_x1(game, x1);
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
    test->add(BOOST_TEST_CASE(&dash_test));
    test->add(BOOST_TEST_CASE(&dodge_block_test));
    return test;
}
