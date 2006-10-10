//  binary_io.hpp: a wrapper for <hamigaki/binary_io.hpp>

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BINARY_IO_HPP
#define HAMIGAKI_IOSTREAMS_BINARY_IO_HPP

#include <hamigaki/iostreams/blocking.hpp>
#include <hamigaki/binary_io.hpp>
#include <cstring>
#include <new>

namespace hamigaki { namespace iostreams {

template<class Source, class T>
inline bool binary_read(Source& src, T& x, const std::nothrow_t&)
{
    char data[struct_size<T>::type::value];
    std::memset(data, 0, sizeof(data));

    const std::streamsize size = static_cast<std::streamsize>(sizeof(data));
    boost::iostreams::non_blocking_adapter<Source> nb(src);
    if (boost::iostreams::read(nb, data, size) != size)
        return false;

    hamigaki::binary_read(data, x);
    return true;
}

template<class Source, class T>
inline void binary_read(Source& src, T& x)
{
    if (!hamigaki::iostreams::binary_read(src, x, std::nothrow))
        throw boost::iostreams::detail::bad_read();
}

template<class Sink, class T>
inline bool binary_write(Sink& sink, const T& x, const std::nothrow_t&)
{
    char data[struct_size<T>::type::value];
    std::memset(data, 0, sizeof(data));
    hamigaki::binary_write(data, x);

    const std::streamsize size = static_cast<std::streamsize>(sizeof(data));
    boost::iostreams::non_blocking_adapter<Sink> nb(sink);
    return boost::iostreams::write(nb, data, size) == size;
}

template<class Sink, class T>
inline void binary_write(Sink& sink, const T& x)
{
    if (!hamigaki::iostreams::binary_write(sink, x, std::nothrow))
        throw boost::iostreams::detail::bad_write();
}


template<endianness E, class Source>
inline boost::uint8_t read_uint8(Source& src)
{
    char buf[1];
    return hamigaki::decode_uint<E,1>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::int8_t read_int8(Source& src)
{
    char buf[1];
    return hamigaki::decode_int<E,1>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::uint16_t read_uint16(Source& src)
{
    char buf[2];
    return hamigaki::decode_uint<E,2>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::int16_t read_int16(Source& src)
{
    char buf[2];
    return hamigaki::decode_int<E,2>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::uint32_t read_uint32(Source& src)
{
    char buf[4];
    return hamigaki::decode_uint<E,4>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::int32_t read_int32(Source& src)
{
    char buf[4];
    return hamigaki::decode_int<E,4>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::uint64_t read_uint64(Source& src)
{
    char buf[8];
    return hamigaki::decode_uint<E,8>(
        hamigaki::iostreams::blocking_read(src, buf));
}

template<endianness E, class Source>
inline boost::int64_t read_int64(Source& src)
{
    char buf[8];
    return hamigaki::decode_int<E,8>(
        hamigaki::iostreams::blocking_read(src, buf));
}


template<endianness E, class Sink>
inline void write_uint8(Sink& sink, boost::uint8_t x)
{
    char buf[1];
    hamigaki::iostreams::blocking_write(
        sink, hamigaki::encode_uint<E,1>(buf, x));
}

template<endianness E, class Sink>
inline void write_int8(Sink& sink, boost::int8_t x)
{
    char buf[1];
    hamigaki::encode_int<E,1>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

template<endianness E, class Sink>
inline void write_uint16(Sink& sink, boost::uint16_t x)
{
    char buf[2];
    hamigaki::encode_uint<E,2>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

template<endianness E, class Sink>
inline void write_int16(Sink& sink, boost::int16_t x)
{
    char buf[2];
    hamigaki::encode_int<E,2>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

template<endianness E, class Sink>
inline void write_uint32(Sink& sink, boost::uint32_t x)
{
    char buf[4];
    hamigaki::encode_uint<E,4>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

template<endianness E, class Sink>
inline void write_int32(Sink& sink, boost::int32_t x)
{
    char buf[4];
    hamigaki::encode_int<E,4>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

template<endianness E, class Sink>
inline void write_uint64(Sink& sink, boost::uint64_t x)
{
    char buf[8];
    hamigaki::encode_uint<E,8>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

template<endianness E, class Sink>
inline void write_int64(Sink& sink, boost::int64_t x)
{
    char buf[8];
    hamigaki::encode_int<E,8>(buf, x);
    hamigaki::iostreams::blocking_write(sink, buf);
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BINARY_IO_HPP
