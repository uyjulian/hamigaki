// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include "stage_map.hpp"
#include <hamigaki/audio/vorbis/comment.hpp>
#include <hamigaki/audio/background_player.hpp>
#include <hamigaki/audio/direct_sound.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/input/direct_input.hpp>
#include <hamigaki/iostreams/concatenate.hpp>
#include <hamigaki/iostreams/lazy_restrict.hpp>
#include <hamigaki/iostreams/repeat.hpp>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <stdexcept>

namespace audio = hamigaki::audio;
namespace ds = audio::direct_sound;
namespace input = hamigaki::input;
namespace di = input::direct_input;
namespace io_ex = hamigaki::iostreams;

namespace
{

const long axis_range = 1000;

di::device_info find_joystick(input::direct_input_manager& dinput)
{
    typedef di::device_info_iterator iter_type;
    typedef std::pair<iter_type,iter_type> pair_type;

    pair_type r(dinput.devices(di::device_type::joystick));
    if (r.first == r.second)
        throw std::runtime_error("Error: joystick not found");

    return *r.first;
}

input::direct_input_joystick create_joystick(::HWND handle)
{
    input::direct_input_manager dinput(::GetModuleHandle(0));
    const di::device_info& info = find_joystick(dinput);
    return dinput.create_joystick_device(info.instance_guid);
}

int vorbis_comment_int_value(
    const std::pair<const char**,const char**>& comments,
    const std::string& name)
{
    std::string value(audio::vorbis::comment_value(comments, name));
    return value.empty() ? -1 : boost::lexical_cast<int>(value);
}

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , vf_("bgm.ogg")
        , joystick_(create_joystick(handle_))
        , active_(false), last_time_(::GetTickCount()), frames_(0)
        , x_(32.0f), y_(416.0f), vx_(0.0f), vy_(0.0f)
        , jump_button_pressed_(false)
    {
        play_bgm();

        unsigned long level = di::exclusive_level|di::foreground_level;
        joystick_.set_cooperative_level(handle_, level);

        di::device_object x_axis = joystick_.object(di::joystick_offset::x);
        x_axis.range(-axis_range, axis_range);
        x_axis.deadzone(2000);

        di::device_object y_axis = joystick_.object(di::joystick_offset::y);
        y_axis.range(-axis_range, axis_range);
        y_axis.deadzone(2000);

        load_map_from_text(map_, "map.txt");
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

        chara_texture_ =
            create_png_texture(device_, "chara.png");
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
        device_.clear_target(D3DCOLOR_XRGB(0,0,255));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);

            for (std::size_t y = 0; y < 15; ++y)
            {
                for (std::size_t x = 0; x < 20; ++x)
                {
                    char c = map_(x, y);
                    if (c == '=')
                        draw_block(x, y);
                }
            }

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            draw_sprite(device_, x_, y_, 0.0f, chara_texture_);
        }
        device_.present();
    }

    void active(bool val)
    {
        active_ = val;
    }

