//  vorbis_encoder.cpp: vorbisenc device adapter

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/vorbis_encoder.hpp>
#include <hamigaki/audio/detail/endian.hpp>
#include <vorbis/vorbisenc.h>
#include <cstdlib>
#include <ctime>

namespace hamigaki { namespace audio {

namespace detail
{

class vorbisenc
{
public:
    long channels() const
    {
        return channels_;
    }

    long rate() const
    {
        return rate_;
    }

    void open(void* self, long channels, long rate, float quality,
        vorbis_encoder_base::write_func write,
        vorbis_encoder_base::close_func close)
    {
        file_ = self;
        channels_ = channels;
        rate_ = rate;
        write_ = write;
        close_ = close;

        ::vorbis_info_init(&vi_);

        ::vorbis_encode_init_vbr(&vi_, channels, rate, quality);

        open_impl();
    }

    void open(void* self, long channels, long rate,
        const vorbis_encode_params& params,
        vorbis_encoder_base::write_func write,
        vorbis_encoder_base::close_func close)
    {
        file_ = self;
        channels_ = channels;
        write_ = write;
        close_ = close;

        ::vorbis_info_init(&vi_);

        ::vorbis_encode_init(&vi_, channels, rate,
            params.max_bitrate, params.nominal_bitrate, params.min_bitrate);

        open_impl();
    }

    void close()
    {
        try
        {
            write_to_downstream(0);
            (*close_)(file_);
        }
        catch (...)
        {
        }
        ::ogg_stream_clear(&os_);
        ::vorbis_block_clear(&vb_);
        ::vorbis_dsp_clear(&vd_);
        ::vorbis_comment_clear(&vc_);
        ::vorbis_info_clear(&vi_);
    }

    std::streamsize write(const float* s, std::streamsize n)
    {
        if (n % channels_ != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid write size");

        const std::streamsize count = n / channels_;
        float** buffer = ::vorbis_analysis_buffer(&vd_, count);

        for (std::streamsize i = 0; i < count; ++i)
        {
            for (std::streamsize j = 0; j < channels_; ++j)
                buffer[j][i] = *(s++);
        }

        write_to_downstream(count);

        return n;
    }

private:
    ::ogg_stream_state  os_;
    ::ogg_page          og_;
    ::ogg_packet        op_;
    ::vorbis_info       vi_;
    ::vorbis_comment    vc_;
    ::vorbis_dsp_state  vd_;
    ::vorbis_block      vb_;

    void* file_;
    long channels_;
    long rate_;
    vorbis_encoder_base::write_func write_;
    vorbis_encoder_base::close_func close_;

    void open_impl()
    {
        ::vorbis_comment_init(&vc_);

        ::vorbis_analysis_init(&vd_, &vi_);
        ::vorbis_block_init(&vd_, &vb_);

        std::srand(std::time(0));
        ::ogg_stream_init(&os_, std::rand());

        ::ogg_packet header;
        ::ogg_packet header_comm;
        ::ogg_packet header_code;

        ::vorbis_analysis_headerout(
            &vd_, &vc_, &header, &header_comm, &header_code);
        ::ogg_stream_packetin(&os_, &header);
        ::ogg_stream_packetin(&os_, &header_comm);
        ::ogg_stream_packetin(&os_, &header_code);

        while (::ogg_stream_flush(&os_, &og_) != 0)
        {
            write_helper(og_.header, og_.header_len);
            write_helper(og_.body, og_.body_len);
        }
    }

    void write_helper(const unsigned char* s, long n)
    {
        (*write_)(file_, reinterpret_cast<const char*>(s), n);
    }

    void write_to_downstream(int count)
    {
        ::vorbis_analysis_wrote(&vd_, count);

        bool eos = false;
        while (::vorbis_analysis_blockout(&vd_, &vb_) == 1)
        {
            ::vorbis_analysis(&vb_, 0);
            ::vorbis_bitrate_addblock(&vb_);

            while (::vorbis_bitrate_flushpacket(&vd_, &op_))
            {
                ::ogg_stream_packetin(&os_, &op_);

                while (!eos)
                {
                    int result = ::ogg_stream_pageout(&os_, &og_);
                    if (result == 0)
                        break;

                    write_helper(og_.header, og_.header_len);
                    write_helper(og_.body, og_.body_len);

                    if (::ogg_page_eos(&og_))
                        eos = true;
                }
            }
        }
    }
};

vorbis_encoder_base::vorbis_encoder_base()
    : ptr_(new vorbisenc), is_open_(false)
{
}

vorbis_encoder_base::~vorbis_encoder_base()
{
    close();
    delete static_cast<vorbisenc*>(ptr_);
}

void vorbis_encoder_base::open(
    void* self, long channels, long rate, float quality,
    vorbis_encoder_base::write_func write,
    vorbis_encoder_base::close_func close)
{
    if (is_open_)
        this->close();

    vorbisenc& enc = *static_cast<vorbisenc*>(ptr_);
    enc.open(self, channels, rate, quality, write, close);
    is_open_ = true;
}

void vorbis_encoder_base::open(
    void* self, long channels, long rate,
    const vorbis_encode_params& params,
    vorbis_encoder_base::write_func write,
    vorbis_encoder_base::close_func close)
{
    if (is_open_)
        this->close();

    vorbisenc& enc = *static_cast<vorbisenc*>(ptr_);
    enc.open(self, channels, rate, params, write, close);
    is_open_ = true;
}

void vorbis_encoder_base::close()
{
    if (is_open_)
    {
        vorbisenc& enc = *static_cast<vorbisenc*>(ptr_);
        enc.close();
        is_open_ = false;
    }
}

std::streamsize vorbis_encoder_base::write(const float* s, std::streamsize n)
{
    vorbisenc& enc = *static_cast<vorbisenc*>(ptr_);
    return enc.write(s, n);
}

} // namespace detail

} } // End namespaces audio, hamigaki.
