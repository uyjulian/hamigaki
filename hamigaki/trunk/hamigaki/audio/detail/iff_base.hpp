// iff_base.hpp: IFF/RIFF base classes

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_IFF_BASE_HPP
#define HAMIGAKI_AUDIO_DETAIL_IFF_BASE_HPP

#include <hamigaki/audio/detail/closer.hpp>
#include <hamigaki/binary/endian.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/cstdint.hpp>
#include <cstring>
#include <string>

namespace hamigaki { namespace audio { namespace detail {

struct iff_chunk_header
{
    char id[4];
    boost::uint32_t size;
};

template<typename Source, endianness E>
class iff_chunk_source
{
    typedef boost::iostreams::stream_offset off_t;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag {};

    iff_chunk_source() : src_ptr_(0)
    {
        std::memset(head_.id, 0, sizeof(head_.id));
        head_.size = 0;
        start_ = 0;
        position_ = 0;
    }

    explicit iff_chunk_source(Source& src) : src_ptr_(&src)
    {
        head_ = read_chunk_header();
        start_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(*src_ptr_, 0, BOOST_IOS::cur));
        position_ = 0;
    }

    std::string chunk_id() const
    {
        return std::string(head_.id, 4);
    }

    boost::iostreams::stream_offset start_offset() const
    {
        return start_;
    }

    boost::iostreams::stream_offset end_offset() const
    {
        return start_ + head_.size;
    }

    boost::iostreams::stream_offset total() const
    {
        return head_.size;
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        if (src_ptr_ == 0)
            return -1;

        off_t rest = head_.size - position_;
        if (rest < static_cast<off_t>(n))
            n = static_cast<std::streamsize>(rest);
        if (n == 0)
            return -1;

        std::streamsize amt = boost::iostreams::read(*src_ptr_, s, n);
        if (amt != n)
            throw BOOST_IOSTREAMS_FAILURE("read error");
        position_ += amt;
        return amt;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        if (src_ptr_ == 0)
            throw BOOST_IOSTREAMS_FAILURE("bad seek");

        if (way == BOOST_IOS::beg)
        {
            if ((off < 0) || (off > head_.size))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = start_ + off;
        }
        else if (way == BOOST_IOS::cur)
        {
            // Optimization for "tell"
            if (off == 0)
                return position_;

            off_t pos = position_ + off;
            if ((pos < 0) || (pos > head_.size))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }
        else
        {
            off_t pos = head_.size + off;
            if ((pos < 0) || (pos > head_.size))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = pos - head_.size;
        }

        std::streampos pos = boost::iostreams::seek(*src_ptr_, off, way);
        position_ = boost::iostreams::position_to_offset(pos) - start_;
        return boost::iostreams::offset_to_position(position_);
    }

private:
    Source* src_ptr_;
    iff_chunk_header head_;
    off_t start_;
    off_t position_;

    iff_chunk_header read_chunk_header()
    {
        char buf[8];
        if (boost::iostreams::read(*src_ptr_, buf, sizeof(buf)) != sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("cannot read chunk header");

        iff_chunk_header chunk;
        std::memcpy(chunk.id, buf, 4);
        chunk.size = decode_uint<E,4>(buf+4);
        return chunk;
    }
};

template<typename Source, endianness E>
class iff_source
{
    typedef iff_chunk_source<Source,E> chunk_type;
    typedef boost::iostreams::stream_offset off_t;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag {};

    explicit iff_source(Source& src, boost::iostreams::stream_offset size)
        : src_(src), size_(size)
    {
        std::streamsize amt =
            boost::iostreams::read(src_, type_, sizeof(type_));
        if (amt != sizeof(type_))
            throw BOOST_IOSTREAMS_FAILURE("cannot read chunk type");

        chunk_ = chunk_type(src_);
    }

    std::string type() const
    {
        return std::string(type_, 4);
    }

    std::string chunk_id() const
    {
        return chunk_.chunk_id();
    }

    void rewind_chunk()
    {
        boost::iostreams::seek(src_, 4, BOOST_IOS::beg);
        chunk_ = chunk_type(src_);
    }

    bool next_chunk()
    {
        off_t next = chunk_.end_offset();
        if (next < size_)
        {
            boost::iostreams::seek(src_, next, BOOST_IOS::beg);
            chunk_ = chunk_type(src_);
            return true;
        }
        else
        {
            chunk_ = chunk_type();
            return false;
        }
    }

    bool select_chunk(const char* name)
    {
        if (chunk_.chunk_id() == name)
        {
            boost::iostreams::seek(chunk_, 0, BOOST_IOS::beg);
            return true;
        }

        rewind_chunk();

        while (chunk_.chunk_id() != name)
        {
            if (!next_chunk())
                return false;
        }

        return true;
    }

    boost::iostreams::stream_offset total() const
    {
        return chunk_.total();
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return chunk_.read(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return chunk_.seek(off, way);
    }

private:
    Source& src_;
    off_t size_;
    char type_[4];
    chunk_type chunk_;
};

template<typename Source, endianness E>
class iff_file_source
{
    typedef iff_chunk_source<Source,E> chunk_type;
    typedef iff_source<chunk_type,E> iff_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag {};

    explicit iff_file_source(Source& src)
        : root_(src), iff_(root_, root_.total())
    {
    }

    std::string type() const
    {
        return iff_.type();
    }

    std::string chunk_id() const
    {
        return iff_.chunk_id();
    }

    void rewind_chunk()
    {
        iff_.rewind_chunk();
    }

