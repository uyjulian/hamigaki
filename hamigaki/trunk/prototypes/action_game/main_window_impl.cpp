// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "blink_effect.hpp"
#include "bounce_routine.hpp"
#include "collision_utility.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "fire_man_routine.hpp"
#include "game_character.hpp"
#include "game_system.hpp"
#include "hop_routine.hpp"
#include "hop_step_jump_routine.hpp"
#include "item_box_routine.hpp"
#include "lift_routine.hpp"
#include "player_routine.hpp"
#include "png_loader.hpp"
#include "pop_up_routine.hpp"
#include "side_scrolling_routine.hpp"
#include "sprite.hpp"
#include "texture_cache.hpp"
#include "turn_routine.hpp"
#include "vanish_routine.hpp"
#include "velocity_routine.hpp"
#include "wait_se_routine.hpp"
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <cmath>
#include <list>

#if !defined(NDEBUG)
    #define HAMIGAKI_DISPLAY_FPS
#endif

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

} // namespace depth

const boost::uint32_t miss_form = static_four_char_code<'M','I','S','S'>::value;

struct character_ptr_z_greator
{
    bool operator()(const character_ptr& lhs, const character_ptr& rhs) const
    {
        return lhs->z > rhs->z;
    }
};

character_ptr
create_character(int x, int y,float z, bool back, const sprite_info_set& infos)
{
    character_ptr c(new game_character);

    const sprite_info& info = infos.get_group(c->form)[0];

    c->sprite_infos = &infos;
    c->x = static_cast<float>(x * 32 + 16);
    c->y = static_cast<float>(y * 32);
    c->z = z;
    c->width = static_cast<float>(info.bounds.lx);
    c->height = static_cast<float>(info.bounds.ly);
    c->origin.first = x;
    c->origin.second = y;
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

        if ((r.x <= c2->x) && (c2->x < r.x + r.lx) && (c2->y == r.y + r.ly))
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

    int x, y;
    boost::tie(x,y) = c->origin;

    if ((x != -1) && (y != -1))
        game->map.erase(x, y);

    float dx[] = {-12.0f, -12.0f, 12.0f, 12.0f };
    float dy[] = { 32.0f,  64.0f, 32.0f, 64.0f };
    float vx[] = { -2.0f,  -2.0f,  2.0f,  2.0f };
    float vy[] = {  2.0f,   4.0f,  2.0f,  4.0f };

    const sprite_info_set& infos = game->sprites["fragment.txt"];

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

    int x, y;
    boost::tie(x,y) = c->origin;

    if ((x != -1) && (y != -1))
        game->map.replace(x, y, 'm');

    c->change_sprite(game->sprites["used_block.txt"]);
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
    if (target->sprite_infos == &game->sprites["boy.txt"])
    {
        target->attrs.set(char_attr::breaker);
        target->change_sprite(game->sprites["man.txt"]);
    }

    c->removed = true;
}

void to_fire_man(game_system* game, game_character* c, game_character* target)
{
    if (target->sprite_infos != &game->sprites["fire_man.txt"])
    {
        target->attrs.set(char_attr::breaker);
        target->change_sprite(game->sprites["fire_man.txt"]);
        target->speed_routine = fire_man_routine();
    }

    c->removed = true;
}

