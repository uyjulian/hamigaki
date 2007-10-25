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
};

inline bool intersect_rects(const rectangle& r1, const rectangle& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y < r2.y + r2.ly) && (r2.y < r1.y + r1.ly) ;
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


struct input_command
{
};


#include <hamigaki/coroutine/shared_coroutine.hpp>

class game_system;
class character;

typedef hamigaki::coroutines::shared_coroutine<
    void (game_system*, character*)
> routine_type;

class character
{
public:
    character() : width_(0.0f), height_(0.0f)
    {
    }

    point position() const
    {
        return pos_;
    }

    void position(const point& pos)
    {
        pos_ = pos;
    }

    float width() const
    {
        return width_;
    }

    void width(float w)
    {
        width_ = w;
    }

    float height() const
    {
        return height_;
    }

    void height(float h)
    {
        height_ = h;
    }

    rectangle bounds() const
    {
        return make_bounds(pos_, width_, height_);
    }

    void routine(const routine_type& x)
    {
        routine_ = x;
    }

    void move(game_system& game)
    {
        if (!routine_.empty())
            routine_(&game, this);
    }

private:
    point pos_;
    float width_;
    float height_;
    routine_type routine_;
};


#include <list>

typedef std::list<character> character_list;
typedef character_list::iterator character_iterator;

class game_system
{
public:
    character_list& characters()
    {
        return chars_;
    }

private:
    character_list chars_;
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
    float vy = 0.0f;

    while (true)
    {
        vy += gravity;
        vy = (std::max)(vy, -10.0f);

        point pos = c->position();
        pos.y += vy;

        rectangle r = make_bounds(pos, c->width(), c->height());

        character_list& ls = game->characters();
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            const rectangle& r2 = i->bounds();

            if (intersect_rects(r, r2))
            {
                vy = 0.0f;
                pos.y = r2.y + r2.ly;
                r = make_bounds(pos, c->width(), c->height());
            }
        }

        c->position(pos);

        boost::tie(game,c) = self.yield();
    }
}

void lift_routine(
    routine_type::self& self, game_system* game, character* c)
{
    const float vy = 2.0f;
    const float max_y = 480.0f;
    const float min_y = -32.0f;

    while (true)
    {
        point pos = c->position();
        pos.y += vy;
        if (pos.y > max_y)
            pos.y = min_y;

        rectangle r = make_bounds(pos, c->width(), c->height());

        character_list& ls = game->characters();
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            const rectangle& r2 = i->bounds();

            if (intersect_rects(r, r2))
            {
                point pos2 = i->position();
                pos2.y = r.y + r.ly;
                i->position(pos2);
            }
        }

        c->position(pos);

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
        c1.position(point(16.0f, 0.0f));
        c1.width(32.0f);
        c1.height(32.0f);
        c1.routine(routine_type(&lift_routine));
        game.characters().push_back(c1);

        character c2;
        c2.position(point(16.0f, 64.0f));
        c2.width(32.0f);
        c2.height(32.0f);
        c2.routine(routine_type(&gravity_routine));
        game.characters().push_back(c2);

        character_list& ls = game.characters();
        for (int i = 0; i < 20; ++i)
        {
            std::for_each(
                ls.begin(), ls.end(),
                boost::bind(&character::move, _1, boost::ref(game))
            );

            std::cout
                << ls.front().position().y
                << '\t'
                << ls.back().position().y
                << std::endl;
        }

    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
