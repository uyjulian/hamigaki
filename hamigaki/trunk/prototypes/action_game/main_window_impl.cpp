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
#include "game_character.hpp"
#include "game_system.hpp"
#include "hop_routine.hpp"
#include "hop_step_jump_routine.hpp"
#include "item_box_routine.hpp"
#include "lift_routine.hpp"
#include "player_routine.hpp"
#include "png_loader.hpp"
#include "pop_up_routine.hpp"
#include "sprite.hpp"
#include "texture_cache.hpp"
#include "turn_routine.hpp"
#include "vanish_routine.hpp"
#include "velocity_routine.hpp"
#include "wait_se_routine.hpp"
#include <hamigaki/functional.hpp>
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

} // namespace depth

const boost::uint32_t miss_form = static_four_char_code<'M','I','S','S'>::value;

game_character
create_character(int x, int y,float z, bool back, const sprite_info_set& infos)
{
    game_character c;

    const sprite_info& info = infos.get_group(c.form)[0];

    c.sprite_infos = &infos;
    c.x = static_cast<float>(x * 32 + 16);
    c.y = static_cast<float>(y * 32);
    c.z = z;
    c.width = static_cast<float>(info.bounds.lx);
    c.height = static_cast<float>(info.bounds.ly);
    c.origin.first = x;
    c.origin.second = y;
    c.back = back;

    return c;
}

void stop(game_system* game, game_character* c, game_character* target)
{
    c->vx = 0.0f;
}

void turn(game_system* game, game_character* c, game_character* target)
{
    c->vx = -c->vx;
    c->back = !c->back;
}

void hit_on_block(game_system& game, game_character& c)
{
    const rect& r = c.bounds();

    character_list& ls = game.characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (i->attrs.test(char_attr::block) || i->on_hit.empty())
            continue;

        if ((r.x <= i->x) && (i->x < r.x + r.lx) && (i->y == r.y + r.ly))
            i->on_hit(&game, &*i, &c);
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
        game_character fr;

        fr.move_routine = &velocity_routine;
        fr.sprite_infos = &infos;
        fr.x = c->x + dx[i];
        fr.y = c->y + dy[i];
        fr.z = layer::front;
        fr.width = 0.0f;
        fr.height = 0.0f;
        fr.vx = vx[i];
        fr.vy = vy[i];
        fr.back = false;

        game->new_characters.push_back(fr);
    }

    game->sound.play_se("break_block.ogg");

    c->y = -c->height - 128.0f;
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

void power_up(game_system* game, game_character* c, game_character* target)
{
    target->attrs.set(char_attr::breaker);
    target->change_sprite(game->sprites["man.txt"]);
    c->y = -c->height - 128.0f;
}

