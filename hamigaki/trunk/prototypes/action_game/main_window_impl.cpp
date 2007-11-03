// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "blink_effect.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "game_character.hpp"
#include "game_system.hpp"
#include "hop_routine.hpp"
#include "hop_step_jump_routine.hpp"
#include "lift_routine.hpp"
#include "player_routine.hpp"
#include "png_loader.hpp"
#include "pop_up_routine.hpp"
#include "sprite.hpp"
#include "sprite_info.hpp"
#include "stage_map.hpp"
#include "straight_routine.hpp"
#include "texture_cache.hpp"
#include "turn_routine.hpp"
#include "vanish_routine.hpp"
#include "velocity_routine.hpp"
#include "wait_se_routine.hpp"
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <cmath>
#include <list>

namespace
{

game_character
create_enemy(
    int x, int y, bool back,
    const routine_type& routine, const sprite_info_set& infos)
{
    game_character enemy;

    const sprite_info& info = infos.get_group(enemy.form)[0];

    enemy.sprite_infos = &infos;
    enemy.x = static_cast<float>(x * 32 + 16);
    enemy.y = static_cast<float>(y * 32);
    enemy.width = static_cast<float>(info.bounds.lx);
    enemy.height = static_cast<float>(info.bounds.ly);
    enemy.vx = 0.0f;
    enemy.vy = 0.0f;
    enemy.origin.first = x;
    enemy.origin.second = y;
    enemy.back = back;

    return enemy;
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
        , system_(handle_), textures_(device_)
        , active_(false), last_time_(::GetTickCount()), frames_(0)
        , stage_file_("map.txt"), scroll_x_(0.0f), missed_(true)
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);
        width_ = static_cast<int>(cr.right);
        height_ = static_cast<int>(cr.bottom);

