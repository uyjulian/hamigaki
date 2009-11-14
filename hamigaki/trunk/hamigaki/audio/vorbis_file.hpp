// vorbis_file.hpp: vorbis_file device adaptor

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_VORBIS_FILE_HPP
#define HAMIGAKI_AUDIO_VORBIS_FILE_HPP

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/audio/detail/auto_link/hamigaki_audio.hpp>
#include <hamigaki/audio/detail/auto_link/ogg.hpp>
#include <hamigaki/audio/detail/auto_link/vorbis.hpp>
#include <hamigaki/audio/detail/auto_link/vorbisfile.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/arbitrary_positional_facade.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <hamigaki/iostreams/traits.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/detail/select.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <cstddef>
#include <cstring>
#include <limits>
#include <utility>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4275)
#endif

namespace hamigaki { namespace audio {

namespace vorbis
{

// Typedefs
typedef std::size_t (*read_func)(void*, std::size_t, std::size_t, void*);
typedef int (*seek_func)(void*, boost::int64_t, int);
typedef int (*close_func)(void*);
typedef long (*tell_func)(void*);

} // namespace vorbis

struct vorbis_info
{
    int version;
    int channels;
    long rate;

    long bitrate_upper;
    long bitrate_nominal;
    long bitrate_lower;
};

class HAMIGAKI_AUDIO_DECL vorbis_error : public BOOST_IOSTREAMS_FAILURE
{
public:
    explicit vorbis_error(int error);
    int error() const { return error_; }
    static void check(int error);

private:
    int error_;
};

namespace detail
{

HAMIGAKI_AUDIO_DECL void clear_errno();
HAMIGAKI_AUDIO_DECL int get_errno();
HAMIGAKI_AUDIO_DECL void set_errno_eio();

class HAMIGAKI_AUDIO_DECL vorbis_file_base : boost::noncopyable
{
public:
    vorbis_file_base();
    ~vorbis_file_base();

    void open(void* self,
        vorbis::read_func read, vorbis::seek_func seek,
        vorbis::close_func close, vorbis::tell_func tell);

    void close();
    long read_samples(float**& buffer, int samples);
    void seek(boost::int64_t pos);
    boost::int64_t tell();
    boost::int64_t total();

    std::pair<const char**,const char**> comments() const;
    const char* vendor() const;
    vorbis_info info() const;

    bool is_open() const;

private:
    void* file_ptr_;
    bool is_open_;
};

template<typename Source>
struct vorbis_nonseekable_source_traits
{
    static ::size_t read_func(
        void* ptr, ::size_t size, ::size_t nmemb, void* datasource)
    {
        BOOST_ASSERT(size == 1);

        try
        {
            Source& src = *static_cast<Source*>(datasource);
            clear_errno();
            std::streamsize n =
                boost::iostreams::read(src, static_cast<char*>(ptr), nmemb);
            return (n == -1) ? 0 : n;
        }
        catch (...)
        {
            if (get_errno() == 0)
                set_errno_eio();
            return 0;
        }
    }

    static int seek_func(void* datasource, boost::int64_t offset, int whence)
    {
        return -1;
    }

    static int close_func(void* datasource)
    {
        try
        {
            Source& src = *static_cast<Source*>(datasource);
            boost::iostreams::close(src, BOOST_IOS::in);
            return 0;
        }
        catch (...)
        {
            return -1;
        }
    }

    static long tell_func(void* datasource)
    {
        return -1;
    }
};

template<typename Source>
struct vorbis_seekable_source_traits
    : vorbis_nonseekable_source_traits<Source>
{
    static int seek_func(void* datasource, boost::int64_t offset, int whence)
    {
        try
        {
            Source& src = *static_cast<Source*>(datasource);
            if (whence == SEEK_SET)
            {
                boost::iostreams::seek(
                    src, offset, BOOST_IOS::beg, BOOST_IOS::in);
            }
            else if (whence == SEEK_CUR)
            {
                boost::iostreams::seek(
                    src, offset, BOOST_IOS::cur, BOOST_IOS::in);
            }
            else
            {
                boost::iostreams::seek(
                    src, offset, BOOST_IOS::end, BOOST_IOS::in);
            }
            return 0;
        }
        catch (...)
        {
            return -1;
        }
    }

    static long tell_func(void* datasource)
    {
        try
        {
            Source& src = *static_cast<Source*>(datasource);
            return static_cast<long>(
                boost::iostreams::position_to_offset(
                    boost::iostreams::seek(src, 0, BOOST_IOS::cur)
                )
            );
        }
        catch (...)
        {
            return -1;
        }
    }
};

template<typename Source>
class vorbis_file_source_impl
    : public iostreams::arbitrary_positional_facade<
        vorbis_file_source_impl<Source>, float, 255
    >
{
    friend class iostreams::core_access;

private:
    typedef vorbis_file_source_impl<Source> self_type;
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582)) || \
    BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
    typedef iostreams::arbitrary_positional_facade<
        vorbis_file_source_impl<Source>, float, 255
    > facade_type;
#else
    typedef typename self_type::arbitrary_positional_facade_ facade_type;
#endif