void power_down(game_system* game, game_character* c, game_character* target)
{
    if (target->effect)
        return;

    if (target->attrs.test(char_attr::breaker))
    {
        game->sound.play_se("damage.ogg");
        target->attrs.reset(char_attr::breaker);
        target->change_sprite(game->sprites["boy.txt"]);
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

void pop_up_milk(game_system* game, game_character* c, game_character* target)
{
    int x, y;
    boost::tie(x,y) = c->origin;

    if ((x != -1) && (y != -1))
        game->map.replace(x, y, 'm');

    c->change_sprite(game->sprites["used_block.txt"]);
    c->on_hit_from_below.clear();


    game_character milk;

    const sprite_info_set& infos = game->sprites["milk.txt"];
    const sprite_info& info = infos.get_group(milk.form)[0];

    milk.sprite_infos = &infos;
    milk.z = layer::enemy;
    milk.width = static_cast<float>(info.bounds.lx);
    milk.height = static_cast<float>(info.bounds.ly);
    milk.vx = 2.0f;
    milk.back = false;
    milk.on_collide_block_side = &turn;
    milk.on_collide_player = &power_up;
    milk.on_hit = &hop;

    c->move_routine = item_box_routine(milk);
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

    if (c->x < target->x)
        c->vx = -4.0f;
    else
        c->vx = 4.0f;
}

} // namespace

class main_window::impl
{
private:
    typedef std::list<game_character> chara_list;
    typedef chara_list::iterator chara_iterator;

public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , input_(handle_), system_(handle_), textures_(device_)
        , active_(false), last_time_(::GetTickCount()), frames_(0)
#if defined(HAMIGAKI_DISPLAY_FPS)
        , last_fps_time_(0), fps_count_(0)
#endif
        , stage_file_("map.txt"), scroll_x_(0.0f)
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
        system_.characters.sort(hamigaki::mem_var_greator(&game_character::z));

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
                system_.characters.begin(), system_.characters.end(),
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
    float scroll_x_;
    chara_iterator player_;
    int width_;
    int height_;

    chara_iterator find_enemy(int x, int y)
    {
        for (chara_iterator i = system_.characters.begin();
            i != system_.characters.end(); ++i)
        {
            if ((i->origin.first == x) && (i->origin.second == y))
                return i;
        }

        return system_.characters.end();
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
            game_character c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c.attrs.set(char_attr::enemy);
            c.vx = back ? -1.0f : 1.0f;
            c.move_routine = &velocity_routine;
            c.on_collide_block_side = &turn;
            c.on_collide_player = &power_down;
            c.on_stomp = &stomp;
            c.on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'a')
        {
            game_character c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c.attrs.set(char_attr::enemy);
            c.vx = back ? -1.0f : 1.0f;
            c.move_routine = &velocity_routine;
            c.speed_routine = &turn_routine;
            c.on_collide_block_side = &turn;
            c.on_collide_player = &power_down;
            c.on_stomp = &stomp;
            c.on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'p')
        {
            game_character c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c.attrs.set(char_attr::enemy);
            c.vx = back ? -2.0f : 2.0f;
            c.move_routine = &velocity_routine;
            c.speed_routine = hop_routine();
            c.on_collide_block_side = &turn;
            c.on_collide_player = &power_down;
            c.on_stomp = &stomp;
            c.on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'w')
        {
            game_character c =
                create_character(
                    x, y, layer::enemy, back,
                    system_.sprites["ball.txt"]
                );
            c.attrs.set(char_attr::enemy);
            c.vx = back ? -2.0f : 2.0f;
            c.move_routine = &velocity_routine;
            c.speed_routine = hop_step_jump_routine();
            c.on_collide_block_side = &turn;
            c.on_collide_player = &power_down;
            c.on_stomp = &stomp;
            c.on_hit = &hit;
            system_.characters.push_back(c);
        }
        else if (type == 'U')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["lift.txt"]
                );
            c.attrs.set(char_attr::block);
            c.vy = 2.0f;
            c.move_routine = &loop_lift_routine;
            system_.characters.push_back(c);
        }
        else if (type == 'D')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["lift.txt"]
                );
            c.attrs.set(char_attr::block);
            c.vy = -2.0f;
            c.move_routine = &loop_lift_routine;
            system_.characters.push_back(c);
        }
        else if (type == '=')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["brick_block.txt"]
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below = &to_fragments;
            system_.characters.push_back(c);
        }
        else if (type == 'G')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["brick_block.txt"]
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below = &to_used_block;
            system_.characters.push_back(c);
        }
        else if (type == 'I')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["brick_block.txt"]
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below = &pop_up_milk;
            system_.characters.push_back(c);
        }
        else if (type == 'm')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["used_block.txt"]
                );
            c.attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if (type == '$')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["item_box.txt"]
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below = &to_used_block;
            system_.characters.push_back(c);
        }
        else if (type == '?')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["item_box.txt"]
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below = &pop_up_milk;
            system_.characters.push_back(c);
        }
        else if (type == '/')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["left_down.txt"]
                );
            c.attrs.set(char_attr::block);
            c.slope = slope_type::left_down;
            system_.characters.push_back(c);
        }
        else if (type == '\\')
        {
            game_character c =
                create_character(
                    x, y, layer::block, false,
                    system_.sprites["right_down.txt"]
                );
            c.attrs.set(char_attr::block);
            c.slope = slope_type::right_down;
            system_.characters.push_back(c);
        }
    }

    void erase_old_enemies()
    {
        int scroll_x1 = static_cast<int>(scroll_x_ / 32.0f);
        int scroll_x2 = scroll_x1 + width_/32;

        chara_iterator next;
        for (chara_iterator i = system_.characters.begin();
            i != system_.characters.end(); i = next)
        {
            next = boost::next(i);

            if (i->y + i->height < -64.0f)
            {
                system_.characters.erase(i);
                continue;
            }

            int origin = i->origin.first;
            if (origin == -1)
                continue;

            int left  = static_cast<int>(i->x - i->width * 0.5f) / 32;
            int right = static_cast<int>(i->x + i->width * 0.5f) / 32;

            if (((origin < scroll_x1)  || (origin > scroll_x2)) &&
                ((right < scroll_x1-3) || (left > scroll_x2+3)) )
            {
                system_.characters.erase(i);
            }
        }
    }

    void reset_characters()
    {
        system_.sound.stop_se();

        load_map_from_text(stage_file_.c_str(), system_.map);

        game_character player;
        player.move_routine = &player_routine;
        player.speed_routine = user_control_routine();
        player.on_collide_block_side = &stop;
        player.sprite_infos = &system_.sprites["boy.txt"];
        player.attrs.set(char_attr::player);
        player.back = false;

        std::pair<int,int> pos = system_.map.player_position();

        sprite_info info =
            player.sprite_infos->get_group(player.form)[0];

        player.x = static_cast<float>(pos.first * 32 + 16);
        player.y = static_cast<float>(pos.second * 32);
        player.z = layer::player;
        player.width = static_cast<float>(info.bounds.lx);
        player.height = static_cast<float>(info.bounds.ly);

        float right_end = static_cast<float>(system_.map.width()*32);
        scroll_x_ = player.x - static_cast<float>(width_ / 2);
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + static_cast<float>(width_) > right_end)
            scroll_x_ = right_end - static_cast<float>(width_);

        system_.characters.clear();
        player_ = system_.characters.insert(system_.characters.end(), player);

        int min_x = static_cast<int>(scroll_x_) / 32 - 3;
        int max_x = static_cast<int>(scroll_x_) / 32 + width_ / 32 + 3;
        for (int y = 0; y < system_.map.height(); ++y)
        {
            for (int x = min_x; x < max_x; ++x)
                add_character(x, y, system_.map(x, y));
        }

        system_.sound.play_bgm("bgm.ogg");
    }

    void process_input_impl()
    {
        character_list& ls = system_.characters;

        if (system_.command.reset)
            reset_characters();

        std::for_each(
            ls.begin(), ls.end(),
            boost::bind(&game_character::move, _1, boost::ref(system_))
        );

        std::for_each(
            ls.begin(), ls.end(),
            boost::bind(&process_collisions, boost::ref(system_), _1)
        );

        if (!system_.new_characters.empty())
            ls.splice(ls.end(), system_.new_characters);

        if ((player_->form == miss_form) && player_->effect.empty())
            reset_characters();
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

        float right_end = static_cast<float>(system_.map.width()*32);

        int old_scroll_block = static_cast<int>(scroll_x_ / 32.0f);
        scroll_x_ = player_->x - static_cast<float>(width_ / 2);
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + static_cast<float>(width_) > right_end)
            scroll_x_ = right_end - static_cast<float>(width_);

        int scroll_block = static_cast<int>(scroll_x_ / 32.0f);
        if (scroll_block > old_scroll_block)
        {
            int x = scroll_block + width_/32 + 2;
            for (int y = 0; y < system_.map.height(); ++y)
                add_character(x, y, system_.map(x, y));
        }
        else if (scroll_block < old_scroll_block)
        {
            int x = scroll_block - 2;
            for (int y = 0; y < system_.map.height(); ++y)
                add_character(x, y, system_.map(x, y));
        }

        erase_old_enemies();
    }

    void draw_character(const game_character& c)
    {
        const sprite_info_set& infos = *(c.sprite_infos);

        const std::vector<sprite_info>& group = infos.get_group(c.form);

        std::size_t pattern = (c.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        const rect& tr = c.texture_rect();

        float x = tr.x - scroll_x_;
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