void power_down(game_system* game, game_character* c, game_character* target)
{
    if (target->effect)
        return;

    if (target->attrs.test(char_attr::breaker))
    {
        game->sound.play_se("damage.ogg");
        if (target->sprite_infos == &game->sprites["man.txt"])
        {
            target->attrs.reset(char_attr::breaker);
            target->change_sprite(game->sprites["boy.txt"]);
        }
        else
        {
            target->change_sprite(game->sprites["man.txt"]);
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
    int x, y;
    boost::tie(x,y) = c->origin;

    if ((x != -1) && (y != -1))
        game->map.replace(x, y, 'm');

    c->change_sprite(game->sprites["used_block.txt"]);
    c->on_hit_from_below.clear();


    game_character item;

    if (target->sprite_infos == &game->sprites["boy.txt"])
    {
        const sprite_info_set& infos = game->sprites["milk.txt"];
        const sprite_info& info = infos.get_group(item.form)[0];

        item.sprite_infos = &infos;
        item.width = static_cast<float>(info.bounds.lx);
        item.height = static_cast<float>(info.bounds.ly);
        item.vx = 2.0f;
        item.on_collide_block_side = &turn;
        item.on_hit = &hop;
        item.on_collide_player = &to_man;
    }
    else
    {
        const sprite_info_set& infos = game->sprites["capsule.txt"];
        const sprite_info& info = infos.get_group(item.form)[0];

        item.sprite_infos = &infos;
        item.width = static_cast<float>(info.bounds.lx);
        item.height = static_cast<float>(info.bounds.ly);
        item.on_collide_player = &to_fire_man;
    }

    item.z = layer::enemy;
    item.back = false;

    c->move_routine = item_box_routine(item);
}

void stomp(game_system* game, game_character* c, game_character* target)
{
    int x, y;
    boost::tie(x,y) = c->origin;

    if ((x != -1) && (y != -1))
        game->map.replace(x, y, ' ');

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

    int x, y;
    boost::tie(x,y) = c->origin;

    if ((x != -1) && (y != -1))
        game->map.replace(x, y, ' ');

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

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , input_(handle_), system_(handle_), textures_(device_)
        , active_(false), last_time_(::GetTickCount()), frames_(0)
#if defined(HAMIGAKI_DISPLAY_FPS)
        , last_fps_time_(0), fps_count_(0)
#endif
        , stage_file_("map.txt")
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);
        width_ = static_cast<int>(cr.right);
        height_ = static_cast<int>(cr.bottom);

        reset_characters();
    }

    ~impl()
    {
    }

    void stage_file(const std::string& filename)
    {
        stage_file_ = filename;
        reset_characters();
    }

    void connect_d3d_device()
    {
        ::D3DPRESENT_PARAMETERS params; 
        std::memset(&params, 0, sizeof(params));
        params.Windowed = TRUE;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.BackBufferFormat = D3DFMT_UNKNOWN;
        params.EnableAutoDepthStencil = TRUE;
        params.AutoDepthStencilFormat = D3DFMT_D16;

        device_ = d3d_.create_device(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle_,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, params);

        last_time_ = ::GetTickCount();
#if defined(HAMIGAKI_DISPLAY_FPS)
        last_fps_time_ = last_time_;
#endif
    }

    bool process_input()
    {
        if (active_)
            system_.command  = input_();
        else
            system_.command  = input_command();

        const unsigned long table[] = { 16, 17, 17 };
        unsigned long now = ::GetTickCount();
        unsigned long elapsed = now - last_time_;
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
            int fps = fps_count_*1000/(now-last_fps_time_);
            ::wsprintfA(buf, "FPS = %d", fps);
            ::SetWindowTextA(handle_, buf);

            last_fps_time_ = now;
            fps_count_ = 0;
        }
#endif

        return updated;
    }

    void render()
    {
        system_.characters.sort(character_ptr_z_greator());

        device_.clear(D3DCOLOR_XRGB(0x77,0x66,0xDD));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            device_.set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            std::for_each(
                boost::make_indirect_iterator(system_.characters.begin()),
                boost::make_indirect_iterator(system_.characters.end()),
                boost::bind(&impl::draw_character, this, _1)
            );

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        device_.present();
    }

    void active(bool val)
    {
        active_ = val;
    }

private:
    ::HWND handle_;
    input_engine input_;
    game_system system_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    texture_cache textures_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
#if defined(HAMIGAKI_DISPLAY_FPS)
    unsigned long last_fps_time_;
    int fps_count_;