    bool next_chunk()
    {
        return iff_.next_chunk();
    }

    bool select_chunk(const char* name)
    {
        return iff_.select_chunk(name);
    }

    boost::iostreams::stream_offset total() const
    {
        return iff_.total();
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return iff_.read(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return iff_.seek(off, way);
    }

private:
    chunk_type root_;
    iff_type iff_;
};

template<typename Sink, endianness E>
class iff_chunk_sink
{
    typedef boost::iostreams::stream_offset off_t;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::closable_tag,
        boost::iostreams::output_seekable,
        boost::iostreams::device_tag {};

    iff_chunk_sink() : snk_ptr_(0)
    {
        std::memset(head_.id, 0, sizeof(head_.id));
        head_.size = 0;
        start_ = 0;
        position_ = 0;
    }

    iff_chunk_sink(Sink& sink, const char* id) : snk_ptr_(&sink)
    {
        std::memcpy(head_.id, id, 4);
        head_.size = 0;

        char buf[8];
        std::memcpy(buf, id, 4);
        std::memset(&buf[4], 0, 4);
        boost::iostreams::write(*snk_ptr_, buf, sizeof(buf));

        start_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(*snk_ptr_, 0, BOOST_IOS::cur));
        position_ = 0;
    }

    std::string chunk_id() const
    {
        return std::string(head_.id, 4);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        if (snk_ptr_ == 0)
            throw BOOST_IOSTREAMS_FAILURE("bad write");

        off_t rest = static_cast<off_t>(0xFFFFFFFF) - position_;
        if (rest < static_cast<off_t>(n))
            throw BOOST_IOSTREAMS_FAILURE("bad write");

        std::streamsize amt = boost::iostreams::write(*snk_ptr_, s, n);
        if (amt != n)
            throw BOOST_IOSTREAMS_FAILURE("write error");

        position_ += amt;
        if (position_ > static_cast<off_t>(head_.size))
            head_.size = static_cast<boost::uint32_t>(position_);
        return amt;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        if (snk_ptr_ == 0)
            throw BOOST_IOSTREAMS_FAILURE("bad seek");

        if (way == BOOST_IOS::beg)
        {
            if ((off < 0) || (off > head_.size))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = start_ + off;
        }
        else if (way == BOOST_IOS::cur)
        {
            // Optimization for "tell"
            if (off == 0)
                return position_;

            off_t pos = position_ + off;
            if ((pos < 0) || (pos > head_.size))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }
        else
        {
            off_t pos = head_.size + off;
            if ((pos < 0) || (pos > head_.size))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = pos - head_.size;
        }

        std::streampos pos = boost::iostreams::seek(*snk_ptr_, off, way);
        position_ = boost::iostreams::position_to_offset(pos) - start_;
        return boost::iostreams::offset_to_position(position_);
    }

    void close()
    {
        if (snk_ptr_ == 0)
            return;

        boost::iostreams::seek(*snk_ptr_, start_-4, BOOST_IOS::beg);

        char buf[4];
        encode_uint<E,4>(&buf[0], head_.size);

        boost::iostreams::write(*snk_ptr_, buf, sizeof(buf));
        boost::iostreams::seek(*snk_ptr_, head_.size, BOOST_IOS::cur);

        snk_ptr_ = 0;
        std::memset(head_.id, 0, sizeof(head_.id));
        head_.size = 0;
        start_ = 0;
        position_ = 0;
    }

private:
    Sink* snk_ptr_;
    iff_chunk_header head_;
    off_t start_;
    off_t position_;
};


template<typename Sink, endianness E>
class iff_sink
{
    typedef iff_chunk_sink<Sink,E> chunk_type;
    typedef boost::iostreams::stream_offset off_t;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::closable_tag,
        boost::iostreams::output_seekable,
        boost::iostreams::device_tag {};

    iff_sink(Sink& sink, const char* type) : sink_(sink)
    {
        std::memcpy(type_, type, sizeof(type_));
        boost::iostreams::write(sink_, type_, sizeof(type_));
    }

    std::string type() const
    {
        return std::string(type_, 4);
    }

    std::string chunk_id() const
    {
        return chunk_.chunk_id();
    }

    void create_chunk(const char* id)
    {
        chunk_.close();
        chunk_ = chunk_type(sink_, id);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return chunk_.write(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return chunk_.seek(off, way);
    }

    void close()
    {
        chunk_.close();
    }

private:
    Sink& sink_;
    char type_[4];
    chunk_type chunk_;
};

template<typename Sink, endianness E>
class iff_file_sink
{
    typedef iff_chunk_sink<Sink,E> chunk_type;
    typedef iff_sink<chunk_type,E> iff_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::closable_tag,
        boost::iostreams::output_seekable,
        boost::iostreams::device_tag {};

    iff_file_sink(Sink& sink, const char* id, const char* type)
        : root_(sink, id), iff_(root_, type)
    {
    }

    std::string type() const
    {
        return iff_.type();
    }

    std::string chunk_id() const
    {
        return iff_.chunk_id();
    }

    void create_chunk(const char* id)
    {
        return iff_.create_chunk(id);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return iff_.write(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return iff_.seek(off, way);
    }

    void close()
    {
        bool nothrow = false;
        detail::device_closer<chunk_type> close_root(root_, nothrow);
        detail::device_closer<iff_type> close_iff(iff_, nothrow);
    }

private:
    chunk_type root_;
    iff_type iff_;
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_IFF_BASE_HPP
