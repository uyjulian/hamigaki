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
#include "wait_se_routine.hpp"
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <cmath>
#include <list>

#if defined(HAMIGAKI_USE_KNOCK_BACK)
    #include "knock_back_routine.hpp"
#endif

namespace
{

game_character
create_enemy(
    int x, int y, bool back,
    const routine_type& routine, const sprite_info_set& infos)
{
    game_character enemy;

    const sprite_info& info = infos.get_group(enemy.form.type)[0];

    enemy.routine = routine;
    enemy.tmp_routine = routine_type();
    enemy.sprite_infos = &infos;
    enemy.position.x = static_cast<float>(x * 32 + info.bounds.x);
    enemy.position.y = static_cast<float>(y * 32);
    enemy.position.lx = static_cast<float>(info.bounds.lx);
    enemy.position.ly = static_cast<float>(info.bounds.ly);
    enemy.speed.vx = 0.0f;
    enemy.speed.vy = 0.0f;
    enemy.origin.first = x;
    enemy.origin.second = y;

    if (back)
        enemy.form.options = sprite_options::back;

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

        device_.clear_target(D3DCOLOR_XRGB(0x77,0x66,0xDD));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            device_.set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            for (int y = 0; y < 15; ++y)
            {
                for (int x = x1; x < x2; ++x)
                {
                    char c = map_(x, y);
                    if ((c == '=') || (c == 'G') || (c == 'I'))
                        draw_block(x, y, 0, 0);
                    else if (c == 'm')
                        draw_block(x, y, 32, 0);
                    else if ((c == '$') || (c == '?'))
                        draw_block(x, y, 64, 0);
                    else if (c == '/')
                        draw_block(x, y, 0, 32);
                    else if (c == '\\')
                        draw_block(x, y, 32, 32);
                }
            }

            std::for_each(
                enemies_.begin(), enemies_.end(),
                boost::bind(&impl::draw_character, this, _1)
            );

            std::for_each(
                items_.begin(), items_.end(),
                boost::bind(&impl::draw_character, this, _1)
            );

            draw_character(player_);

            std::for_each(
                particles_.begin(), particles_.end(),
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
    game_character player_;
    chara_list enemies_;
    chara_list items_;
    chara_list particles_;
    int width_;
    int height_;
    bool missed_;

    chara_iterator find_enemy(int x, int y)
    {
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); ++i)
        {
            if ((i->origin.first == x) && (i->origin.second == y))
                return i;
        }

        return enemies_.end();
    }

