// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "blink_effect.hpp"
#include "bounce_routine.hpp"
#include "character_repository.hpp"
#include "collision_utility.hpp"
#include "draw.hpp"
#include "fire_man_routine.hpp"
#include "game_character.hpp"
#include "game_system.hpp"
#include "guid_io.hpp"
#include "hop_routine.hpp"
#include "hop_step_jump_routine.hpp"
#include "item_box_routine.hpp"
#include "lift_routine.hpp"
#include "player_routine.hpp"
#include "pipe_routine.hpp"
#include "pop_up_routine.hpp"
#include "render_context.hpp"
#include "side_scrolling_routine.hpp"
#include "sprite.hpp"
#include "stage_map_load.hpp"
#include "texture_cache.hpp"
#include "turn_routine.hpp"
#include "vanish_routine.hpp"
#include "velocity_routine.hpp"
#include "wait_se_routine.hpp"
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/next_prior.hpp>
#include <cmath>
#include <cstdio>
#include <list>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#else
    #include <time.h>
#endif

#if !defined(NDEBUG)
    #define HAMIGAKI_DISPLAY_FPS
#endif

namespace fs = boost::filesystem;

namespace
{

namespace layer
{
    const float back    = 1.00f;
    const float block   = 0.75f;
    const float enemy   = 0.50f;
    const float player  = 0.25f;
    const float front   = 0.00f;
    const float hidden  =-1.00f;

} // namespace layer

#if defined(BOOST_WINDOWS)
boost::uint64_t get_tick_count()
{
    return ::GetTickCount();
}
#else
boost::uint64_t get_tick_count()
{
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<boost::uint64_t>(ts.tv_sec)*1000 + ts.tv_nsec/1000000;
}
#endif

float color_byte_to_float(unsigned char n)
{
    return static_cast<float>(n) / 255.0f;
}

void set_clear_color(unsigned long color)
{
    ::glClearColor(
        color_byte_to_float((color >> 16) & 0xFF),
        color_byte_to_float((color >>  8) & 0xFF),
        color_byte_to_float((color      ) & 0xFF),
        color_byte_to_float((color >> 24) & 0xFF)
    );
}

const boost::uint32_t miss_form = static_four_char_code<'M','I','S','S'>::value;

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

const hamigaki::uuid player_id(guids[0]);
const hamigaki::uuid up_lift_id(guids[1]);
const hamigaki::uuid down_lift_id(guids[2]);
const hamigaki::uuid enemy_o_id(guids[3]);
const hamigaki::uuid enemy_a_id(guids[4]);
const hamigaki::uuid enemy_p_id(guids[5]);
const hamigaki::uuid enemy_w_id(guids[6]);
const hamigaki::uuid brick_id(guids[7]);
const hamigaki::uuid coin_brick_id(guids[8]);
const hamigaki::uuid item_brick_id(guids[9]);
const hamigaki::uuid used_block_id(guids[10]);
const hamigaki::uuid coin_box_id(guids[11]);
const hamigaki::uuid item_box_id(guids[12]);
const hamigaki::uuid secret_coin_id(guids[13]);
const hamigaki::uuid left_down_id(guids[14]);
const hamigaki::uuid right_down_id(guids[15]);

struct character_ptr_z_greator
{
    bool operator()(const character_ptr& lhs, const character_ptr& rhs) const
    {
        return lhs->z > rhs->z;
    }
};

character_ptr create_character(
    const map_element& e, float z, bool back, const sprite_info_set& infos)
{
    character_ptr c(new game_character);

    const sprite_group& grp = infos.get_group(c->form);

    c->sprite_infos = &infos;
    c->x = static_cast<float>(e.x);
    c->y = static_cast<float>(e.y);
    c->z = z;
    c->width = static_cast<float>(grp.bound_width);
    c->height = static_cast<float>(grp.bound_height);
    c->origin = e;
    c->back = back;

    return c;
}

void stop(game_system* game, game_character* c, game_character* target)
{
    c->vx = 0.0f;
}

void turn(game_system* game, game_character* c, game_character* target)
{
    float dx = c->x - target->x;
    if ((dx < 0.0f) && (c->vx > 0.0f))
    {
        c->vx = -c->vx;
        c->back = true;
    }
    else if ((dx > 0.0f) && (c->vx < 0.0f))
    {
        c->vx = -c->vx;
        c->back = false;
    }
}

void hit_on_block(game_system& game, game_character& c)
{
    const rect& r = c.bounds();

    character_list& ls = game.characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        game_character* c2 = i->get();

        // itself
        if (c2 == &c)
            continue;

        if (c2->attrs.test(char_attr::block) || c2->on_hit.empty())
            continue;

        const rect& r2 = c2->bounds();
        if ((r.x < r2.x+r2.lx) && (r2.x <= r.x+r.lx) && (r2.y == r.y + r.ly))
            c2->on_hit(&game, c2, &c);
    }
}

