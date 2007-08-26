// main_window.cpp: main window for sound_player

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/aiff_file.hpp>
#include <hamigaki/audio/background_player.hpp>
#include <hamigaki/audio/pcm_device.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wave_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "main_window.hpp"
#include <commctrl.h>
#include "controls.h"
#include "menus.h"

namespace audio = hamigaki::audio;
namespace algo = boost::algorithm;
namespace io = boost::iostreams;

class main_window::impl
{
public:
    explicit impl(::HWND handle) : handle_(handle), total_(-1), timer_(0)
    {
        ::HINSTANCE hInstance =
            reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

        play_btn_ = ::CreateWindowEx(
            0, "BUTTON", "Play",
            WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON,
            51, 55, 75, 23, handle_,
            reinterpret_cast< ::HMENU>(static_cast< ::UINT_PTR>(IDC_PLAY)),
            hInstance, 0
        );

        stop_btn_ = ::CreateWindowEx(
            0, "BUTTON", "Stop",
            WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_PUSHBUTTON,
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

            audio::pcm_format fmt = wav.format();

            info_.container = "RIFF";
            if ((fmt.type == audio::float_le32) ||
                (fmt.type == audio::float_le64) )
            {
                info_.encoding = "Floating Point PCM (little-endian)";
            }
            else
                info_.encoding = "Linear PCM (little-endian)";
            info_.length = wav.total() / fmt.block_size() / fmt.rate;
            info_.bit_rate = fmt.bits() * fmt.rate * fmt.channels;
            info_.bits = fmt.bits();
            info_.sampling_rate = fmt.rate;
            info_.channels = fmt.channels;

            block_size_ = fmt.channels;
            total_ = static_cast<int>(
                wav.total() / (fmt.block_size()/fmt.channels)
            );

            if (fmt.type == audio::int8)
                fmt.type = audio::uint8;
            else
                fmt.type = audio::int_le16;

            ::EnableWindow(slider_, TRUE);
            ::SendMessage(
                slider_, TBM_SETRANGEMAX, FALSE, total_/block_size_);
            ::SendMessage(slider_, TBM_SETPOS, TRUE, 0);

            player_.open(
                audio::widen<double>(wav),
                audio::widen<double>(audio::pcm_sink(fmt)),
                fmt.optimal_buffer_size()
            );
        }
        else if (algo::iends_with(filename, ".ogg"))
        {
            audio::vorbis_file_source vf(filename);

            const audio::vorbis_info& info = vf.info();

            info_.container = "Ogg";
            info_.encoding = "Vorbis";
            info_.bit_rate = info.bitrate_nominal;
            info_.bits = 32;
            info_.sampling_rate = info.rate;
            info_.channels = info.channels;

            block_size_ = static_cast<int>(info.channels);
            total_ = static_cast<int>(vf.total());
            if (total_ > 0)
            {
                info_.length = vf.total() / info.rate;
                ::EnableWindow(slider_, TRUE);
                ::SendMessage(
                    slider_, TBM_SETRANGEMAX, FALSE, total_/block_size_);
                ::SendMessage(slider_, TBM_SETPOS, TRUE, 0);
            }
            else
            {
                info_.length = -1;
                ::EnableWindow(slider_, FALSE);
            }

            audio::pcm_format fmt;
            fmt.type = audio::int_le16;
            fmt.channels = info.channels;
            fmt.rate = info.rate;

            player_.open(
                vf, audio::widen<float>(audio::pcm_sink(fmt)),
                vf.optimal_buffer_size()
            );
        }
        else if (
            (algo::iends_with(filename, ".aiff")) ||
            (algo::iends_with(filename, ".aif" )) )
        {
            audio::aiff_file_source aiff(filename);

            audio::pcm_format fmt = aiff.format();

            info_.container = "IFF";
            info_.encoding = "Linear PCM (big-endian)";
            info_.length = aiff.total() / fmt.block_size() / fmt.rate;
            info_.bit_rate = fmt.bits() * fmt.rate * fmt.channels;
            info_.bits = fmt.bits();
            info_.sampling_rate = fmt.rate;
            info_.channels = fmt.channels;

            block_size_ = fmt.channels;
            total_ = static_cast<int>(
                aiff.total() / (fmt.block_size()/fmt.channels)
            );

            if (fmt.type == audio::int8)
                fmt.type = audio::uint8;
            else
                fmt.type = audio::int_le16;

            ::EnableWindow(slider_, TRUE);
            ::SendMessage(
                slider_, TBM_SETRANGEMAX, FALSE, total_/block_size_);
            ::SendMessage(slider_, TBM_SETPOS, TRUE, 0);

            player_.open(
                audio::widen<boost::int_t<16> >(aiff),
                audio::widen<boost::int_t<16> >(audio::pcm_sink(fmt)),
                fmt.optimal_buffer_size()
            );
        }
        else
            throw std::runtime_error("unsupported file format");

        ::EnableWindow(play_btn_, TRUE);

        ::EnableMenuItem(
            ::GetMenu(handle_), ID_FILE_PROP, MF_BYCOMMAND|MF_ENABLED);
    }

    void play()
    {
        if (total_ >= 0)
        {
            ::EnableWindow(slider_, FALSE);

            ::LONG pos = ::SendMessage(slider_, TBM_GETPOS, 0, 0);
            ::LONG max_val = ::SendMessage(slider_, TBM_GETRANGEMAX, 0, 0);
            if (pos != max_val)
                player_.seek(pos*block_size_, BOOST_IOS::beg);
            else
            {
                ::SendMessage(slider_, TBM_SETPOS, TRUE, 0);
                player_.seek(0, BOOST_IOS::beg);

            }
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

        update_progress();
    }

    void pause()
    {
        ::KillTimer(handle_, timer_);
        timer_ = 0;
        player_.stop();

        update_progress();
    }

    bool playing() const
    {
        return timer_ != 0;
    }

    audio_info info() const
    {
        return info_;
    }

private:
    ::HWND handle_;
    ::HWND play_btn_;
    ::HWND stop_btn_;
    ::HWND slider_;
    audio::background_player player_;
    int total_;
    int block_size_;
    ::UINT_PTR timer_;
    audio_info info_;

    static void CALLBACK timer_callback(
        ::HWND, ::UINT, ::UINT_PTR idEvent, ::DWORD)
    {
        reinterpret_cast<impl*>(idEvent)->update_progress();
    }

    void update_progress()
    {
        if (player_.playing())
        {
            if (total_ >= 0)
            {
                const int pos = static_cast<int>(player_.tell());
                ::SendMessage(slider_, TBM_SETPOS, TRUE, pos/block_size_);
            }
        }
        else
        {
            if (timer_ != 0)
            {
                ::KillTimer(handle_, timer_);
                timer_ = 0;
            }
            ::SetWindowText(play_btn_, "Play");
            ::EnableWindow(stop_btn_, FALSE);

            if (total_ >= 0)
            {
                const int pos = static_cast<int>(player_.tell());
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

audio_info main_window::info() const
{
    return pimpl_->info();
}