    void add_enemy(int x, int y, char type)
    {
        if (is_block(type) || (type == ' '))
            return;

        if (find_enemy(x, y) != enemies_.end())
            return;

        bool back = left_block(player_.position) <= x;

        if (type == 'o')
        {
            enemies_.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(straight_routine()),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'a')
        {
            enemies_.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(turn_routine(map_)),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'p')
        {
            enemies_.push_back(
                create_enemy(
                    x, y, back,
                    routine_type(hop_routine(map_, 2.0f)),
                    ball_sprite_info_
                )
            );
        }
        else if (type == 'w')
        {
            enemies_.push_back(
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
            c.position.lx = 0.0f;
            c.position.ly = 0.0f;
            c.flying = true;
            enemies_.push_back(c);
        }
        else if (type == 'D')
        {
            game_character c =
                create_enemy(
                    x, y, false,
                    routine_type(lift_routine(-2.0f)),
                    lift_sprite_info_
                );
            c.position.lx = 0.0f;
            c.position.ly = 0.0f;
            c.flying = true;
            enemies_.push_back(c);
        }
    }

    void erase_old_enemies()
    {
        int scroll_x1 = static_cast<int>(scroll_x_ / 32.0f);
        int scroll_x2 = scroll_x1 + width_/32;

        chara_iterator next;
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); i = next)
        {
            next = boost::next(i);

            const rect& r = i->position;
            int left = left_block(r);
            int right = left_block(r);
            int origin = i->origin.first;

            if (((origin < scroll_x1) || (origin > scroll_x2)) &&
                ((left < scroll_x1-3) || (right > scroll_x2+3)) )
            {
                enemies_.erase(i);
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
        b.flying = true;
        b.origin.first = -1;
        b.origin.second = -1;

        particles_.push_back(b);

        game_character it =
            create_enemy(
                x, y, false,
                routine_type((straight_routine(2.0f))),
                milk_sprite_info_
            );
        it.tmp_routine =
            routine_type(pop_up_routine(1.0f, 0.6f, 32));
        it.origin.first = -1;
        it.origin.second = -1;

        items_.push_back(it);
    }

    void hit_enemy(game_character& enemy)
    {
        map_.replace(enemy.origin.first, enemy.origin.second, enemy.next_char);

        enemy.change_form(player_routine::miss_form);
        enemy.routine = routine_type(vanish_routine(10));
        enemy.position.y += 16.0f;

        if (enemy.position.x < player_.position.x)
            enemy.speed.vx = -4.0f;
        else
            enemy.speed.vx = 4.0f;

        enemy.speed.vy = 4.0f;
    }

    void reset_characters()
    {
        missed_ = false;
        system_.sound.stop_se();

        load_map_from_text(stage_file_.c_str(), map_);

        player_.sprite_infos = &mini_sprite_info_;
        player_.routine = routine_type(player_routine(map_, system_.sound));
        player_.tmp_routine = routine_type();
        player_.effect = effect_type();
        player_.form = sprite_form();

        std::pair<int,int> pos = map_.player_position();

        sprite_info info =
            player_.sprite_infos->get_group(player_.form.type)[0];

        player_.position.x = static_cast<float>(pos.first * 32 + info.bounds.x);
        player_.position.y = static_cast<float>(pos.second * 32);
        player_.position.lx = static_cast<float>(info.bounds.lx);
        player_.position.ly = static_cast<float>(info.bounds.ly);
        player_.speed.vx = 0.0f;
        player_.speed.vy = 0.0f;
        player_.auto_slip_out = true;

        const rect& r = player_.position;
        float center = player_.position.x + player_.position.lx * 0.5f;
        float right_end = static_cast<float>(map_.width()*32);
        scroll_x_ = center - static_cast<float>(width_ / 2);
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + static_cast<float>(width_) > right_end)
            scroll_x_ = right_end - static_cast<float>(width_);

        enemies_.clear();
        int min_x = static_cast<int>(scroll_x_) / 32 - 3;
        int max_x = static_cast<int>(scroll_x_) / 32 + width_ / 32 + 3;
        for (int y = 0; y < map_.height(); ++y)
        {
            for (int x = min_x; x < max_x; ++x)
                add_enemy(x, y, map_(x, y));
        }

        items_.clear();
        particles_.clear();

        system_.sound.play_bgm("bgm.ogg");
    }