void to_fragments(
    game_system* game, game_character* c, game_character* target)
{
    hit_on_block(*game, *c);

    if (!target->attrs.test(char_attr::breaker))
    {
        c->move_routine = bounce_routine();
        return;
    }

    if (!c->origin.type.is_null())
        game->map.elements.get<0>().erase(c->origin.x_y());

    float dx[] = {-12.0f, -12.0f, 12.0f, 12.0f };
    float dy[] = { 32.0f,  64.0f, 32.0f, 64.0f };
    float vx[] = { -2.0f,  -2.0f,  2.0f,  2.0f };
    float vy[] = {  2.0f,   4.0f,  2.0f,  4.0f };

    const sprite_info_set& infos = game->sprites["fragment.ags-yh"];

    for (std::size_t i = 0; i < 4; ++i)
    {
        character_ptr fr(new game_character);

        fr->move_routine = &velocity_routine;
        fr->sprite_infos = &infos;
        fr->x = c->x + dx[i];
        fr->y = c->y + dy[i];
        fr->z = layer::front;
        fr->width = 0.0f;
        fr->height = 0.0f;
        fr->vx = vx[i];
        fr->vy = vy[i];
        fr->back = false;

        game->new_characters.push_back(fr);
    }

    game->sound.play_se("break_block.ogg");

    c->removed = true;
}

void to_used_block(
    game_system* game, game_character* c, game_character* target)
{
    hit_on_block(*game, *c);

    if (!c->origin.type.is_null())
    {
        // FIXME: should use replace
        game->map.elements.get<0>().erase(c->origin.x_y());
        game->map.elements.insert(
            map_element(c->origin.x, c->origin.y, hamigaki::uuid(used_block_id))
        );
    }

    c->change_sprite(game->sprites["used_block.ags-yh"]);
    c->move_routine = bounce_routine();
    c->on_hit_from_below.clear();
}

void secret_block(
    game_system* game, game_character* c, game_character* target)
{
    if (target->vy > 0.0f)
    {
        target->y = c->y - target->height;
        target->vy = -target->vy * 0.5f;
        c->attrs.set(char_attr::block);
        to_used_block(game, c, target);
    }
}

void to_man(game_system* game, game_character* c, game_character* target)
{
    if (target->sprite_infos == &game->sprites["boy.ags-yh"])
    {
        target->attrs.set(char_attr::breaker);
        target->change_sprite(game->sprites["man.ags-yh"]);

        game->effect = wait_se_routine("power_up.ogg");
        game->effect_target = target;
    }
    else
        game->sound.play_se("power_up.ogg");

    c->removed = true;
}

void to_fire_man(game_system* game, game_character* c, game_character* target)
{
    if (target->sprite_infos != &game->sprites["fire_man.ags-yh"])
    {
        target->attrs.set(char_attr::breaker);
        target->change_sprite(game->sprites["fire_man.ags-yh"]);
        target->speed_routine = fire_man_routine();

        game->effect = wait_se_routine("power_up.ogg");
        game->effect_target = target;
    }
    else
        game->sound.play_se("power_up.ogg");

    c->removed = true;
}

