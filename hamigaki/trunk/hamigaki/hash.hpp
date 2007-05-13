// hash.hpp: some utilities for Boost.Functional/Hash

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_HASH_HPP
#define HAMIGAKI_HASH_HPP

#include <boost/functional/hash/hash.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace hamigaki {

namespace detail
{

template<std::size_t Size>
struct hash_value_ui64_impl;

template<>
struct hash_value_ui64_impl<4>
{
    std::size_t operator()(boost::uint64_t val) const
    {
        return static_cast<std::size_t>(
            static_cast<boost::uint32_t>(val) ^
            static_cast<boost::uint32_t>(val >> 32)
        );
    }
};

template<>
struct hash_value_ui64_impl<8>
{
    std::size_t operator()(boost::uint64_t val) const
    {
        return static_cast<std::size_t>(val);
    }
};

template<std::size_t Size>
struct hash_value_integer_impl
{
    template<class T>
    std::size_t operator()(T val) const
    {
        return boost::hash_value(val);
    }
};

template<>
struct hash_value_integer_impl<8>
{
    template<class T>
    std::size_t operator()(T val) const
    {
        return hash_value_ui64_impl<
            sizeof(std::size_t)
        >()(static_cast<boost::uint64_t>(val));
    }
};

template<std::size_t Size>
struct hash_value_to_ui32_impl;

template<>
struct hash_value_to_ui32_impl<4>
{
    boost::uint32_t operator()(std::size_t seed) const
    {
        return static_cast<boost::uint32_t>(seed);
    }
};

template<>
struct hash_value_to_ui32_impl<8>
{
    boost::uint32_t operator()(std::size_t seed) const
    {
        boost::uint64_t tmp = static_cast<boost::uint64_t>(seed);

        return
            static_cast<boost::uint32_t>(tmp) ^
            static_cast<boost::uint32_t>(tmp >> 32);
    }
};

} // namespace detail

inline std::size_t hash_value_ui64(boost::uint64_t val)
{
    return detail::hash_value_ui64_impl<sizeof(std::size_t)>()(val);
}

inline std::size_t hash_value_i64(boost::int64_t val)
{
    return hamigaki::hash_value_i64(static_cast<boost::uint64_t>(val));
}

template<class T>
inline std::size_t hash_value_integer(T val)
{
    BOOST_STATIC_ASSERT(::boost::is_integral<T>::value);
    return detail::hash_value_integer_impl<sizeof(T)>()(val);
}

inline boost::uint32_t hash_value_to_ui32(std::size_t seed)
{
    return detail::hash_value_to_ui32_impl<sizeof(std::size_t)>()(seed);
}

} // End namespace hamigaki.

#endif // HAMIGAKI_HASH_HPP