    void process_hit_from_below(int x, int y)
    {
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); ++i)
        {
            const rect& r = i->position;
            if (r.ly == 0.0f)
                continue;

            int center = static_cast<int>(r.x+r.lx*0.5f)/32;

            if ((center == x) && (r.y == static_cast<float>(y*32)))
                hit_enemy(*i);
        }

        for (chara_iterator i = items_.begin(); i != items_.end(); ++i)
        {
            const rect& r = i->position;
            if (r.ly == 0.0f)
                continue;

            int center = static_cast<int>(r.x+r.lx*0.5f)/32;

            if ((center == x) && (r.y == static_cast<float>(y*32)))
                i->speed.vy = 8.0f;
        }
    }

    void process_map_collisions()
    {
        chara_iterator next;
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); i = next)
        {
            next = boost::next(i);

            rect& r = i->position;

            rect old_rect = r;
            old_rect.y -= i->speed.vy;

            int old_y = top_block(old_rect);
            int new_y = top_block(r);
            int x1 = left_block(r);
            int x2 = right_block(r);
            int x = static_cast<int>(r.x+r.lx*0.5f)/32;

            if ((x2 < 0) || (x1 >= map_.width()) || (new_y < -1))
            {
                enemies_.erase(i);
                continue;
            }

            if (r.ly == 0.0f)
                continue;

            for (int y = old_y+1; y <= new_y; ++y)
            {
                if (is_ceiling(map_(x, y)))
                {
                    r.y = static_cast<float>(y * 32) - r.ly;
                    i->speed.vy = -i->speed.vy * 0.5f;
                    break;
                }
                else if (find_ceiling(map_, y, x1, x2))
                {
                    r.x = static_cast<float>(x * 32);
                    break;
                }
            }
        }

        if (player_.speed.vy > 0.0f)
        {
            rect& r = player_.position;

            rect old_rect = r;
            old_rect.y -= player_.speed.vy;

            int old_y = top_block(old_rect);
            int new_y = top_block(r);
            int x1 = left_block(r);
            int x2 = right_block(r);
            int x = static_cast<int>(r.x+r.lx*0.5f)/32;

            for (int y = old_y+1; y <= new_y; ++y)
            {
                char type = map_(x, y);
                if ((type == '=') &&
                    (player_.sprite_infos == &mini_sprite_info_) )
                {
                    map_.replace(x, y, '_');
                    r.y = static_cast<float>(y * 32) - r.ly;
                    player_.speed.vy = -player_.speed.vy * 0.5f;

                    process_hit_from_below(x, y+1);

                    game_character b =
                        create_enemy(
                            x, y, false,
                            routine_type(vanish_routine(10)),
                            brick_sprite_info_
                        );
                    b.position.lx = 0.0f;
                    b.position.ly = 0.0f;
                    b.speed.vy = 4.0f;
                    b.next_char = '=';

                    particles_.push_back(b);

                    break;
                }
                else if (type == '=')
                {
                    map_.erase(x, y);
                    r.y = static_cast<float>(y * 32) - r.ly;
                    player_.speed.vy = -player_.speed.vy * 0.5f;

                    float dx[] = { -4.0f, -4.0f, 20.0f, 20.0f };
                    float dy[] = { 32.0f, 64.0f, 32.0f, 64.0f };
                    float vx[] = { -2.0f, -2.0f,  2.0f,  2.0f };
                    float vy[] = {  2.0f,  4.0f,  2.0f,  4.0f };

                    for (std::size_t i = 0; i < 4; ++i)
                    {
                        game_character fr;

                        fr.routine = routine_type(vanish_routine(30));
                        fr.tmp_routine = routine_type();
                        fr.sprite_infos = &fragment_sprite_info_;
                        fr.position.x = static_cast<float>(x*32) + dx[i];
                        fr.position.y = static_cast<float>(y*32) + dy[i];
                        fr.position.lx = 0.0f;
                        fr.position.ly = 0.0f;
                        fr.speed.vx = vx[i];
                        fr.speed.vy = vy[i];
                        fr.origin.first = -1;
                        fr.origin.second = -1;

                        particles_.push_back(fr);
                    }

                    process_hit_from_below(x, y+1);

                    system_.sound.play_se("break_block.ogg");

                    break;
                }
                else if ((type == '$') || (type == 'G') || (type == 'S'))
                {
                    map_.replace(x, y, '_');
                    r.y = static_cast<float>(y * 32) - r.ly;
                    player_.speed.vy = -player_.speed.vy * 0.5f;

                    process_hit_from_below(x, y+1);

                    game_character b =
                        create_enemy(
                            x, y, false,
                            routine_type(vanish_routine(10)),
                            block_sprite_info_
                        );
                    b.position.lx = 0.0f;
                    b.position.ly = 0.0f;
                    b.speed.vy = 4.0f;
                    b.next_char = 'm';

                    particles_.push_back(b);

                    break;
                }
                else if ((type == '?') || (type == 'I'))
                {
                    map_.replace(x, y, '_');
                    r.y = static_cast<float>(y * 32) - r.ly;
                    player_.speed.vy = -player_.speed.vy * 0.5f;

                    process_hit_from_below(x, y+1);

                    game_character b =
                        create_enemy(
                            x, y, false,
                            routine_type(vanish_routine(10)),
                            block_sprite_info_
                        );
                    b.position.lx = 0.0f;
                    b.position.ly = 0.0f;
                    b.speed.vy = 4.0f;
                    b.next_char = 'm';
                    b.on_end = boost::bind(&impl::pop_up_milk, this, x, y);

                    particles_.push_back(b);

                    break;
                }
                else if (is_ceiling(type))
                {
                    r.y = static_cast<float>(y * 32) - r.ly;
                    player_.speed.vy = -player_.speed.vy * 0.5f;
                    break;
                }
                else if (find_ceiling(map_, y, x1, x2))
                {
                    r.x = static_cast<float>(x * 32);
                    break;
                }
            }
        }
    }

    void remove_nform_characters()
    {
        chara_iterator next;
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); i = next)
        {
            next = boost::next(i);

            if (i->form.type == sprite_form::nform)
            {
                map_.replace(i->origin.first, i->origin.second, i->next_char);
                if (i->on_end)
                     i->on_end();
                enemies_.erase(i);
                continue;
            }
        }

        for (chara_iterator i = items_.begin(); i != items_.end(); i = next)
        {
            next = boost::next(i);

            if (i->form.type == sprite_form::nform)
            {
                items_.erase(i);
                continue;
            }
        }

        for (chara_iterator i=particles_.begin(); i != particles_.end(); i=next)
        {
            next = boost::next(i);

            if (i->form.type == sprite_form::nform)
            {
                map_.replace(i->origin.first, i->origin.second, i->next_char);
                if (i->on_end)
                     i->on_end();
                particles_.erase(i);
                continue;
            }
        }
    }

    void process_collisions()
    {
        bool on_slope = is_on_slope(map_, player_.position);

        chara_iterator next;
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); i = next)
        {
            next = boost::next(i);

            rect r = i->position;
            if ((r.lx == 0.0f) || (r.ly == 0.0f))
                continue;

            r.ly *= 0.5f;
            r.y += r.ly;

            const rect& r2 = player_.position;
            if ((r2.lx == 0.0f) || (r2.ly == 0.0f))
                continue;

            float x1 = r2.x;
            float y1 = r2.y;
            float x2 = r2.x + r2.lx;
            float y2 = r2.y + r2.ly;

            if ( (includes_point(r, x1, y1) || includes_point(r, x2, y1)) &&
                !(includes_point(r, x1, y2) || includes_point(r, x2, y2)) &&
                !on_slope )
            {
                i->change_form(static_four_char_code<'S','T','M','P'>::value);
                i->routine = routine_type(vanish_routine(5));
                i->speed.vx = 0.0f;
                i->speed.vy = 0.0f;
                system_.sound.play_se("stomp.ogg");
                player_.speed.vy = 8.0f;
                if (player_.form.type != player_routine::duck_jump_form)
                {
                    if (player_.form.type == player_routine::duck_form)
                        player_.change_form(player_routine::duck_jump_form);
                    else
                        player_.change_form(player_routine::jump_form);
                }
            }
            else if (player_.effect.empty() && player_.tmp_routine.empty() &&
                intersect_rects(player_.position, i->position))
            {
#if !defined(HAMIGAKI_USE_KNOCK_BACK)
                if (player_.sprite_infos == &mini_sprite_info_)
                {
                    system_.sound.stop_bgm();
                    player_.change_form(player_routine::miss_form);
                    player_.routine =
                        routine_type(
                            wait_se_routine(system_.sound, "miss.ogg")
                        );
                    player_.speed.vx = 0.0f;
                    player_.speed.vy = 10.0f;
                    missed_ = true;
                }
                else
                {
                    player_.change_sprite(mini_sprite_info_);
                    player_.effect = effect_type(&blink_effect);
                    system_.sound.play_se("damage.ogg");
                }
#else
                player_.tmp_routine = routine_type(knock_back_routine(10,4.0f));
                player_.form.type = player_routine::knock_back_form;
                system_.sound.play_se("damage.ogg");
#endif
            }
        }

        for (chara_iterator i = items_.begin(); i != items_.end(); i = next)
        {
            next = boost::next(i);

            if (intersect_rects(player_.position, i->position))
            {
                items_.erase(i);
                if (player_.sprite_infos == &mini_sprite_info_)
                    player_.change_sprite(player_sprite_info_);
            }
        }
    }

    void process_attack_collisions()
    {
        const sprite_info& info =
            player_.sprite_infos->get_group(player_.form.type)[0];

        const rectangle<int>& br = info.bounds;
        const rectangle<int>& ar0 = info.attack;

        if ((ar0.lx == 0) || (ar0.ly == 0))
            return;

        rect ar;
        if ((player_.form.options & sprite_options::back) != 0)
        {
            ar.x = player_.position.x +
                static_cast<float>(
                    player_.sprite_infos->width() - ar0.x - ar0.lx - br.x);
        }
        else
            ar.x = player_.position.x + static_cast<float>(ar0.x - br.x);
        ar.y = player_.position.y +
            static_cast<float>(player_.sprite_infos->height() - ar0.y - ar0.ly);
        ar.lx = static_cast<float>(ar0.lx);
        ar.ly = static_cast<float>(ar0.ly);

        chara_iterator next;
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); ++i)
        {
            if (intersect_rects(ar, i->position))
                hit_enemy(*i);
        }
    }

    void process_enemy_collisions()
    {
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); ++i)
        {
            if ((i->position.lx == 0) || (i->position.ly == 0))
                continue;

            rect left_rect = i->position;
            left_rect.lx *= 0.5f;

            rect right_rect = i->position;
            right_rect.lx *= 0.5f;
            right_rect.x += right_rect.lx;

            for (chara_iterator j = enemies_.begin(); j != enemies_.end(); ++j)
            {
                if (i == j)
                    continue;

                if ((j->position.lx == 0) || (j->position.ly == 0))
                    continue;

                if (i->speed.vx < 0.0f)
                {
                    if ( intersect_rects(left_rect,  j->position) &&
                        !intersect_rects(right_rect, j->position) )
                    {
                        i->speed.vx = 0.0f;
                    }
                }
                else if (i->speed.vx > 0.0f)
                {
                    if ( intersect_rects(right_rect, j->position) &&
                        !intersect_rects(left_rect,  j->position) )
                    {
                        i->speed.vx = 0.0f;
                    }
                }
            }
        }
    }

    void process_input_impl(const input_command& cmd)
    {
        if (cmd.reset)
            reset_characters();

        std::for_each(
            enemies_.begin(), enemies_.end(),
            boost::bind(&game_character::move, _1, cmd, boost::cref(map_))
        );

        player_.move(cmd, map_);
        if (player_.form.type == sprite_form::nform)
            reset_characters();

        if (player_.position.x < 0.0f)
        {
            player_.position.x = 0.0f;
            player_.speed.vx = 0.0f;
        }

        float max_x = static_cast<float>(map_.width()*32) - player_.position.lx;
        if (player_.position.x > max_x)
        {
            player_.position.x = max_x;
            player_.speed.vx = 0.0f;
        }

        if (!missed_ && (top_block(player_.position) < 0))
        {
            system_.sound.stop_bgm();
            player_.change_sprite(mini_sprite_info_);
            player_.change_form(player_routine::miss_form);
            player_.routine =
                routine_type(wait_se_routine(system_.sound, "miss.ogg"));
            missed_ = true;
        }

        std::for_each(
            items_.begin(), items_.end(),
            boost::bind(&game_character::move, _1, cmd, boost::cref(map_))
        );

        std::for_each(
            particles_.begin(), particles_.end(),
            boost::bind(&game_character::move, _1, cmd, boost::cref(map_))
        );

        process_map_collisions();
        remove_nform_characters();
        process_attack_collisions();
        process_collisions();
        process_enemy_collisions();

        float center = player_.position.x + player_.position.lx * 0.5f;
        float right_end = static_cast<float>(map_.width()*32);

        int old_scroll_block = static_cast<int>(scroll_x_ / 32.0f);
        scroll_x_ = center - static_cast<float>(width_ / 2);
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

        const std::vector<sprite_info>& group =
            infos.get_group(chara.form.type);

        std::size_t pattern = (chara.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        float x = chara.position.x - info.bounds.x - scroll_x_;
        float y = static_cast<float>(height_)-chara.position.y-infos.height();

        const std::string& texture = infos.texture();
        if (!texture.empty())
        {
            ::draw_sprite(
                device_, x, y, 0.0f, textures_[texture],
                info.x, info.y, infos.width(), infos.height(),
                chara.form.options, chara.color
            );
        }
    }

    void draw_block(int x, int y, int tx, int ty)
    {
        ::draw_sprite(
            device_,
            static_cast<float>(x*32)-scroll_x_,
            static_cast<float>(height_-32 - y*32),
            0.0f,
            textures_["map_chips.png"],
            tx, ty, 32, 32, 0
        );
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