void power_down(game_system* game, game_character* c, game_character* target)
{
    if (target->effect)
        return;

    if (target->attrs.test(char_attr::breaker))
    {
        game->sound.play_se("damage.ogg");
        if (target->sprite_infos == &game->sprites["man.ags-yh"])
        {
            target->attrs.reset(char_attr::breaker);
            target->change_sprite(game->sprites["boy.ags-yh"]);
        }
        else
        {
            target->change_sprite(game->sprites["man.ags-yh"]);
            target->speed_routine = user_control_routine();
        }
        target->effect = blink_effect();
    }
    else
    {
        game->sound.stop_bgm();
        target->vx = 0.0f;
        target->vy = 10.0f;
        target->change_form(miss_form);
        target->effect = wait_se_routine("miss.ogg");
    }
}

void hop(game_system* game, game_character* c, game_character* target)
{
    c->vy = 10.0f;
}

void pop_up_item(game_system* game, game_character* c, game_character* target)
{
    if (!c->origin.type.is_null())
    {
        // FIXME: should use replace
        game->map.elements.get<0>().erase(c->origin.x_y());
        game->map.elements.insert(
            map_element(c->origin.x, c->origin.y, hamigaki::uuid(used_block_id))
        );
    }

    c->change_sprite(game->sprites["used_block.ags-yh"]);
    c->on_hit_from_below.clear();


    game_character item;

    if (target->sprite_infos == &game->sprites["boy.ags-yh"])
    {
        const sprite_info_set& infos = game->sprites["milk.ags-yh"];
        const sprite_group& grp = infos.get_group(item.form);

        item.sprite_infos = &infos;
        item.width = static_cast<float>(grp.bound_width);
        item.height = static_cast<float>(grp.bound_height);
        item.vx = 2.0f;
        item.on_collide_block_side = &turn;
        item.on_hit = &hop;
        item.on_collide_player = &to_man;
    }
    else
    {
        const sprite_info_set& infos = game->sprites["capsule.ags-yh"];
        const sprite_group& grp = infos.get_group(item.form);

        item.sprite_infos = &infos;
        item.width = static_cast<float>(grp.bound_width);
        item.height = static_cast<float>(grp.bound_height);
        item.on_collide_player = &to_fire_man;
    }

    item.z = layer::enemy;
    item.back = false;

    c->move_routine = item_box_routine(item);
}

void stomp(game_system* game, game_character* c, game_character* target)
{
    if (!c->origin.type.is_null())
        game->map.elements.get<0>().erase(c->origin.x_y());

    c->change_form(static_four_char_code<'S','T','M','P'>::value);
    c->move_routine = vanish_routine(5);
    c->speed_routine.clear();
    c->vx = 0.0f;
    c->vy = 0.0f;
    game->sound.play_se("stomp.ogg");

    target->vy = 8.0f;
}

void hit(game_system* game, game_character* c, game_character* target)
{
    if (c->attrs.test(char_attr::enemy) && target->on_collide_enemy)
        target->on_collide_enemy(game, target, c);

    if (!c->origin.type.is_null())
        game->map.elements.get<0>().erase(c->origin.x_y());

    c->change_form(miss_form);
    c->move_routine = &velocity_routine;
    c->speed_routine.clear();
    c->effect = vanish_routine(20);
    c->y += 16.0f;
    c->vy = 8.0f;

    c->vx = 4.0f;
    if (target->vx < 0.0f)
        c->vx = -c->vx;
    else if ((target->vx == 0.0f) && (c->x < target->x))
        c->vx = -c->vx;
}