    typedef typename
        boost::iostreams::select<
            boost::iostreams::is_direct<Source>,
                boost::iostreams::detail::direct_adapter<Source>,
            boost::iostreams::else_,
                Source
        >::type value_type;

    typedef typename
        boost::iostreams::select<
            iostreams::is_input_seekable<value_type>,
                vorbis_seekable_source_traits<value_type>,
            boost::iostreams::else_,
                vorbis_nonseekable_source_traits<value_type>
        >::type source_traits;

public:
    BOOST_STATIC_ASSERT((
        boost::is_same<
            char,
            BOOST_DEDUCED_TYPENAME
                boost::iostreams::char_type_of<Source>::type
        >::value
    ));

    explicit vorbis_file_source_impl(const Source& src)
        : src_(src)
    {
        base_.open(
            &src_,
            source_traits::read_func,
            source_traits::seek_func,
            source_traits::close_func,
            source_traits::tell_func);

        facade_type::block_size(base_.info().channels);
    }

    ~vorbis_file_source_impl()
    {
        if (base_.is_open())
            close();
    }

    void close()
    {
        base_.close();
    }

    boost::iostreams::stream_offset total()
    {
        return base_.total() * base_.info().channels;
    }

    std::pair<const char**,const char**> comments() const
    {
        return base_.comments();
    }

    const char* vendor() const
    {
        return base_.vendor();
    }

    vorbis_info info() const
    {
        return base_.info();
    }

    using facade_type::read;
    using facade_type::seek;

private:
    vorbis_file_base base_;
    value_type src_;

    std::streamsize read_blocks(float* s, std::streamsize n)
    {
        std::streamsize channels = base_.info().channels;
        std::streamsize total = 0;
        while (n > 0)
        {
            float** buffer;
            long res = base_.read_samples(buffer, static_cast<int>(n));
            if (res == 0)
                break;

            for (std::streamsize j = 0; j < res; ++j)
            {
                for (std::streamsize i = 0; i < channels; ++i)
                    *(s++) = buffer[i][j];
            }

            total += res*channels;
            n -= res;
        }

        return total != 0 ? total : -1;
    }

    std::streampos seek_blocks(
        boost::iostreams::stream_offset off, std::ios_base::seekdir way)
    {
        if (way == BOOST_IOS::beg)
        {
            base_.seek(off);
            return off;
        }
        else if (way == BOOST_IOS::cur)
        {
            boost::int64_t cur = base_.tell();
            base_.seek(cur + off);
            return cur + off;
        }
        else
        {
            boost::int64_t end = base_.total();
            base_.seek(end + off);
            return end + off;
        }
    }
};

} // namespace detail

template<typename Source>
class basic_vorbis_file_source
{
    typedef detail::vorbis_file_source_impl<Source> impl_type;

public:
    typedef float char_type;

    struct category
        : boost::iostreams::optimally_buffered_tag
        , boost::iostreams::mode_of<Source>::type
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_vorbis_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        const vorbis_info& info = pimpl_->info();
        return info.channels * (info.rate / 5);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return pimpl_->seek(off, way);
    }

    std::pair<const char**,const char**> comments() const
    {
        return pimpl_->comments();
    }

    const char* vendor() const
    {
        return pimpl_->vendor();
    }

    vorbis_info info() const
    {
        return pimpl_->info();
    }

    boost::iostreams::stream_offset total()
    {
        return pimpl_->total();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class vorbis_file_source
{
private:
    typedef basic_vorbis_file_source<iostreams::file_source> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    explicit vorbis_file_source(const std::string& path)
        : impl_(iostreams::file_source(path, BOOST_IOS::in|BOOST_IOS::binary))
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return impl_.optimal_buffer_size();
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

    void close()
    {
        impl_.close();
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return impl_.seek(off, way);
    }

    std::pair<const char**,const char**> comments() const
    {
        return impl_.comments();
    }

    const char* vendor() const
    {
        return impl_.vendor();
    }

    vorbis_info info() const
    {
        return impl_.info();
    }

    boost::iostreams::stream_offset total()
    {
        return impl_.total();
    }

private:
    impl_type impl_;
};

template<typename Source>
inline basic_vorbis_file_source<Source>
make_vorbis_file_source(const Source& src)
{
    return basic_vorbis_file_source<Source>(src);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::basic_vorbis_file_source, 1)

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_AUDIO_VORBIS_FILE_HPP