private:
    ::HWND handle_;
    audio::direct_sound_device dsound_;
    audio::vorbis_file_source vf_;
    audio::background_player bgm_;
    input::direct_input_joystick joystick_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 chara_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    float x_;
    float y_;
    float vx_;
    float vy_;
    bool jump_button_pressed_;
    stage_map map_;

    void play_bgm()
    {
        const audio::vorbis_info& info = vf_.info();
        const int block_size = info.channels;

        int loop_start =
            vorbis_comment_int_value(vf_.comments(), "LOOPSTART");
        std::streamsize offset = loop_start != -1 ? block_size*loop_start : -1;

        int loop_length =
            vorbis_comment_int_value(vf_.comments(), "LOOPLENGTH");
        std::streamsize len = loop_length != -1 ? block_size*loop_length : -1;

        audio::pcm_format fmt;
        fmt.type = audio::int_le16;
        fmt.channels = info.channels;
        fmt.rate = info.rate;

        dsound_.set_cooperative_level(handle_, ds::priority_level);
        dsound_.format(fmt);

        if (offset == -1)
        {
            bgm_.open(
                io_ex::repeat(vf_, -1),
                audio::widen<float>(dsound_.create_buffer(fmt))
            );
        }
        else
        {
            using namespace io_ex::cat_operators;

            bgm_.open(
                io_ex::lazy_restrict(vf_, 0, offset+len) +
                io_ex::lazy_restrict(vf_, offset, len) * -1,
                audio::widen<float>(dsound_.create_buffer(fmt))
            );
        }
        bgm_.play();
    }

    void update_input_state(di::joystick_state& state)
    {
        if (!active_)
            return;

        try
        {
            joystick_.get_state(state);
        }
        catch (const input::direct_input_error& e)
        {
            if (e.code() == E_ACCESSDENIED)
            {
                active_ = false;
                return;
            }
            throw;
        }
    }

    void process_input_impl(const di::joystick_state& state)
    {
        if ((state.buttons[6] & 0x80) != 0)
        {
            x_ = 32.0f;
            y_ = 416.0f;
            vx_ = 0.0f;
            vy_ = 0.0f;
            jump_button_pressed_ = false;
        }

        bool jump_button_pressed = (state.buttons[0] & 0x80) != 0;
        bool jump_button_down = !jump_button_pressed_ && jump_button_pressed;

        bool dash_button_pressed = (state.buttons[2] & 0x80) != 0;

        float a = static_cast<float>(axis_range);
        float dx = static_cast<float>(state.position.x)/a;
        float dy = static_cast<float>(state.position.y)/a;

        float r = std::sqrt(dx*dx + dy*dy);
        if (r > 1.0f)
        {
            dx /= r;
            dy /= r;
        }

        float x_max = 608.0f;
        float y_max = 480.0f;

        bool on_ground = false;
        if (y_ >= 0.0f)
        {
            std::size_t y = static_cast<std::size_t>(y_ / 32.0f);
            std::size_t x1 = static_cast<std::size_t>(x_ / 32.0f);
            std::size_t x2 = static_cast<std::size_t>(std::ceil(x_ / 32.0f));
            if (y_ == static_cast<float>(y*32))
            {
                if (map_(x1, y+1) != ' ')
                    on_ground = true;
                if (map_(x2, y+1) != ' ')
                    on_ground = true;
            }
        }

        if (dx != 0.0f)
        {
            vx_ += dx/4.0f;
            if (dash_button_pressed && on_ground)
            {
                if (vx_ < -5.0f)
                    vx_ = -5.0f;
                else if (vx_ > 5.0f)
                    vx_ = 5.0f;
            }
            else
            {
                if (vx_ < -3.0f)
                    vx_ = -3.0f;
                else if (vx_ > 3.0f)
                    vx_ = 3.0f;
            }
        }
        else
        {
            // FIXME:
            if (vx_ < -0.0f)
            {
                vx_ += 0.2f;
                if (vx_ > 0.0f)
                    vx_ = 0.0f;
            }
            else if (vx_ > 0.0f)
            {
                vx_ -= 0.2f;
                if (vx_ < 0.0f)
                    vx_ = 0.0f;
            }
        }

        if (vx_ < 0.0f)
        {
            std::size_t old_x = static_cast<std::size_t>(x_ / 32.0f);

            x_ += vx_;
            if (x_ < 0.0f)
                x_ = 0.0f;

            std::size_t new_x = static_cast<std::size_t>(x_ / 32.0f);
            std::size_t y1 = static_cast<std::size_t>(y_ / 32.0f);
            std::size_t y2 = static_cast<std::size_t>(std::ceil(y_ / 32.0f));
            while (new_x != old_x)
            {
                char c = map_(old_x-1, y1);
                if (c != ' ')
                {
                    x_ = static_cast<float>(old_x * 32);
                    break;
                }

                c = map_(old_x-1, y2);
                if (c != ' ')
                {
                    x_ = static_cast<float>(old_x * 32);
                    break;
                }

                --old_x;
            }
        }
        else if (vx_ > 0.0f)
        {
            std::size_t old_x = static_cast<std::size_t>(std::ceil(x_ / 32.0f));

            x_ += vx_;
            if (x_ > x_max)
                x_ = x_max;

            std::size_t new_x = static_cast<std::size_t>(std::ceil(x_ / 32.0f));
            std::size_t y1 = static_cast<std::size_t>(y_ / 32.0f);
            std::size_t y2 = static_cast<std::size_t>(std::ceil(y_ / 32.0f));
            while (new_x != old_x)
            {
                char c = map_(old_x+1, y1);
                if (c != ' ')
                {
                    x_ = static_cast<float>(old_x * 32);
                    break;
                }
                c = map_(old_x+1, y2);
                if (c != ' ')
                {
                    x_ = static_cast<float>(old_x * 32);
                    break;
                }
                ++old_x;
            }
        }

        if (!on_ground)
        {
            // FIXME: The button should keep being pushed.
            if (jump_button_pressed && (vy_ < 0.0))
                vy_ -= 0.175f;
            vy_ += 0.3f;

            if (vy_ > 5.0f)
                vy_ = 5.0f;
        }
        else if (jump_button_down)
        {
            vy_ = -4.0f;
            if (std::abs(vx_) > 2.0f)
                vy_ += -0.5f;
        }

        if (vy_ < 0.0f)
        {
            std::size_t old_y = static_cast<std::size_t>(y_ / 32.0f);

            y_ += vy_ * 2.0f;

            if (y_ >= 0.0f)
            {
                std::size_t new_y = static_cast<std::size_t>(y_ / 32.0f);
                std::size_t x1 = static_cast<std::size_t>(x_ / 32.0f);
                std::size_t x2 = static_cast<std::size_t>(std::ceil(x_/32.0f));
                while (new_y != old_y)
                {
                    char c = map_(x1, old_y-1);
                    if (c != ' ')
                    {
                        y_ = static_cast<float>(old_y * 32);
                        vy_ = -vy_ * 0.5f;
                        break;
                    }

                    c = map_(x2, old_y-1);
                    if (c != ' ')
                    {
                        y_ = static_cast<float>(old_y * 32);
                        vy_ = -vy_ * 0.5f;
                        break;
                    }

                    --old_y;
                }
            }
        }
        else if (vy_ > 0.0f)
        {
            std::size_t old_y = static_cast<std::size_t>(std::ceil(y_ / 32.0f));

            y_ += vy_ * 2.0f;
            if (y_ > y_max)
                y_ = y_max;

            std::size_t new_y = static_cast<std::size_t>(std::ceil(y_ / 32.0f));
            std::size_t x1 = static_cast<std::size_t>(x_ / 32.0f);
            std::size_t x2 = static_cast<std::size_t>(std::ceil(x_ / 32.0f));
            while (new_y != old_y)
            {
                char c = map_(x1, old_y+1);
                if (c != ' ')
                {
                    y_ = static_cast<float>(old_y * 32);
                    break;
                }
                c = map_(x2, old_y+1);
                if (c != ' ')
                {
                    y_ = static_cast<float>(old_y * 32);
                    break;
                }
                ++old_y;
            }
        }

        jump_button_pressed_ = jump_button_pressed;
    }

    void draw_block(std::size_t x, std::size_t y)
    {
        draw_rectangle(
            device_, static_cast<float>(x*32), static_cast<float>(y*32), 0.0f,
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