void transfer(
    game_system* game, game_character* c, game_character* target)
{
    transfer_info_table::iterator it =
        game->transfer_table.find(std::make_pair(c->origin.x, c->origin.y));
    if (it == game->transfer_table.end())
        return;

    const rect& r = c->bounds();
    const rect& r2 = target->bounds();

    transfer_info info = it->second;

    if (info.enter_dir == transfer_info::down)
    {
        if ((game->command.x != 0.0f) || (game->command.y >= 0.0f))
            return;

        if ((r.x > r2.x) || (r2.x+r2.lx > r.x+r.lx) || (r2.y != r.y + r.ly))
            return;
    }
    else if (info.enter_dir == transfer_info::up)
    {
        if (game->command.y <= 0.0f)
            return;

        if ((r.x > r2.x) || (r2.x+r2.lx > r.x+r.lx) || (r2.y + r2.ly != r.y))
            return;
    }
    else if (info.enter_dir == transfer_info::left)
    {
        if (game->command.x >= 0.0f)
            return;

        if ((r.y > r2.y) || (r2.y+r2.ly > r.y+r.ly) || (r2.x != r.x + r.lx))
            return;
    }
    else if (info.enter_dir == transfer_info::right)
    {
        if (game->command.x <= 0.0f)
            return;

        if ((r.y > r2.y) || (r2.y+r2.ly > r.y+r.ly) || (r2.x + r2.lx != r.x))
            return;
    }

    target->change_form(sprite_form::normal);
    if (info.leave_dir == transfer_info::up)
        info.y -= static_cast<int>(target->height);
    else if (info.leave_dir == transfer_info::left)
        info.x += static_cast<int>(target->width);
    else if (info.leave_dir == transfer_info::right)
        info.x -= static_cast<int>(target->width);

    game->effect = pipe_routine(info);
    game->effect_target = target;
}

const hamigaki::uuid
    loop_lift_routine_id("2df594a8-470b-4125-ade3-0fda70a2ad1d");
const hamigaki::uuid
    velocity_routine_id("461c9bc7-803a-41d6-95c2-0635eb4cc57c");

const hamigaki::uuid hop_routine_id("453dc7fe-0039-4f5e-9337-8866834d4b49");
const hamigaki::uuid
    hop_step_jump_routine_id("a67e5469-c71e-4a48-a5e3-5c7e796e7282");
const hamigaki::uuid turn_routine_id("352f6c96-62a6-4c9a-922e-84d45fcfb90c");

const hamigaki::uuid hit_id("6a199951-2b1c-45bb-9ec0-ff552996e5a3");
const hamigaki::uuid pop_up_item_id("bad301d1-2a1c-4156-a125-35746dac4883");
const hamigaki::uuid power_down_id("bd31f66d-891b-4ed4-aec1-327cff2c01bc");
const hamigaki::uuid secret_block_id("52cdb853-e6f3-48ca-a12a-9de6e7f6e76b");
const hamigaki::uuid stomp_id("122ab3e8-c2e5-429f-8088-0c8b7b7107bb");
const hamigaki::uuid to_fragments_id("3cb2edaf-4af1-498b-8025-9527a8b13808");
const hamigaki::uuid to_used_block_id("c2209d0a-811e-4f31-8066-24f4cd998296");
const hamigaki::uuid transfer_id("31aaa83d-5a2d-45e0-910d-74afe0e173f4");
const hamigaki::uuid turn_id("796cff8f-1db7-4d80-b7bd-ac2ed12ecca0");

move_routine_type find_move_routine(const hamigaki::uuid& id)
{
    if (id == loop_lift_routine_id)
        return &loop_lift_routine;
    else if (id == velocity_routine_id)
        return &velocity_routine;
    else
        return move_routine_type();
}

speed_routine_type find_speed_routine(const hamigaki::uuid& id)
{
    if (id == hop_routine_id)
        return hop_routine();
    else if (id == hop_step_jump_routine_id)
        return hop_step_jump_routine();
    else if (id == turn_routine_id)
        return &turn_routine;
    else
        return speed_routine_type();
}

