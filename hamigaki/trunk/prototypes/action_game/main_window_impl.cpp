// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "joystick_config.hpp"
#include "player_routine.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include "sprite_info.hpp"
#include "stage_map.hpp"
#include "straight_routine.hpp"
#include <hamigaki/input/direct_input.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <list>
#include <stdexcept>

namespace input = hamigaki::input;
namespace di = input::direct_input;
namespace coro = hamigaki::coroutines;

namespace
{

const long axis_range = 1000;

bool find_joystick(
    input::direct_input_manager& dinput, const hamigaki::uuid& guid)
{
    di::device_info_iterator beg, end;
    boost::tie(beg,end) = dinput.devices(di::device_type::joystick);

    for ( ; beg != end; ++beg)
    {
        if (beg->instance_guid == guid)
            return true;
    }

    return false;
}

boost::optional<input::direct_input_joystick>
open_joystick(const char* cfg_path, joystick_config& cfg)
{
    joystick_config_list cfg_list;
    load_joystick_config_list(cfg_list, cfg_path);

    input::direct_input_manager dinput(::GetModuleHandle(0));

    typedef joystick_config_list::const_iterator cfg_iter_type;
    for (cfg_iter_type it = cfg_list.begin(); it != cfg_list.end(); ++it)
    {
        const hamigaki::uuid& guid = it->first;

        if (find_joystick(dinput, guid))
        {
            cfg = it->second;
            return dinput.create_joystick_device(guid);
        }
    }

    di::device_info_iterator beg, end;
    boost::tie(beg,end) = dinput.devices(di::device_type::joystick);
    if (beg != end)
    {
        append_joystick_config(cfg_path, beg->instance_guid, cfg);
        return dinput.create_joystick_device(beg->instance_guid);
    }

    return boost::optional<input::direct_input_joystick>();
}

struct game_character
{
    move_info position;
    routine_type routine;
    std::size_t form;
    int step;
    bool back;
    sprite_info_list* sprite_infos;
    direct3d_texture9* texture;

    game_character() : form(0), step(0), back(false)
    {
    }
};

} // namespace

class main_window::impl
{
private:
    typedef std::list<game_character> chara_list;
    typedef chara_list::iterator chara_iterator;

public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , sound_(handle_)
        , active_(false), last_time_(::GetTickCount()), frames_(0)
        , scroll_x_(0.0f)
    {
        joystick_ = open_joystick("joystick-config.txt", joy_cfg_);

        if (input::direct_input_joystick* joy = joystick_.get_ptr())
        {
            unsigned long level = di::exclusive_level|di::foreground_level;
            joy->set_cooperative_level(handle_, level);

            di::device_object x_axis = joy->object(di::joystick_offset::x);
            x_axis.range(-axis_range, axis_range);
            x_axis.deadzone(2000);

            di::device_object y_axis = joy->object(di::joystick_offset::y);
            y_axis.range(-axis_range, axis_range);
            y_axis.deadzone(2000);
        }

        load_map_from_text(map_, "map.txt");
        load_sprite_info_list_from_text(player_sprite_info_, "man.txt");
        load_sprite_info_list_from_text(ball_sprite_info_, "ball.txt");

        player_.sprite_infos = &player_sprite_info_;
        player_.texture = &man_texture_;

        player_.routine = routine_type(player_routine(map_, sound_));

        int x, y;
        boost::tie(x, y) = map_.player_position();

        sprite_info info = player_.sprite_infos->get_group(player_.form)[0];
        player_.position.r.x = static_cast<float>(x * 32 + info.left);
        player_.position.r.y = static_cast<float>(y * 32);
        player_.position.r.lx = static_cast<float>(info.width);
        player_.position.r.ly = static_cast<float>(info.height);
        player_.position.vx = 0.0f;
        player_.position.vy = 0.0f;

        for (int y = 0; y < map_.height(); ++y)
        {
            for (int x = 0; x < map_.width(); ++x)
            {
                if (map_(x, y) == 'o')
                {
                    const sprite_info& info = ball_sprite_info_.get_group(0)[0];

                    game_character enemy;
                    enemy.routine = routine_type(&straight_routine);
                    enemy.sprite_infos = &ball_sprite_info_;
                    enemy.texture = &chara_texture_;
                    enemy.position.r.x = static_cast<float>(x * 32 + info.left);
                    enemy.position.r.y = static_cast<float>(y * 32);
                    enemy.position.r.lx = static_cast<float>(info.width);
                    enemy.position.r.ly = static_cast<float>(info.height);
                    enemy.position.vx = 0.0f;
                    enemy.position.vy = 0.0f;

                    enemies_.push_back(enemy);
                }
            }
        }

        sound_.play_bgm("bgm.ogg");
    }

    ~impl()
    {
    }

    void connect_d3d_device()
    {
        ::D3DPRESENT_PARAMETERS params; 
        std::memset(&params, 0, sizeof(params));
        params.Windowed = TRUE;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.BackBufferFormat = D3DFMT_UNKNOWN;

        device_ = d3d_.create_device(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle_,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, params);

        man_texture_ = create_png_texture(device_, "man.png");
        chara_texture_ = create_png_texture(device_, "chara.png");

        last_time_ = ::GetTickCount();
    }

    void process_input()
    {
        di::joystick_state state = {};
        update_input_state(state);

        const unsigned long table[] = { 16, 17, 17 };
        unsigned long now = ::GetTickCount();
        unsigned long elapsed = now - last_time_;
        while (elapsed >= table[frames_ % 3])
        {
            elapsed -= table[frames_ % 3];
            if (++frames_ == 60)
                frames_ = 0;
            this->process_input_impl(state);
        }
        last_time_ = now - elapsed;
    }