        load_sprite_info_set_from_text("man.txt", player_sprite_info_);
        load_sprite_info_set_from_text("boy.txt", mini_sprite_info_);
        load_sprite_info_set_from_text("ball.txt", ball_sprite_info_);
        load_sprite_info_set_from_text("fragment.txt", fragment_sprite_info_);
        load_sprite_info_set_from_text("used_block.txt", block_sprite_info_);
        load_sprite_info_set_from_text("brick_block.txt", brick_sprite_info_);
        load_sprite_info_set_from_text("milk.txt", milk_sprite_info_);
        load_sprite_info_set_from_text("lift.txt", lift_sprite_info_);
        load_sprite_info_set_from_text("item_box.txt", item_box_sprite_info_);
        load_sprite_info_set_from_text("left_down.txt", left_down_sprite_info_);
        load_sprite_info_set_from_text(
            "right_down.txt", right_down_sprite_info_);

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
    }

    void process_input()
    {
        input_command cmd;
        if (active_)
            cmd  = system_.input();

        const unsigned long table[] = { 16, 17, 17 };
        unsigned long now = ::GetTickCount();
        unsigned long elapsed = now - last_time_;
        while (elapsed >= table[frames_ % 3])
        {
            elapsed -= table[frames_ % 3];
            if (++frames_ == 60)
                frames_ = 0;
            this->process_input_impl(cmd);
        }
        last_time_ = now - elapsed;
    }

    void render()
    {
        int x1 = static_cast<int>(scroll_x_) / 32;

        int x2 = static_cast<int>(std::ceil((scroll_x_) / 32.0f));
        x2 += width_ / 32;

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

            draw_character(player_);

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
    game_system system_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    texture_cache textures_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    std::string stage_file_;
    stage_map map_;
    float scroll_x_;
    sprite_info_set player_sprite_info_;
    sprite_info_set mini_sprite_info_;
    sprite_info_set ball_sprite_info_;
    sprite_info_set fragment_sprite_info_;
    sprite_info_set block_sprite_info_;
    sprite_info_set brick_sprite_info_;
    sprite_info_set milk_sprite_info_;
    sprite_info_set lift_sprite_info_;
    sprite_info_set item_box_sprite_info_;
    sprite_info_set left_down_sprite_info_;
    sprite_info_set right_down_sprite_info_;
    game_character player_;
    int width_;
    int height_;
    bool missed_;

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

    void add_enemy(int x, int y, char type)
    {
        if (type == ' ')
            return;

        if (find_enemy(x, y) != system_.characters.end())
            return;

        bool back = player_.x <= static_cast<float>(x*32);

        if (type == 'o')
        {
            system_.characters.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(straight_routine()),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'a')
        {
            system_.characters.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(turn_routine(map_)),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'p')
        {
            system_.characters.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(hop_routine(map_, 2.0f)),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'w')
        {
            system_.characters.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(hop_step_jump_routine(map_, 2.0f)),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'U')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(lift_routine(2.0f)),
                    lift_sprite_info_
                );
            c.attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if (type == 'D')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(lift_routine(-2.0f)),
                    lift_sprite_info_
                );
            c.attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if ((type == '=') || (type == 'G') || (type == 'I'))
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    brick_sprite_info_
                );
            c.attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if (type == 'm')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    block_sprite_info_
                );
            c.attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if ((type == '$') || (type == '?'))
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    item_box_sprite_info_
                );
            c.attrs.set(char_attr::block);
            system_.characters.push_back(c);
        }
        else if (type == '/')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    left_down_sprite_info_
                );
            c.attrs.set(char_attr::block);
            c.slope = slope_type::left_down;
            system_.characters.push_back(c);
        }
        else if (type == '\\')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    right_down_sprite_info_
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

            int left = static_cast<int>(i->x) / 32;
            int right = static_cast<int>(i->x + i->width) / 32;
            int origin = i->origin.first;

            if (((origin < scroll_x1) || (origin > scroll_x2)) &&
                ((left < scroll_x1-3) || (right > scroll_x2+3)) )
            {
                system_.characters.erase(i);
            }
        }
    }

    void pop_up_milk(int x, int y)
    {
        game_character b =
            create_enemy(
                x, y, false,
                routine_type(vanish_routine(32)),
                block_sprite_info_
            );
        b.origin.first = -1;
        b.origin.second = -1;

        system_.characters.push_back(b);

        game_character it =
            create_enemy(
                x, y, false,
                routine_type((straight_routine(2.0f))),
                milk_sprite_info_
            );
        it.origin.first = -1;
        it.origin.second = -1;

        system_.characters.push_back(it);
    }

    void reset_characters()
    {
        missed_ = false;
        system_.sound.stop_se();

        load_map_from_text(stage_file_.c_str(), map_);

        player_.move_routine = &velocity_routine;
        player_.sprite_infos = &mini_sprite_info_;
        player_.effect = effect_type();
        player_.form = sprite_form::normal;
        player_.attrs.set(char_attr::player);


        std::pair<int,int> pos = map_.player_position();

        sprite_info info =
            player_.sprite_infos->get_group(player_.form)[0];

        player_.x = static_cast<float>(pos.first * 32 + 16);
        player_.y = static_cast<float>(pos.second * 32);
        player_.width = static_cast<float>(info.bounds.lx);
        player_.height = static_cast<float>(info.bounds.ly);
        player_.vx = 2.0f;
        player_.vy = 0.0f;

        float right_end = static_cast<float>(map_.width()*32);
        scroll_x_ = player_.x - static_cast<float>(width_ / 2);
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + static_cast<float>(width_) > right_end)
            scroll_x_ = right_end - static_cast<float>(width_);

        system_.characters.clear();
        int min_x = static_cast<int>(scroll_x_) / 32 - 3;
        int max_x = static_cast<int>(scroll_x_) / 32 + width_ / 32 + 3;
        for (int y = 0; y < map_.height(); ++y)
        {
            for (int x = min_x; x < max_x; ++x)
                add_enemy(x, y, map_(x, y));
        }

        system_.sound.play_bgm("bgm.ogg");
    }

    void process_input_impl(const input_command& cmd)
    {
        if (cmd.reset)
            reset_characters();

        std::for_each(
            system_.characters.begin(), system_.characters.end(),
            boost::bind(&game_character::move, _1, boost::ref(system_))
        );

        player_.move(system_);
        if (player_.form == sprite_form::nform)
            reset_characters();

        float min_x = player_.width * 0.5f;
        if (player_.x < min_x)
        {
            player_.x = min_x;
            player_.vx = 0.0f;
        }

        float max_x = static_cast<float>(map_.width()*32) - player_.width*0.5f;
        if (player_.x > max_x)
        {
            player_.x = max_x;
            player_.vx = 0.0f;
        }

        if (!missed_ && (player_.y + player_.height < 0.0f))
        {
            system_.sound.stop_bgm();
            player_.change_sprite(mini_sprite_info_);
            player_.change_form(player_routine::miss_form);
            missed_ = true;
        }

        float right_end = static_cast<float>(map_.width()*32);

        int old_scroll_block = static_cast<int>(scroll_x_ / 32.0f);
        scroll_x_ = player_.x - static_cast<float>(width_ / 2);
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + static_cast<float>(width_) > right_end)
            scroll_x_ = right_end - static_cast<float>(width_);

        int scroll_block = static_cast<int>(scroll_x_ / 32.0f);
        if (scroll_block > old_scroll_block)
        {
            int x = scroll_block + width_/32 + 2;
            for (int y = 0; y < map_.height(); ++y)
                add_enemy(x, y, map_(x, y));
        }
        else if (scroll_block < old_scroll_block)
        {
            int x = scroll_block - 2;
            for (int y = 0; y < map_.height(); ++y)
                add_enemy(x, y, map_(x, y));
        }

        erase_old_enemies();
    }

    void draw_character(const game_character& chara)
    {
        const sprite_info_set& infos = *(chara.sprite_infos);

        const std::vector<sprite_info>& group = infos.get_group(chara.form);

        std::size_t pattern = (chara.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        const rect& tr = chara.texture_rect();

        float x = tr.x - scroll_x_;
        float y = height_ - tr.y - tr.ly;

        const std::string& texture = infos.texture();
        if (!texture.empty())
        {
            ::draw_sprite(
                device_, x, y, 0.0f, textures_[texture],
                info.x, info.y, infos.width(), infos.height(),
                chara.back, chara.color
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