collision_event_type find_collision_event(const hamigaki::uuid& id)
{
    if (id == hit_id)
        return &hit;
    else if (id == pop_up_item_id)
        return &pop_up_item;
    else if (id == power_down_id)
        return &power_down;
    else if (id == secret_block_id)
        return &secret_block;
    else if (id == stomp_id)
        return &stomp;
    else if (id == to_fragments_id)
        return &to_fragments;
    else if (id == to_used_block_id)
        return &to_used_block;
    else if (id == transfer_id)
        return &transfer;
    else if (id == turn_id)
        return &turn;
    else
        return collision_event_type();
}

} // namespace

class main_window::impl
{
public:
    explicit impl(GtkWidget* widget, const game_project& proj)
        : widget_(widget), project_(proj)
        , input_(widget_), system_(widget_), rc_(widget_), textures_(rc_)
        , last_time_(get_tick_count()), frames_(0)
#if defined(HAMIGAKI_DISPLAY_FPS)
        , last_fps_time_(0), fps_count_(0)
#endif
    {
        ::gtk_window_set_title(GTK_WINDOW(widget_), "ActionGame");

        rc_.select();

        ::glOrtho(
            0.0, static_cast<double>(project_.screen_width),
            static_cast<double>(project_.screen_height), 0.0,
            0.0, 1.0
        );

        system_.gravity = boost::lexical_cast<float>(project_.gravity);
        system_.min_vy = boost::lexical_cast<float>(project_.min_vy);
        system_.screen_width = project_.screen_width;
        system_.screen_height = project_.screen_height;

        if (project_.start_map.empty())
            project_.start_map = "map.agm-yh";

        stage_file_ = project_.start_map;

        reset_characters();

        last_time_ = get_tick_count();
#if defined(HAMIGAKI_DISPLAY_FPS)
        last_fps_time_ = last_time_;
#endif
    }

    ~impl()
    {
    }

    bool process_input()
    {
        if (::gtk_window_is_active(GTK_WINDOW(widget_)))
            system_.command  = input_();
        else
            system_.command  = input_command();

        const unsigned long table[] = { 16, 17, 17 };
        boost::uint64_t now = get_tick_count();
        if (now < last_time_)
            last_time_ = now;
        unsigned long elapsed = static_cast<unsigned long>(now - last_time_);
        bool updated = false;
        while (elapsed >= table[frames_ % 3])
        {
            elapsed -= table[frames_ % 3];
            if (++frames_ == 60)
                frames_ = 0;
            this->process_input_impl();
            updated = true;
        }
        last_time_ = now - elapsed;

#if defined(HAMIGAKI_DISPLAY_FPS)
        if (updated)
            ++fps_count_;

        if (now - last_fps_time_ >= 1000)
        {
            char buf[64];
            int fps = fps_count_*1000/static_cast<int>(now-last_fps_time_);
            std::sprintf(buf, "FPS = %d", fps);
            ::gtk_window_set_title(GTK_WINDOW(widget_), buf);

            last_fps_time_ = now;
            fps_count_ = 0;
        }
#endif

        return updated;
    }

    void render()
    {
        system_.characters.sort(character_ptr_z_greator());

        rc_.select();

        set_clear_color(project_.bg_color);
        ::glClearDepth(1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT);

        ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ::glEnable(GL_BLEND);

        ::glEnable(GL_TEXTURE_2D);
        std::for_each(
            boost::make_indirect_iterator(system_.characters.begin()),
            boost::make_indirect_iterator(system_.characters.end()),
            boost::bind(&impl::draw_character, this, _1)
        );
        ::glDisable(GL_TEXTURE_2D);

        rc_.swap_buffers();
    }

    void active(bool val)
    {
        active_ = val;
    }

private:
    GtkWidget* widget_;
    game_project project_;
    input_engine input_;
    game_system system_;
    hamigaki::render_context rc_;
    texture_cache textures_;
    character_repository char_repository_;
    bool active_;
    boost::uint64_t last_time_;
    unsigned frames_;
#if defined(HAMIGAKI_DISPLAY_FPS)
    boost::uint64_t last_fps_time_;
    int fps_count_;
#endif
    std::string stage_file_;