#endif
    std::string stage_file_;
    character_ptr player_;
    character_ptr scroll_;
    int width_;
    int height_;

    character_iterator find_enemy(int x, int y)
    {
        character_list& ls = system_.characters;
        for (character_iterator i = ls.begin(); i != ls.end(); ++i)
        {
            if (((*i)->origin.first == x) && ((*i)->origin.second == y))
                return i;
        }

        return ls.end();
    }

    void add_character(int x, int y, char type)
    {
        if (type == ' ')
            return;

        if (find_enemy(x, y) != system_.characters.end())
            return;

        bool back = player_->x <= static_cast<float>(x*32);

        if (type == 'o')
        {
            character_ptr c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c->attrs.set(char_attr::enemy);
            c->vx = back ? -1.0f : 1.0f;
            c->move_routine = &velocity_routine;
            c->on_collide_block_side = &turn;
            c->on_collide_player = &power_down;
            c->on_collide_enemy = &turn;
            c->on_stomp = &stomp;
            c->on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'a')
        {
            character_ptr c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c->attrs.set(char_attr::enemy);
            c->vx = back ? -1.0f : 1.0f;
            c->move_routine = &velocity_routine;
            c->speed_routine = &turn_routine;
            c->on_collide_block_side = &turn;
            c->on_collide_player = &power_down;
            c->on_collide_enemy = &turn;
            c->on_stomp = &stomp;
            c->on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'p')
        {
            character_ptr c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c->attrs.set(char_attr::enemy);
            c->vx = back ? -2.0f : 2.0f;
            c->move_routine = &velocity_routine;
            c->speed_routine = hop_routine();
            c->on_collide_block_side = &turn;
            c->on_collide_player = &power_down;
            c->on_stomp = &stomp;
            c->on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'w')
        {
            character_ptr c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c->attrs.set(char_attr::enemy);
            c->vx = back ? -2.0f : 2.0f;
            c->move_routine = &velocity_routine;
            c->speed_routine = hop_step_jump_routine();
            c->on_collide_block_side = &turn;
            c->on_collide_player = &power_down;
            c->on_stomp = &stomp;
            c->on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'U')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["lift.txt"]
                );
            c->attrs.set(char_attr::block);
            c->vy = 2.0f;
            c->move_routine = &loop_lift_routine;
            system_.characters.push_back(c);
        }
        else if (type == 'D')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["lift.txt"]
                );
            c->attrs.set(char_attr::block);
            c->vy = -2.0f;
            c->move_routine = &loop_lift_routine;
            system_.characters.push_back(c);
        }
        else if (type == '=')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["brick_block.txt"]
                );
            c->attrs.set(char_attr::block);
            c->on_hit_from_below = &to_fragments;
            system_.characters.push_back(c);
        }
        else if (type == 'G')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["brick_block.txt"]
                );
            c->attrs.set(char_attr::block);
            c->on_hit_from_below = &to_used_block;
            system_.characters.push_back(c);
        }
        else if (type == 'I')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["brick_block.txt"]
                );
            c->attrs.set(char_attr::block);
            c->on_hit_from_below = &pop_up_item;
            system_.characters.push_back(c);
        }
        else if (type == 'm')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["used_block.txt"]
                );
            c->attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if (type == '$')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["item_box.txt"]
                );
            c->attrs.set(char_attr::block);
            c->on_hit_from_below = &to_used_block;
            system_.characters.push_back(c);
        }
        else if (type == '?')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["item_box.txt"]
                );
            c->attrs.set(char_attr::block);
            c->on_hit_from_below = &pop_up_item;
            system_.characters.push_back(c);
        }
        else if (type == 'S')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["secret_block.txt"]
                );
            c->on_collide_player = &secret_block;
            system_.characters.push_back(c);
        }
        else if (type == '/')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["left_down.txt"]
                );
            c->attrs.set(char_attr::block);
            c->slope = slope_type::left_down;
            system_.characters.push_back(c);
        }
        else if (type == '\\')
        {
            character_ptr c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["right_down.txt"]
                );
            c->attrs.set(char_attr::block);
            c->slope = slope_type::right_down;
            system_.characters.push_back(c);
        }
    }

    void erase_old_characters()
    {
        float scroll_x1 = scroll_->x;
        float scroll_x2 = scroll_x1 + width_;

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

            if (c->origin.first == -1)
            {
                if ((right < scroll_x1) || (left > scroll_x2))
                    system_.characters.erase(i);
            }
            else
            {
                float origin = c->origin.first * 32.0f;
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

        load_map_from_text(stage_file_.c_str(), system_.map);

        player_.reset(new game_character);
        player_->move_routine = &player_routine;
        player_->speed_routine = user_control_routine();
        player_->on_collide_block_side = &stop;
        player_->sprite_infos = &system_.sprites["boy.txt"];
        player_->attrs.set(char_attr::player);
        player_->back = false;

        std::pair<int,int> pos = system_.map.player_position();

        sprite_info info =
            player_->sprite_infos->get_group(player_->form)[0];

        player_->x = static_cast<float>(pos.first * 32 + 16);
        player_->y = static_cast<float>(pos.second * 32);
        player_->z = layer::player;
        player_->width = static_cast<float>(info.bounds.lx);
        player_->height = static_cast<float>(info.bounds.ly);

        system_.characters.clear();
        system_.characters.push_back(player_);

        scroll_.reset(new game_character);
        scroll_->move_routine = side_scrolling_routine(player_);
        scroll_->z = layer::hidden;
        system_.characters.push_back(scroll_);

        scroll_->move_routine(&system_, scroll_.get());

        int min_x = static_cast<int>(scroll_->x) / 32 - 3;
        int max_x = static_cast<int>(scroll_->x) / 32 + width_ / 32 + 3;
        for (int y = 0; y < system_.map.height(); ++y)
        {
            for (int x = min_x; x < max_x; ++x)
                add_character(x, y, system_.map(x, y));
        }

        system_.characters.sort(character_ptr_z_greator());

        system_.sound.play_bgm("bgm.ogg");
    }

    void process_input_impl()
    {
        character_list& ls = system_.characters;

        if (system_.command.reset)
        {
            reset_characters();
            return;
        }

        int old_scroll_block = static_cast<int>(scroll_->x / 32.0f);

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

        if ((player_->form == miss_form) && player_->effect.empty())
        {
            reset_characters();
            return;
        }
        else if (player_->y < - player_->height - 32.0f)
        {
            if (player_->form != miss_form)
            {
                system_.sound.stop_bgm();
                player_->change_sprite(system_.sprites["boy.txt"]);
                player_->change_form(miss_form);
                player_->effect = wait_se_routine("miss.ogg");
            }
            player_->y = -player_->height - 32.0f;
            player_->move_routine.clear();
        }

        int scroll_block = static_cast<int>(scroll_->x / 32.0f);
        if (scroll_block > old_scroll_block)
        {
            int min_x = old_scroll_block + width_/32 + 3;
            int max_x = scroll_block + width_/32 + 2;

            for (int x = min_x; x <= max_x; ++x)
            {
                for (int y = 0; y < system_.map.height(); ++y)
                    add_character(x, y, system_.map(x, y));
            }
        }
        else if (scroll_block < old_scroll_block)
        {
            int max_x = old_scroll_block - 3;
            int min_x = scroll_block - 2;

            for (int x = max_x; x >= min_x; --x)
            {
                for (int y = 0; y < system_.map.height(); ++y)
                    add_character(x, y, system_.map(x, y));
            }
        }

        erase_old_characters();
    }

    void draw_character(const game_character& c)
    {
        if (!c.sprite_infos)
            return;

        const sprite_info_set& infos = *(c.sprite_infos);

        const std::vector<sprite_info>& group = infos.get_group(c.form);

        std::size_t pattern = (c.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        const rect& tr = c.texture_rect();

        float x = tr.x - scroll_->x;
        float y = height_ - tr.y - tr.ly;

        const std::string& texture = infos.texture();
        if (!texture.empty())
        {
            ::draw_sprite(
                device_, x, y, c.z, textures_[texture],
                info.x, info.y, infos.width(), infos.height(),
                c.back, c.color
            );
        }
    }
};

main_window::main_window(::HWND handle) : pimpl_(new impl(handle))
{
}

void main_window::stage_file(const std::string& filename)
{
    pimpl_->stage_file(filename);
}

void main_window::connect_d3d_device()
{
    pimpl_->connect_d3d_device();
}

bool main_window::process_input()
{
    return pimpl_->process_input();
}

void main_window::render()
{
    pimpl_->render();
}

void main_window::active(bool val)
{
    pimpl_->active(val);
}
