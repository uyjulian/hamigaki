//  main_window.cpp: main window for sound_player

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/background_player.hpp>
#include <hamigaki/audio/pcm_device.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wave_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "main_window.hpp"
#include <commctrl.h>
#include "controls.h"

namespace audio = hamigaki::audio;
namespace algo = boost::algorithm;
namespace io = boost::iostreams;

class main_window::impl
{
public:
    explicit impl(::HWND handle) : handle_(handle), timer_(0)
    {
        ::HINSTANCE hInstance =
            reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

        play_btn_ = ::CreateWindowEx(
            0, "BUTTON", "Play",
            WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_NOTIFY | BS_PUSHBUTTON,
            51, 55, 75, 23, handle_,
            reinterpret_cast< ::HMENU>(static_cast< ::UINT_PTR>(IDC_PLAY)),
            hInstance, 0
        );

        stop_btn_ = ::CreateWindowEx(
            0, "BUTTON", "Stop",
            WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_NOTIFY | BS_PUSHBUTTON,
            152, 55, 75, 23, handle_,
            reinterpret_cast< ::HMENU>(static_cast< ::UINT_PTR>(IDC_STOP)),
            hInstance, 0
        );

        slider_ = ::CreateWindowEx(
            WS_EX_CLIENTEDGE, "msctls_trackbar32", "",
            WS_CHILD | WS_VISIBLE | WS_DISABLED | TBS_BOTH | TBS_NOTICKS,
            19, 14, 242, 22, handle_,
            reinterpret_cast< ::HMENU>(static_cast< ::UINT_PTR>(IDC_SLIDER)),
            hInstance, 0
        );
    }

    ~impl()
    {
        if (timer_ != 0)
            ::KillTimer(handle_, timer_);
    }

    void open(const std::string& filename)
    {
        if (!!player_)
        {
            stop();
            player_.close();
        }

        if (algo::iends_with(filename, ".wav"))
        {
            audio::wave_file_source wav(filename);

            const audio::pcm_format& fmt = wav.format();
            block_size_ = static_cast<int>(fmt.block_size());

            const int total = static_cast<int>(wav.total());
            ::EnableWindow(slider_, TRUE);
            ::SendMessage(
                slider_, TBM_SETRANGEMAX, FALSE, total/block_size_);
            ::SendMessage(slider_, TBM_SETPOS, TRUE, 0);

            player_.open(
                wav, audio::pcm_sink(fmt),
                fmt.optimal_buffer_size()
            );
        }
        else if (algo::iends_with(filename, ".ogg"))
        {
            audio::vorbis_file_source vf(filename);
            block_size_ = static_cast<int>(vf.info().channels);

            const int total = static_cast<int>(vf.total());
            if (total > 0)
            {
                ::EnableWindow(slider_, TRUE);
                ::SendMessage(
                    slider_, TBM_SETRANGEMAX, FALSE, total/block_size_);
                ::SendMessage(slider_, TBM_SETPOS, TRUE, 0);
            }
            else
                ::EnableWindow(slider_, FALSE);

            const audio::vorbis_info& info = vf.info();
            audio::pcm_format fmt;
            fmt.type = audio::int_le16;
            fmt.channels = info.channels;
            fmt.rate = info.rate;

            player_.open(
                vf, audio::widen<float>(audio::pcm_sink(fmt)),
                vf.optimal_buffer_size()
            );
        }
        else
            throw std::runtime_error("unsupported file format");

        ::EnableWindow(play_btn_, TRUE);
    }

    void play()
    {
        if (::IsWindowEnabled(slider_) != FALSE)
        {
            ::EnableWindow(slider_, FALSE);

            ::LONG pos = ::SendMessage(slider_, TBM_GETPOS, 0, 0);
            ::LONG max_val = ::SendMessage(slider_, TBM_GETRANGEMAX, 0, 0);
            if (pos != max_val)
                player_.seek(pos*block_size_, BOOST_IOS::beg);
            else
                player_.seek(0, BOOST_IOS::beg);
        }

        player_.play();
        ::SetWindowText(play_btn_, "Pause");
        ::EnableWindow(stop_btn_, TRUE);

        timer_ = ::SetTimer(
            handle_, reinterpret_cast< ::UINT_PTR>(this), 100, &timer_callback);
    }

    void stop()
    {
        ::KillTimer(handle_, timer_);
        timer_ = 0;
        player_.stop();
        player_.seek(0, BOOST_IOS::beg);
        ::SetWindowText(play_btn_, "Play");
        ::EnableWindow(stop_btn_, FALSE);
        update_progress();

        if (static_cast<int>(player_.tell()) >= 0)
            ::EnableWindow(slider_, TRUE);
    }

    void pause()
    {
        ::KillTimer(handle_, timer_);
        timer_ = 0;
        player_.stop();
        ::SetWindowText(play_btn_, "Play");
        ::EnableWindow(stop_btn_, FALSE);
        update_progress();

        if (static_cast<int>(player_.tell()) >= 0)
            ::EnableWindow(slider_, TRUE);
    }

    bool playing() const
    {
        return timer_ != 0;
    }

private:
    ::HWND handle_;
    ::HWND play_btn_;
    ::HWND stop_btn_;
    ::HWND slider_;
    audio::background_player player_;
    int block_size_;
    ::UINT_PTR timer_;

    static void CALLBACK timer_callback(
        ::HWND, ::UINT, ::UINT_PTR idEvent, ::DWORD)
    {
        reinterpret_cast<impl*>(idEvent)->update_progress();
    }

    void update_progress()
    {
        if (player_.playing())
        {
            const int pos = static_cast<int>(player_.tell());
            if (pos >= 0)
                ::SendMessage(slider_, TBM_SETPOS, TRUE, pos/block_size_);
        }
        else
        {
            ::KillTimer(handle_, timer_);
            timer_ = 0;
            ::SetWindowText(play_btn_, "Play");
            ::EnableWindow(stop_btn_, FALSE);

            const int pos = static_cast<int>(player_.tell());
            if (pos >= 0)
            {
                ::SendMessage(slider_, TBM_SETPOS, TRUE, pos/block_size_);
                ::EnableWindow(slider_, TRUE);
            }
        }
    }
};

main_window::main_window(::HWND handle) : pimpl_(new impl(handle))
{
}

void main_window::open(const std::string& filename)
{
    pimpl_->open(filename);
}

void main_window::play()
{
    pimpl_->play();
}

void main_window::stop()
{
    pimpl_->stop();
}

void main_window::pause()
{
    pimpl_->pause();
}

bool main_window::playing() const
{
    return pimpl_->playing();
}
