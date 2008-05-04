// uuid.hpp: Universally Unique IDentifier class

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_UUID_HPP
#define HAMIGAKI_UUID_HPP

#include <boost/config.hpp>

#include <hamigaki/hex_format.hpp>
#include <hamigaki/static_widen.hpp>
#include <boost/detail/endian.hpp>
#include <boost/serialization/level.hpp>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <cstring>
#include <iosfwd>
#include <string>

struct _GUID;

namespace hamigaki {

namespace detail
{

template<class CharT, class Traits, class Allocator>
inline void append_hex(
    std::basic_string<CharT,Traits,Allocator>& s,
    boost::uint8_t n, bool is_upper)
{
    std::pair<CharT,CharT> hex = to_hex_pair<CharT>(n, is_upper);
    s.push_back(hex.first);
    s.push_back(hex.second);
}

} // namespace detail

class uuid : boost::totally_ordered<uuid>
{
public:
    uuid()
    {
        data_.assign(0);
    }

    explicit uuid(const char* s)
    {
        if (*s == '{')
        {
            if (std::strlen(s) != 38)
                invalid_uuid();
            ++s;
        }
        else if (std::strlen(s) != 36)
            invalid_uuid();
        from_string_impl<char>(s);
    }

    explicit uuid(const wchar_t* s)
    {
        if (*s == L'{')
        {
            if (std::char_traits<wchar_t>::length(s) != 38)
                invalid_uuid();
            ++s;
        }
        else if (std::char_traits<wchar_t>::length(s) != 36)
            invalid_uuid();
        from_string_impl<wchar_t>(s);
    }

    uuid(const ::_GUID& id)
    {
#if defined(BOOST_BIG_ENDIAN)
        std::memcpy(data_.c_array(), &id, 16);
#else
        boost::uint8_t tmp[16];
        std::memcpy(tmp, &id, 16);
        data_[0] = tmp[3];
        data_[1] = tmp[2];
        data_[2] = tmp[1];
        data_[3] = tmp[0];
        data_[4] = tmp[5];
        data_[5] = tmp[4];
        data_[6] = tmp[7];
        data_[7] = tmp[6];
        for (std::size_t i = 8; i < 16; ++i)
            data_[i] = tmp[i];
#endif
    }

    const ::_GUID& copy(::_GUID& id) const
    {
#if defined(BOOST_BIG_ENDIAN)
        std::memcpy(&id, data_.c_array(), 16);
#else
        boost::uint8_t tmp[16];
        tmp[3] = data_[0];
        tmp[2] = data_[1];
        tmp[1] = data_[2];
        tmp[0] = data_[3];
        tmp[5] = data_[4];
        tmp[4] = data_[5];
        tmp[7] = data_[6];
        tmp[6] = data_[7];
        for (std::size_t i = 8; i < 16; ++i)
            tmp[i] = data_[i];
        std::memcpy(&id, tmp, 16);
#endif
        return id;
    }

    bool is_null() const
    {
        return *this == uuid();
    }

    bool operator<(const uuid& rhs) const
    {
        return data_ < rhs.data_;
    }

    bool operator==(const uuid& rhs) const
    {
        return data_ == rhs.data_;
    }

    std::size_t size() const
    {
        return data_.size();
    }

    const boost::uint8_t operator[](std::size_t idx) const
    {
        return data_[idx];
    }

    template<class CharT, class Traits, class Allocator>
    std::basic_string<CharT,Traits,Allocator> to_basic_string() const
    {
        std::basic_string<CharT,Traits,Allocator> s;
        s.reserve(36);
        this->to_string_impl<char>(s, false);
        return s;
    }

    std::string to_string() const
    {
        std::string s;
        s.reserve(36);
        this->to_string_impl<char>(s, false);
        return s;
    }

    std::string to_guid_string() const
    {
        std::string s;
        s.reserve(38);
        s += '{';
        this->to_string_impl<char>(s, true);
        s += '}';
        return s;
    }

    std::basic_string<wchar_t> to_wstring() const
    {
        std::basic_string<wchar_t> s;
        s.reserve(36);
        this->to_string_impl<wchar_t>(s, false);
        return s;
    }

    std::basic_string<wchar_t> to_guid_wstring() const
    {
        std::basic_string<wchar_t> s;
        s.reserve(38);
        s += L'{';
        this->to_string_impl<wchar_t>(s, true);
        s += L'}';
        return s;
    }

private:
    boost::array<boost::uint8_t,16> data_;

    template<class CharT>
    void from_string_impl(const CharT* s)
    {
        for (std::size_t i = 0; i < 16; ++i)
        {
            data_[i] = from_hex(s[0], s[1]);
            s += 2;
            if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
            {
                if (*s != static_widen<CharT,'-'>::value)
                    invalid_uuid();
                ++s;
            }
        }
    }

    template<class CharT, class Traits, class Allocator>
    void to_string_impl(
        std::basic_string<CharT,Traits,Allocator>& s, bool is_upper) const
    {
        for (std::size_t i = 0; i < 4; ++i)
            detail::append_hex(s, data_[i], is_upper);
        s += static_widen<CharT,'-'>::value;
        detail::append_hex(s, data_[4], is_upper);
        detail::append_hex(s, data_[5], is_upper);
        s += static_widen<CharT,'-'>::value;
        detail::append_hex(s, data_[6], is_upper);
        detail::append_hex(s, data_[7], is_upper);
        s += static_widen<CharT,'-'>::value;
        detail::append_hex(s, data_[8], is_upper);
        detail::append_hex(s, data_[9], is_upper);
        s += static_widen<CharT,'-'>::value;
        for (std::size_t i = 10; i < 16; ++i)
            detail::append_hex(s, data_[i], is_upper);
    }

    static void invalid_uuid()
    {
        throw std::runtime_error("invalid uuid string");
    }
};

template<class CharT, class Traits>
inline std::basic_ostream<CharT,Traits>&
operator<<(std::basic_ostream<CharT,Traits>& os, const uuid& id)
{
    typedef typename std::basic_ostream<CharT,Traits>::sentry sentry_t;
    sentry_t ok(os);
    if (ok)
        os << id.to_basic_string<CharT,Traits,std::allocator<CharT> >();
    return os;
}

template<class CharT, class Traits>
inline std::basic_istream<CharT,Traits>&
operator>>(std::basic_istream<CharT,Traits>& is, uuid& id)
{
    typedef typename std::basic_istream<CharT,Traits>::sentry sentry_t;
    sentry_t ok(is);
    if (ok)
    {
        CharT c;
        if (!is.get(c))
            return is;

        is.unget();

        if (c == is.widen('{'))
        {
            CharT buf[38+1];
            if (!is.read(buf, 38))
                return is;
            buf[38] = CharT();
            id = uuid(buf);
        }
        else
        {
            CharT buf[36+1];
            if (!is.read(buf, 36))
                return is;
            buf[36] = CharT();
            id = uuid(buf);
        }
    }
    return is;
}

} // End namespace hamigaki.

BOOST_CLASS_IMPLEMENTATION(hamigaki::uuid, boost::serialization::primitive_type)

#endif // HAMIGAKI_UUID_HPP