    character_iterator find_enemy(const map_element& e)
    {
        character_list& ls = system_.characters;
        for (character_iterator i = ls.begin(); i != ls.end(); ++i)
        {
            if ((*i)->origin == e)
                return i;
        }

        return ls.end();
    }

    void add_character(const map_element& e)
    {
        if (find_enemy(e) != system_.characters.end())
            return;

        bool back = system_.player->x <= static_cast<float>(e.x);

        if (e.type == player_id)
            return;

        const game_character_class& cc = char_repository_[e.type];

        character_ptr c(new game_character);

        const sprite_info_set& infos = system_.sprites[cc.sprite];
        const sprite_group& grp = infos.get_group(c->form);

        c->vx = boost::lexical_cast<float>(cc.vx);
        c->vy = boost::lexical_cast<float>(cc.vy);
        c->attrs = cc.attrs;
        c->slope = cc.slope;
        c->move_routine = find_move_routine(cc.move_routine);
        c->speed_routine = find_speed_routine(cc.speed_routine);
        c->on_collide_block_side =
            find_collision_event(cc.on_collide_block_side);
        c->on_hit_from_below = find_collision_event(cc.on_hit_from_below);
        c->on_collide_player = find_collision_event(cc.on_collide_player);
        c->on_collide_enemy = find_collision_event(cc.on_collide_enemy);
        c->on_stomp = find_collision_event(cc.on_stomp);
        c->on_hit = find_collision_event(cc.on_hit);
        c->on_touch_player = find_collision_event(cc.on_touch_player);

        c->sprite_infos = &infos;
        c->x = static_cast<float>(e.x);
        c->y = static_cast<float>(e.y);
        c->width = static_cast<float>(grp.bound_width);
        c->height = static_cast<float>(grp.bound_height);
        c->origin = e;

        if (cc.attrs.test(char_attr::block))
        {
            c->z = layer::block;
            c->back = false;
        }
        else
        {
            c->z = layer::enemy;
            c->back = back;
            if (back)
                c->vx = -c->vx;
        }

        system_.characters.push_back(c);
    }

    void erase_old_characters()
    {
        float scroll_x1 = system_.camera->x;
        float scroll_x2 = scroll_x1 + system_.screen_width;

        character_iterator next;
        for (character_iterator i = system_.characters.begin();
            i != system_.characters.end(); i = next)
        {
            next = boost::next(i);

            game_character* c = i->get();
            if (c->removed || (c->y + c->height < -64.0f))
            {
                system_.characters.erase(i);
                continue;
            }

            float left  = c->x - c->width * 0.5f;
            float right = c->x + c->width * 0.5f;

            if (c->origin.type.is_null())
            {
                if ((right < scroll_x1) || (left > scroll_x2))
                    system_.characters.erase(i);
            }
            else
            {
                float origin = static_cast<float>(c->origin.x);
                if (((origin < scroll_x1) || (origin+32.0f > scroll_x2)) &&
                    ((right < scroll_x1-96.0f) || (left > scroll_x2+96.0f)) )
                {
                    system_.characters.erase(i);
                }
            }
        }
    }

