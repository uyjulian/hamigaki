// binary_io.hpp: binary I/O using struct_traits

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary

#ifndef HAMIGAKI_BINARY_BINARY_IO_HPP
#define HAMIGAKI_BINARY_BINARY_IO_HPP

#include <boost/config.hpp>

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/for_each.hpp>
#include <cstring>
#include <string>

namespace hamigaki {

template<endianness E, class T>
struct binary_io_traits;

namespace detail
{

template<endianness Parent, endianness Self>
struct endian_select
{
    static const endianness endian = (Self == default_) ? Parent : Self;
};

template<endianness E, class Struct>
class read_member
{
public:
    read_member(const char* data, Struct& x) : data_(data), ptr_(&x)
    {
    }

    template<class T>
    void operator()(const T&)
    {
        typedef endian_select<E,T::endian> sel;

        binary_io_traits<sel::endian, typename T::member_type>::read(
            data_,
            T()(*ptr_)
        );

        data_ += member_size<T>::value;
    }

    template<std::size_t Size>
    void operator()(const padding<Size>&)
    {
        data_ += Size;
    }

private:
    const char* data_;
    Struct* ptr_;
};


template<endianness E, class Struct>
class write_member
{
public:
    write_member(char* data, const Struct& x) : data_(data), ptr_(&x)
    {
    }

    template<class T>
    void operator()(const T&)
    {
        typedef endian_select<E,T::endian> sel;

        binary_io_traits<sel::endian, typename T::member_type>::write(
            data_,
            T()(*ptr_)
        );

        data_ += member_size<T>::value;
    }

    template<std::size_t Size>
    void operator()(const padding<Size>&)
    {
        data_ += Size;
    }

private:
    char* data_;
    const Struct* ptr_;
};

} // namespace detail

template<endianness E, class T>
struct binary_io_traits
{
    static void read(const char* s, T& x)
    {
        typedef typename struct_traits<T>::members members;

        boost::mpl::for_each<members>(detail::read_member<E,T>(s, x));
    }

    static void write(char* s, const T& x)
    {
        typedef typename struct_traits<T>::members members;

        boost::mpl::for_each<members>(detail::write_member<E,T>(s, x));
    }
};

template<endianness E>
struct binary_io_traits<E,char>
{
    static void read(const char* s, char& x)
    {
        x = *s;
    }

    static void write(char* s, const char& x)
    {
        *s = x;
    }
};

template<endianness E>
struct binary_io_traits<E,signed char>
{
    static void read(const char* s, signed char& x)
    {
        x = static_cast<signed char>(*s);
    }

    static void write(char* s, const signed char& x)
    {
        *s = static_cast<char>(x);
    }
};

template<endianness E>
struct binary_io_traits<E,unsigned char>
{
    static void read(const char* s, unsigned char& x)
    {
        x = static_cast<unsigned char>(*s);
    }

    static void write(char* s, const unsigned char& x)
    {
        *s = static_cast<char>(x);
    }
};

template<endianness E>
struct binary_io_traits<E,short>
{
    static void read(const char* s, short& x)
    {
        x = static_cast<short>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const short& x)
    {
        encode_int<E,sizeof(x)>(s, x);
    }
};

template<endianness E>
struct binary_io_traits<E,unsigned short>
{
    static void read(const char* s, unsigned short& x)
    {
        x = static_cast<unsigned short>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const unsigned short& x)
    {
        encode_uint<E,sizeof(x)>(s, x);
    }
};

template<endianness E>
struct binary_io_traits<E,int>
{
    static void read(const char* s, int& x)
    {
        x = static_cast<int>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const int& x)
    {
        encode_int<E,sizeof(x)>(s, x);
    }
};

template<endianness E>
struct binary_io_traits<E,unsigned>
{
    static void read(const char* s, unsigned& x)
    {
        x = static_cast<unsigned>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const unsigned& x)
    {
        encode_uint<E,sizeof(x)>(s, x);
    }
};

template<endianness E>
struct binary_io_traits<E,long>
{
    static void read(const char* s, long& x)
    {
        x = static_cast<long>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const long& x)
    {
        encode_int<E,sizeof(x)>(s, x);
    }
};

template<endianness E>
struct binary_io_traits<E,unsigned long>
{
    static void read(const char* s, unsigned long& x)
    {
        x = static_cast<unsigned long>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const unsigned long& x)
    {
        encode_uint<E,sizeof(x)>(s, x);
    }
};

#if defined(BOOST_HAS_LONG_LONG)
template<endianness E>
struct binary_io_traits<E,long long>
{
    static void read(const char* s, long long& x)
    {
        x = static_cast<long long>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const long long& x)
    {
        encode_int<E,sizeof(x)>(s, x);
    }
};

template<endianness E>
struct binary_io_traits<E,unsigned long long>
{
    static void read(const char* s, unsigned long long& x)
    {
        x = static_cast<unsigned long long>(decode_int<E,sizeof(x)>(s));
    }

    static void write(char* s, const unsigned long long& x)
    {
        encode_uint<E,sizeof(x)>(s, x);
    }
};
#endif // defined(BOOST_HAS_LONG_LONG)

template<endianness E, class T, std::size_t N>
struct binary_io_traits<E, T[N]>
{
    static const std::size_t value_size = hamigaki::binary_size<T>::value;

    static void read(const char* s, T (&x)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
            binary_io_traits<E,T>::read(s+value_size*i, x[i]);
    }

    static void write(char* s, const T (&x)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
            binary_io_traits<E,T>::write(s+value_size*i, x[i]);
    }
};

// optimization
template<endianness E, std::size_t N>
struct binary_io_traits<E, char[N]>
{
    static void read(const char* s, char (&x)[N])
    {
        std::memcpy(x, s, N);
    }

    static void write(char* s, const char (&x)[N])
    {
        std::memcpy(s, x, N);
    }
};


template<class T>
inline void binary_read(const char* s, T& x)
{
    binary_io_traits<native,T>::read(s, x);
}

template<endianness E, class T>
inline void binary_read(const char* s, T& x)
{
    binary_io_traits<E,T>::read(s, x);
}

template<class T>
inline T binary_read(const char* s)
{
    T tmp;
    binary_io_traits<native,T>::read(s, tmp);
    return tmp;
}

template<endianness E, class T>
inline T binary_read(const char* s)
{
    T tmp;
    binary_io_traits<E,T>::read(s, tmp);
    return tmp;
}


template<class T>
inline void binary_write(char* s, const T& x)
{
    binary_io_traits<native,T>::write(s, x);
}

template<endianness E, class T>
inline void binary_write(char* s, const T& x)
{
    binary_io_traits<E,T>::write(s, x);
}


template<class T>
inline void binary_write(std::string& s, const T& x)
{
    char data[binary_size<T>::value];
    std::memset(data, 0, sizeof(data));
    binary_io_traits<native,T>::write(data, x);
    s.append(data, sizeof(data));
}

template<endianness E, class T>
inline void binary_write(std::string& s, const T& x)
{
    char data[binary_size<T>::value];
    std::memset(data, 0, sizeof(data));
    binary_io_traits<E,T>::write(data, x);
    s.append(data, sizeof(data));
}

} // End namespace hamigaki.

#endif // HAMIGAKI_BINARY_BINARY_IO_HPP
