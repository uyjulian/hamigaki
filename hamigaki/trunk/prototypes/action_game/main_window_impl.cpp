// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "blink_effect.hpp"
#include "bounce_routine.hpp"
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

void stop(game_system* game, game_character* c, game_character* target)
{
    c->vx = 0.0f;
}

void turn(game_system* game, game_character* c, game_character* target)
{
    c->vx = -c->vx;
    c->back = !c->back;
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
        , stage_file_("map.txt"), scroll_x_(0.0f)
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
        if (active_)
            system_.command  = input_();

        const unsigned long table[] = { 16, 17, 17 };
        unsigned long now = ::GetTickCount();
        unsigned long elapsed = now - last_time_;
        while (elapsed >= table[frames_ % 3])
        {
            elapsed -= table[frames_ % 3];
            if (++frames_ == 60)
                frames_ = 0;
            this->process_input_impl();
        }
        last_time_ = now - elapsed;
    }

    void render()
    {
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
    chara_iterator block_end_;
    chara_iterator player_;
    int width_;
    int height_;

    void to_fragments(
        game_system* game, game_character* c, game_character* target)
    {
        int x, y;
        boost::tie(x,y) = c->origin;

        if ((x != -1) && (y != -1))
            map_.erase(x, y);

        float dx[] = {-12.0f, -12.0f, 12.0f, 12.0f };
        float dy[] = { 32.0f,  64.0f, 32.0f, 64.0f };
        float vx[] = { -2.0f,  -2.0f,  2.0f,  2.0f };
        float vy[] = {  2.0f,   4.0f,  2.0f,  4.0f };

        for (std::size_t i = 0; i < 4; ++i)
        {
            game_character fr;

            fr.move_routine = &velocity_routine;
            fr.sprite_infos = &fragment_sprite_info_;
            fr.x = c->x + dx[i];
            fr.y = c->y + dy[i];
            fr.width = 0.0f;
            fr.height = 0.0f;
            fr.vx = vx[i];
            fr.vy = vy[i];
            fr.back = false;

            game->new_particles.push_back(fr);
        }

        system_.sound.play_se("break_block.ogg");

        c->y = -c->height - 128.0f;
    }

    void to_used_block(
        game_system* game, game_character* c, game_character* target)
    {
        int x, y;
        boost::tie(x,y) = c->origin;

        if ((x != -1) && (y != -1))
            map_.replace(x, y, 'm');

        c->change_sprite(block_sprite_info_);
        c->move_routine = bounce_routine();
        c->on_hit_from_below.clear();
    }

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

    void add_enemy(const game_character& c)
    {
        chara_iterator i = system_.characters.insert(player_, c);
        if (block_end_ == player_)
            block_end_ = i;
    }

    void add_block(const game_character& c)
    {
        system_.characters.insert(block_end_, c);
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
                create_enemy(
                    x, y, back,
                    routine_type(),
                    ball_sprite_info_
                );
            c.vx = back ? -1.0f : 1.0f;
            c.move_routine = &velocity_routine;
            c.on_collide_block_side = &turn;
            add_enemy(c);
        }
        else if (type == 'a')
        {
            game_character c =
                create_enemy(
                    x, y, back,
                    routine_type(),
                    ball_sprite_info_
                );
            c.vx = back ? -1.0f : 1.0f;
            c.move_routine = &velocity_routine;
            c.speed_routine = &turn_routine;
            c.on_collide_block_side = &turn;
            add_enemy(c);
        }
        else if (type == 'p')
        {
            game_character c =
                create_enemy(
                    x, y, back,
                    routine_type(),
                    ball_sprite_info_
                );
            c.vx = back ? -2.0f : 2.0f;
            c.move_routine = &velocity_routine;
            c.speed_routine = hop_routine();
            c.on_collide_block_side = &turn;
            add_enemy(c);
        }
        else if (type == 'w')
        {
            game_character c =
                create_enemy(
                    x, y, back,
                    routine_type(),
                    ball_sprite_info_
                );
            c.vx = back ? -2.0f : 2.0f;
            c.move_routine = &velocity_routine;
            c.speed_routine = hop_step_jump_routine();
            c.on_collide_block_side = &turn;
            add_enemy(c);
        }
        else if (type == 'U')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    lift_sprite_info_
                );
            c.attrs.set(char_attr::block);
            c.vy = 2.0f;
            c.move_routine = &loop_lift_routine;
            add_block(c);
        }
        else if (type == 'D')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    lift_sprite_info_
                );
            c.attrs.set(char_attr::block);
            c.vy = -2.0f;
            c.move_routine = &loop_lift_routine;
            add_block(c);
        }
        else if (type == '=')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    brick_sprite_info_
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below =
                boost::bind(&impl::to_fragments, this, _1, _2, _3);
            add_block(c);
        }
        else if (type == 'G')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    brick_sprite_info_
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below =
                boost::bind(&impl::to_used_block, this, _1, _2, _3);
            add_block(c);
        }
        else if (type == 'I')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    brick_sprite_info_
                );
            c.attrs.set(char_attr::block);
            add_block(c);
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
            add_block(c);
        }
        else if (type == '$')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    item_box_sprite_info_
                );
            c.attrs.set(char_attr::block);
            c.on_hit_from_below =
                boost::bind(&impl::to_used_block, this, _1, _2, _3);
            add_block(c);
        }
        else if (type == '?')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(),
                    item_box_sprite_info_
                );
            c.attrs.set(char_attr::block);
            add_block(c);
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
            add_block(c);
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
            add_block(c);
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
                if (i == block_end_)
                    block_end_ = player_;
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
                if (i == block_end_)
                    block_end_ = player_;
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

        system_.characters.insert(player_, b);

        game_character it =
            create_enemy(
                x, y, false,
                routine_type(),
                milk_sprite_info_
            );
        it.vx = 2.0f;
        it.move_routine = &velocity_routine;
        it.on_collide_block_side = &turn;
        it.origin.first = -1;
        it.origin.second = -1;

        system_.characters.insert(player_, it);
    }

    void reset_characters()
    {
        system_.sound.stop_se();

        load_map_from_text(stage_file_.c_str(), map_);

        game_character player;
        player.move_routine = &velocity_routine;
        player.speed_routine = player_routine();
        player.on_collide_block_side = &stop;
        player.sprite_infos = &mini_sprite_info_;
        player.attrs.set(char_attr::player);
        player.attrs.set(char_attr::breaker);
        player.back = false;

        std::pair<int,int> pos = map_.player_position();

        sprite_info info =
            player.sprite_infos->get_group(player.form)[0];

        player.x = static_cast<float>(pos.first * 32 + 16);
        player.y = static_cast<float>(pos.second * 32);
        player.width = static_cast<float>(info.bounds.lx);
        player.height = static_cast<float>(info.bounds.ly);

        float right_end = static_cast<float>(map_.width()*32);
        scroll_x_ = player.x - static_cast<float>(width_ / 2);
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + static_cast<float>(width_) > right_end)
            scroll_x_ = right_end - static_cast<float>(width_);

        system_.characters.clear();
        player_ = system_.characters.insert(system_.characters.end(), player);
        block_end_ = player_;

        int min_x = static_cast<int>(scroll_x_) / 32 - 3;
        int max_x = static_cast<int>(scroll_x_) / 32 + width_ / 32 + 3;
        for (int y = 0; y < map_.height(); ++y)
        {
            for (int x = min_x; x < max_x; ++x)
                add_character(x, y, map_(x, y));
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

        if (!system_.new_blocks.empty())
            ls.splice(block_end_, system_.new_blocks);

        if (!system_.new_enemies.empty())
        {
            if (block_end_ != player_)
                ls.splice(player_, system_.new_enemies);
            else if (block_end_ == ls.begin())
            {
                ls.splice(player_, system_.new_enemies);
                block_end_ = ls.begin();
            }
            else
            {
                chara_iterator i = boost::prior(block_end_);
                ls.splice(player_, system_.new_enemies);
                block_end_ = boost::next(i);
            }
        }

        if (!system_.new_particles.empty())
            ls.splice(ls.end(), system_.new_particles);

        float min_x = player_->width * 0.5f;
        if (player_->x < min_x)
        {
            player_->x = min_x;
            player_->vx = 0.0f;
        }

        float max_x = static_cast<float>(map_.width()*32) - player_->width*0.5f;
        if (player_->x > max_x)
        {
            player_->x = max_x;
            player_->vx = 0.0f;
        }

        if (player_->y + player_->height < -64.0f)
        {
            reset_characters();
            return;
        }

        float right_end = static_cast<float>(map_.width()*32);

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
            for (int y = 0; y < map_.height(); ++y)
                add_character(x, y, map_(x, y));
        }
        else if (scroll_block < old_scroll_block)
        {
            int x = scroll_block - 2;
            for (int y = 0; y < map_.height(); ++y)
                add_character(x, y, map_(x, y));
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
