//  uuid.hpp: Universally Unique IDentifier class

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_UUID_HPP
#define HAMIGAKI_UUID_HPP

#include <boost/config.hpp>

#include <hamigaki/hex_format.hpp>
#include <boost/detail/endian.hpp>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <cstring>
#include <string>

#if defined(BOOST_WINDOWS)
struct _GUID;
#endif

namespace hamigaki {

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
        from_string_impl<char,'-'>(s);
    }

#if !defined(BOOST_NO_STD_WSTRING)
    explicit uuid(const wchar_t* s)
    {
        if (*s == '{')
        {
            if (std::wcslen(s) != 38)
                invalid_uuid();
            ++s;
        }
        else if (std::wcslen(s) != 36)
            invalid_uuid();
        from_string_impl<wchar_t,'-'>(s);
    }
#endif

#if defined(BOOST_WINDOWS)
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
#endif

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

    std::string to_string() const
    {
        std::string s;
        s.reserve(36);
        this->to_string_impl<char,'-'>(s, false);
        return s;
    }

    std::string to_guid_string() const
    {
        std::string s;
        s.reserve(38);
        s += '{';
        this->to_string_impl<char,'-'>(s, true);
        s += '}';
        return s;
    }

#if !defined(BOOST_NO_STD_WSTRING)
    std::wstring to_wstring() const
    {
        std::wstring s;
        s.reserve(36);
        this->to_string_impl<wchar_t,L'-'>(s, false);
        return s;
    }

    std::wstring to_guid_wstring() const
    {
        std::wstring s;
        s.reserve(38);
        s += L'{';
        this->to_string_impl<wchar_t,L'-'>(s, true);
        s += L'}';
        return s;
    }
#endif

private:
    boost::array<boost::uint8_t,16> data_;

    template<class CharT, CharT Delim>
    void from_string_impl(const CharT* s)
    {
        for (std::size_t i = 0; i < 16; ++i)
        {
            data_[i] = from_hex(s[0], s[1]);
            s += 2;
            if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
            {
                if (*s != Delim)
                    invalid_uuid();
                ++s;
            }
        }
    }

    template<class CharT, CharT Delim>
    void to_string_impl(std::basic_string<CharT>& s, bool is_upper) const
    {
        s += hamigaki::to_hex<CharT>(
            data_[0], data_[1], data_[2], data_[3], is_upper);
        s += Delim;
        s += hamigaki::to_hex<CharT>(data_[4], data_[5], is_upper);
        s += Delim;
        s += hamigaki::to_hex<CharT>(data_[6], data_[7], is_upper);
        s += Delim;
        s += hamigaki::to_hex<CharT>(data_[8], data_[9], is_upper);
        s += Delim;
        for (std::size_t i = 10; i < 16; ++i)
            s += hamigaki::to_hex<CharT>(data_[i], is_upper);
    }

    static void invalid_uuid()
    {
        throw std::runtime_error("invalid uuid string");
    }
};

} // End namespace hamigaki.

#endif // HAMIGAKI_UUID_HPP