    void reset_characters()
    {
        system_.sound.stop_se();

        load_map_from_binary(stage_file_.c_str(), system_.map);

        fs::path agt = fs::change_extension(stage_file_, ".agt-yh");
        if (fs::exists(agt))
        {
            load_transfer_infos(
                agt.file_string().c_str(), system_.transfer_table);
        }
        else
            system_.transfer_table.clear();

        system_.player.reset(new game_character);
        system_.player->move_routine = &player_routine;
        system_.player->speed_routine = user_control_routine();
        system_.player->on_collide_block_side = &stop;
        system_.player->sprite_infos = &system_.sprites["boy.ags-yh"];
        system_.player->attrs.set(char_attr::player);
        system_.player->back = false;

        std::pair<int,int> pos = system_.map.player_position;

        const sprite_group& grp =
            system_.player->sprite_infos->get_group(system_.player->form);

        system_.player->x = static_cast<float>(pos.first);
        system_.player->y = static_cast<float>(pos.second);
        system_.player->z = layer::player;
        system_.player->width = static_cast<float>(grp.bound_width);
        system_.player->height = static_cast<float>(grp.bound_height);

        system_.characters.clear();
        system_.characters.push_back(system_.player);

        system_.camera.reset(new game_character);
        system_.camera->move_routine = side_scrolling_routine(system_.player);
        system_.camera->z = layer::hidden;
        system_.camera->color = 0x00000000ul;
        system_.characters.push_back(system_.camera);

        system_.camera->move_routine(&system_, system_.camera.get());

        int min_x = static_cast<int>(system_.camera->x) - 32*3;
        int max_x =
            static_cast<int>(system_.camera->x) + system_.screen_width + 32*3;
        int map_height = system_.map.height;

        typedef map_elements::nth_index<0>::type x_y_index_t;
        typedef x_y_index_t::iterator iter_type;

        const x_y_index_t& x_y = system_.map.elements.get<0>();
        iter_type beg = x_y.lower_bound(std::make_pair(min_x, 0));
        iter_type end = x_y.upper_bound(std::make_pair(max_x, map_height));

        for ( ; beg != end; ++beg)
        {
            const map_element& e = *beg;
            if ((e.y >= 0) && (e.y < map_height))
                add_character(e);
        }

        system_.characters.sort(character_ptr_z_greator());

        system_.sound.play_bgm("bgm.ogg");
    }

    void change_map()
    {
        stage_file_ = system_.next_pos.map_file;
        system_.next_pos.map_file.clear();

        load_map_from_binary(stage_file_.c_str(), system_.map);

        fs::path agt = fs::change_extension(stage_file_, ".agt-yh");
        if (fs::exists(agt))
        {
            load_transfer_infos(
                agt.file_string().c_str(), system_.transfer_table);
        }
        else
            system_.transfer_table.clear();

        system_.player->x = static_cast<float>(system_.next_pos.x);
        system_.player->y = static_cast<float>(system_.next_pos.y);
        system_.camera->move_routine(&system_, system_.camera.get());

        system_.characters.clear();
        system_.characters.push_back(system_.player);
        system_.characters.push_back(system_.camera);

        int min_x = static_cast<int>(system_.camera->x) - 32*3;
        int max_x =
            static_cast<int>(system_.camera->x) + system_.screen_width + 32*3;
        int map_height = system_.map.height;

        typedef map_elements::nth_index<0>::type x_y_index_t;
        typedef x_y_index_t::iterator iter_type;

        const x_y_index_t& x_y = system_.map.elements.get<0>();
        iter_type beg = x_y.lower_bound(std::make_pair(min_x, 0));
        iter_type end = x_y.upper_bound(std::make_pair(max_x, map_height));

        for ( ; beg != end; ++beg)
        {
            const map_element& e = *beg;
            if ((e.y >= 0) && (e.y < map_height))
                add_character(e);
        }

        system_.characters.sort(character_ptr_z_greator());
    }

    void scroll(int old_scroll_x)
    {
        int scroll_x = static_cast<int>(system_.camera->x);
        if (scroll_x > old_scroll_x)
        {
            int min_x = old_scroll_x + system_.screen_width + 32*3;
            int max_x = scroll_x + system_.screen_width + 32*3;
            int map_height = system_.map.height;

            typedef map_elements::nth_index<0>::type x_y_index_t;
            typedef x_y_index_t::iterator iter_type;

            const x_y_index_t& x_y = system_.map.elements.get<0>();
            iter_type beg = x_y.lower_bound(std::make_pair(min_x, 0));
            iter_type end = x_y.upper_bound(std::make_pair(max_x, map_height));

            for ( ; beg != end; ++beg)
            {
                const map_element& e = *beg;
                if ((e.y >= 0) && (e.y < map_height))
                    add_character(e);
            }
        }
        else if (scroll_x < old_scroll_x)
        {
            int max_x = old_scroll_x - 32*3;
            int min_x = scroll_x - 32*3;
            int map_height = system_.map.height;

            typedef map_elements::nth_index<0>::type x_y_index_t;
            typedef x_y_index_t::iterator iter_type;

            const x_y_index_t& x_y = system_.map.elements.get<0>();
            iter_type beg = x_y.lower_bound(std::make_pair(min_x, 0));
            iter_type end = x_y.upper_bound(std::make_pair(max_x, map_height));

            for ( ; beg != end; ++beg)
            {
                const map_element& e = *beg;
                if ((e.y >= 0) && (e.y < map_height))
                    add_character(e);
            }
        }

        erase_old_characters();
    }