    void render()
    {
        int x1 = static_cast<int>(scroll_x_) / 32;
        int x2 = static_cast<int>(std::ceil((scroll_x_+640.0f) / 32.0f));

        device_.clear_target(D3DCOLOR_XRGB(0,0,255));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);

            for (int y = 0; y < 15; ++y)
            {
                for (int x = x1; x < x2; ++x)
                {
                    char c = map_(x, y);
                    if (c == '=')
                        draw_block(x, y);
                }
            }

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            std::for_each(
                enemies_.begin(), enemies_.end(),
                boost::bind(&impl::draw_character, this, _1)
            );

            draw_character(player_);
        }
        device_.present();
    }

    void active(bool val)
    {
        active_ = val;
    }

private:
    ::HWND handle_;
    sound_engine sound_;
    joystick_config joy_cfg_;
    boost::optional<input::direct_input_joystick> joystick_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 man_texture_;
    direct3d_texture9 chara_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    stage_map map_;
    float scroll_x_;
    sprite_info_list player_sprite_info_;
    sprite_info_list ball_sprite_info_;
    game_character player_;
    chara_list enemies_;

    void update_input_state(di::joystick_state& state)
    {
        if (!active_)
            return;

        try
        {
            if (input::direct_input_joystick* joy = joystick_.get_ptr())
                joy->get_state(state);
        }
        catch (const input::direct_input_error& e)
        {
            if (e.code() == E_ACCESSDENIED)
                return;
            throw;
        }
    }

    void process_input_impl(const di::joystick_state& state)
    {
        if ((joy_cfg_.reset != -1) &&
            ((state.buttons[joy_cfg_.reset] & 0x80) != 0))
        {
            int x, y;
            boost::tie(x, y) = map_.player_position();

            const sprite_info& info = player_.sprite_infos->get_group(0)[0];
            player_.position.r.x = static_cast<float>(x * 32 + info.left);
            player_.position.r.y = static_cast<float>(y * 32);
            player_.position.r.lx = static_cast<float>(info.width);
            player_.position.r.ly = static_cast<float>(info.height);
            player_.position.vx = 0.0f;
            player_.position.vy = 0.0f;
            player_.form = 0;
            player_.step = 0;
            player_.back = false;
        }

        float r = static_cast<float>(axis_range);
        float dx = static_cast<float>(state.position.x)/r;
        float dy = static_cast<float>(-state.position.y)/r;

        float radius = std::sqrt(dx*dx + dy*dy);
        if (radius > 1.0f)
        {
            dx /= radius;
            dy /= radius;
        }

        input_command cmd;
        cmd.x = dx;
        cmd.y = dy;
        cmd.jump = false;
        cmd.dash = false;
        cmd.reset = false;

        if (joy_cfg_.jump != -1)
            cmd.jump = (state.buttons[joy_cfg_.jump] & 0x80) != 0;
        if (joy_cfg_.dash != -1)
            cmd.dash = (state.buttons[joy_cfg_.dash] & 0x80) != 0;
        if (joy_cfg_.reset != -1)
            cmd.reset = (state.buttons[joy_cfg_.reset] & 0x80) != 0;

        acceleration a;
        std::size_t form;

        boost::tie(a, form) = player_.routine(player_.position, cmd);

        std::size_t old_form = player_.form;
        player_.form = form;

        if (old_form != player_.form)
        {
            const sprite_info& old =
                player_.sprite_infos->get_group(old_form)[0];
            const sprite_info& cur =
                player_.sprite_infos->get_group(player_.form)[0];
            player_.position.change_form(old, cur);
            player_.step = 0;
        }
        else
            ++(player_.step);

        player_.position.move(a, map_);

        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); ++i)
        {
            move_info& pos = i->position;
            pos.move(i->routine(pos, cmd).first, map_);
        }

        if (player_.position.vx >= 1.0f)
            player_.back = false;
        else if (player_.position.vx <= -1.0f)
            player_.back = true;

        float center = player_.position.r.x + player_.position.r.lx * 0.5f;
        float right_end = static_cast<float>(map_.width()*32);

        scroll_x_ = center - 320.0f;
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + 640.0f > right_end)
            scroll_x_ = right_end - 640.0f;
    }

    void draw_character(const game_character& chara)
    {
        const sprite_info_list& infos = *(chara.sprite_infos);
        const std::vector<sprite_info>& group = infos.get_group(chara.form);

        std::size_t pattern = (chara.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        ::draw_sprite(
            device_,
            chara.position.r.x - info.left - scroll_x_,
            480.0f - chara.position.r.y - infos.height(),
            0.0f,
            *(chara.texture),
            info.x, info.y,
            infos.width(), infos.height(),
            chara.back
        );
    }

    void draw_block(int x, int y)
    {
        draw_rectangle(
            device_,
            static_cast<float>(x*32)-scroll_x_,
            static_cast<float>(480-32 - y*32),
            0.0f,
            32.0f, 32.0f, D3DCOLOR_XRGB(0xAA,0x55,0x33));
    }
};

main_window::main_window(::HWND handle) : pimpl_(new impl(handle))
{
}

void main_window::connect_d3d_device()
{
    pimpl_->connect_d3d_device();
}

void main_window::process_input()
{
    pimpl_->process_input();
}

void main_window::render()
{
    pimpl_->render();
}

void main_window::active(bool val)
{
    pimpl_->active(val);
}
