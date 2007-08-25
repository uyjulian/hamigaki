// vorbis_file.cpp: vorbisfile device adapter

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/vorbis_file.hpp>
#include <vorbis/vorbisfile.h>
#include <errno.h>

namespace hamigaki { namespace audio {

vorbis_error::vorbis_error(int error)
    : BOOST_IOSTREAMS_FAILURE("vorbis error"), error_(error) 
{
}

void vorbis_error::check(int error)
{
    if (error < 0)
        throw vorbis_error(error);
}

namespace detail
{

HAMIGAKI_AUDIO_DECL void clear_errno()
{
    errno = 0;
}

HAMIGAKI_AUDIO_DECL int get_errno()
{
    return errno;
}

HAMIGAKI_AUDIO_DECL void set_errno_eio()
{
    errno = EIO;
}

vorbis_file_base::vorbis_file_base()
    : file_ptr_(new ::OggVorbis_File), is_open_(false)
{
}

vorbis_file_base::~vorbis_file_base()
{
    delete static_cast<OggVorbis_File*>(file_ptr_);
}

void vorbis_file_base::open(void* self,
    vorbis::read_func read, vorbis::seek_func seek,
    vorbis::close_func close, vorbis::tell_func tell)
{
    if (is_open_)
        vorbis_file_base::close();

    ::ov_callbacks callbacks =
    {
        read, seek, close, tell
    };

    vorbis_error::check(::ov_open_callbacks(self,
        static_cast<OggVorbis_File*>(file_ptr_), 0, 0, callbacks));
    is_open_ = true;
}

void vorbis_file_base::close()
{
    if (is_open_)
    {
        ::ov_clear(static_cast<OggVorbis_File*>(file_ptr_));
        is_open_ = false;
    }
}

long vorbis_file_base::read_samples(
    float**& buffer, int samples)
{
    long res = ::ov_read_float(
        static_cast<OggVorbis_File*>(file_ptr_), &buffer, samples, 0);
    if (res < 0)
        throw vorbis_error(static_cast<int>(res));
    return res;
}

void vorbis_file_base::seek(boost::int64_t pos)
{
    vorbis_error::check(
        ::ov_pcm_seek(static_cast<OggVorbis_File*>(file_ptr_), pos));
}

boost::int64_t vorbis_file_base::tell()
{
    boost::int64_t pos =
        ::ov_pcm_tell(static_cast<OggVorbis_File*>(file_ptr_));
    if (pos < 0)
        throw vorbis_error(static_cast<int>(pos));
    return pos;
}

boost::int64_t vorbis_file_base::total()
{
    boost::int64_t pos =
        ::ov_pcm_total(static_cast<OggVorbis_File*>(file_ptr_), -1);
    if (pos < 0)
        throw vorbis_error(static_cast<int>(pos));
    return pos;
}

std::pair<const char**,const char**>
vorbis_file_base::comments() const
{
    if (!is_open_)
        throw vorbis_error(OV_EINVAL);

    ::vorbis_comment* ptr =
        ::ov_comment(static_cast<OggVorbis_File*>(file_ptr_), -1);

    return std::pair<const char**,const char**>(
        const_cast<const char**>(ptr->user_comments),
        const_cast<const char**>(ptr->user_comments + ptr->comments));
}

const char* vorbis_file_base::vendor() const
{
    if (!is_open_)
        throw vorbis_error(OV_EINVAL);

    return ::ov_comment(static_cast<OggVorbis_File*>(file_ptr_), -1)->vendor;
}

vorbis_info vorbis_file_base::info() const
{
    if (!is_open_)
        throw vorbis_error(OV_EINVAL);

    const ::vorbis_info* ptr =
        ::ov_info(static_cast<OggVorbis_File*>(file_ptr_), -1);

    vorbis_info info;
    info.version = ptr->version;
    info.channels = ptr->channels;
    info.rate = ptr->rate;
    info.bitrate_upper = ptr->bitrate_upper;
    info.bitrate_nominal = ptr->bitrate_nominal;
    info.bitrate_lower = ptr->bitrate_lower;

    return info;
}

bool vorbis_file_base::is_open() const
{
    return is_open_;
}

} // namespace detail

} } // End namespaces audio, hamigaki.