    void process_input_impl()
    {
        character_list& ls = system_.characters;

        if (system_.command.reset)
        {
            stage_file_ = project_.start_map;
            reset_characters();
            return;
        }
        else if (!system_.next_pos.map_file.empty())
        {
            if (stage_file_ != system_.next_pos.map_file)
            {
                change_map();
                return;
            }
            else
            {
                system_.next_pos.map_file.clear();

                int old_scroll_x = static_cast<int>(system_.camera->x);
                system_.player->x = static_cast<float>(system_.next_pos.x);
                system_.player->y = static_cast<float>(system_.next_pos.y);
                system_.camera->move(system_);

                scroll(old_scroll_x);
                return;
            }
        }

        if (system_.effect)
        {
            // Note:
            // This copy guarantees the lifetime until the call is completed.
            effect_type e = system_.effect;
            if (!e(&system_, system_.effect_target))
                system_.effect.clear();
            return;
        }

        int old_scroll_x = static_cast<int>(system_.camera->x);

        std::for_each(
            boost::make_indirect_iterator(ls.begin()),
            boost::make_indirect_iterator(ls.end()),
            boost::bind(&game_character::move, _1, boost::ref(system_))
        );

        std::for_each(
            boost::make_indirect_iterator(ls.begin()),
            boost::make_indirect_iterator(ls.end()),
            boost::bind(&process_collisions, boost::ref(system_), _1)
        );

        if (!system_.new_characters.empty())
            ls.splice(ls.end(), system_.new_characters);

        if ((system_.player->form == miss_form) &&
            system_.player->effect.empty() )
        {
            reset_characters();
            return;
        }
        else if (system_.player->y < - system_.player->height - 32.0f)
        {
            if (system_.player->form != miss_form)
            {
                system_.sound.stop_bgm();
                system_.player->change_sprite(system_.sprites["boy.ags-yh"]);
                system_.player->change_form(miss_form);
                system_.player->effect = wait_se_routine("miss.ogg");
            }
            system_.player->y = -system_.player->height - 32.0f;
            system_.player->move_routine.clear();
        }

        scroll(old_scroll_x);
    }

    void draw_character(const game_character& c)
    {
        if (!c.sprite_infos)
        {
            if ((c.color & 0xFF000000ul) != 0ul)
            {
                ::draw_rectangle(
                    0.0f, 0.0f, 0.0f,
                    static_cast<float>(system_.screen_width),
                    static_cast<float>(system_.screen_height),
                    c.color
                );
            }

            return;
        }

        const sprite_info_set& infos = *(c.sprite_infos);

        const sprite_group& group = infos.get_group(c.form);
        const sprite_pattern& ptn = group.patterns[c.step / infos.wait];

        const rect& tr = c.texture_rect();

        float x = tr.x - system_.camera->x;
        float y = system_.screen_height - tr.y - tr.ly;

        const std::string& texture = infos.texture;
        if (!texture.empty() && (c.z >= 0.0f))
        {
            ::draw_sprite(
                x, y, 0.0f, textures_[texture],
                ptn.x * infos.width, ptn.y * infos.height,
                infos.width, infos.height,
                c.back, c.color
            );
        }
    }
};

main_window::main_window(GtkWidget* widget, const game_project& proj)
    : pimpl_(new impl(widget, proj))
{
}

bool main_window::process_input()
{
    return pimpl_->process_input();
}

void main_window::render()
{
    pimpl_->render();
}
