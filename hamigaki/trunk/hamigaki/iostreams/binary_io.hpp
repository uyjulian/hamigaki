//  binary_io.hpp: a wrapper for <hamigaki/binary_io.hpp>

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BINARY_IO_HPP
#define HAMIGAKI_IOSTREAMS_BINARY_IO_HPP

#include <hamigaki/binary_io.hpp>
#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
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

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BINARY_IO_HPP
