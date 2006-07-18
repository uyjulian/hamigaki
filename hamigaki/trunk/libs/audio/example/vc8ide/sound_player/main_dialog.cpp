//  main_dialog.cpp: main dialog for sound_player

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "main_dialog.hpp"
#include <win32gui/common_dialogs.hpp>
#include <win32gui/event_handler.hpp>
#include <hamigaki/audio/pcm_device.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>

#include "win32gui_res/main.hpp"
#include "win32gui_res/menus.hpp"

namespace gui = win32::gui;
namespace audio = hamigaki::audio;
namespace io = boost::iostreams;

namespace
{
    int timer_id = gui::unique_timer_id(); 
} // namespace

class main_dialog_handler
    : public gui::event_handler<main_dialog_handler,main_dialog>
{
public:
    main_dialog_handler()
    {
    }

    gui::handle_event on_file_load(gui::mark_event_not_handled)
    {
        gui::file_dlg open_dlg(true);
        open_dlg.filter("*.ogg");
        if (open_dlg.show())
        {
            self->open(open_dlg.file_names()[0]);
            self->play();
        }
        return gui::command<gui::res_id::menu_::file_load>()
            .HANDLED_BY(&me::on_file_load);
    }

    gui::handle_event on_exit(gui::mark_event_not_handled)
    {
        ::PostQuitMessage(0);
        return gui::command<gui::res_id::menu_::file_exit>()
            .HANDLED_BY(&me::on_exit);
    }

    gui::handle_event on_play(gui::mark_event_not_handled)
    {
        self->toggle_play();
        return gui::command<m_play_::id>().HANDLED_BY(&me::on_play);
    }

    gui::handle_event on_stop(gui::mark_event_not_handled)
    {
        self->stop();
        return gui::command<m_stop_::id>().HANDLED_BY(&me::on_stop);
    }

    gui::handle_event on_timer(gui::w_param<int> id, gui::result r)
    {
        if (id == timer_id)
            self->update_progress();
        else
            r = gui::event_not_handled;
        return gui::event<WM_TIMER>().HANDLED_BY(&me::on_timer);
    }

private:
};

main_dialog::main_dialog()
{
    m_play->enable(false);
    m_stop->enable(false);
    m_slider->enable(false);
}

main_dialog::~main_dialog()
{
}

int main_dialog::dialog_id()
{
    return dialog_id_;
}

void main_dialog::open(const std::string& filename)
{
    if (!!player_)
    {
        stop();
        player_.close();
    }

    audio::vorbis_file_source vf(filename);

    const int total = static_cast<int>(vf.total());
    if (total > 0)
    {
        block_size_ = static_cast<int>(vf.info().channels);
        m_slider->enable(true);
        m_slider->max_val(total/block_size_);
        m_slider->pos(0, true);
    }
    else
        m_slider->enable(false);

    const audio::vorbis_info& info = vf.info();
    audio::pcm_format fmt;
    fmt.type = audio::int_le16;
    fmt.channels = info.channels;
    fmt.rate = info.rate;

    player_.open(
        vf, audio::widen<float>(audio::pcm_sink(fmt)),
        vf.optimal_buffer_size()
    );

    m_play->enable(true);
}

void main_dialog::play()
{
    if (m_slider->is_enabled())
    {
        m_slider->enable(false);
        if (m_slider->pos() != m_slider->max_val())
            player_.seek(m_slider->pos()*block_size_, BOOST_IOS::beg);
        else
            player_.seek(0, BOOST_IOS::beg);
    }

    player_.play();
    m_play->text("Pause");
    m_stop->enable(true);

    set_timer(timer_id, 100);
}

void main_dialog::stop()
{
    kill_timer(timer_id);
    player_.stop();
    player_.seek(0, BOOST_IOS::beg);
    m_play->text("Play");
    m_stop->enable(false);
    update_progress();

    if (static_cast<int>(player_.tell()) >= 0)
        m_slider->enable(true);
}

void main_dialog::pause()
{
    kill_timer(timer_id);
    player_.stop();
    m_play->text("Play");
    m_stop->enable(false);
    update_progress();

    if (static_cast<int>(player_.tell()) >= 0)
        m_slider->enable(true);
}

void main_dialog::toggle_play()
{
    if (m_play->text() == "Play")
        play();
    else
        pause();
}

void main_dialog::update_progress()
{
    if (player_.playing())
    {
        const int pos = static_cast<int>(player_.tell());
        if (pos >= 0)
            m_slider->pos(pos/block_size_, true);
    }
    else
    {
        kill_timer(timer_id);
        m_play->text("Play");
        m_stop->enable(false);

        const int pos = static_cast<int>(player_.tell());
        if (pos >= 0)
        {
            m_slider->pos(pos/block_size_, true);
            m_slider->enable(true);
        }
    }
}
