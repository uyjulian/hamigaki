// zip_encryption_keys.hpp: ZIP encryption keys

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ZIP_ENCRYPTION_KEYS_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ZIP_ENCRYPTION_KEYS_HPP

#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class zip_encryption_keys
{
public:
    explicit zip_encryption_keys(const std::string& s)
        : key0_(boost::detail::reflector<32>::reflect(0x12345678))
        , key1_(0x23456789)
        , key2_(boost::detail::reflector<32>::reflect(0x34567890))
    {
        for (std::size_t i = 0; i < s.size(); ++i)
            update_keys(s[i]);
    }

    char decrypt(char c)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        uc = static_cast<unsigned char>(uc ^ decrypt_byte());
        c = static_cast<char>(uc);
        update_keys(c);
        return c;
    }

    char encrypt(char c)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        uc = static_cast<unsigned char>(uc ^ decrypt_byte());
        update_keys(c);
        return static_cast<char>(uc);
    }

private:
    typedef boost::crc_optimal<32,0x04C11DB7,0,0,true,true> crc_type;

    crc_type key0_;
    boost::uint32_t key1_;
    crc_type key2_;

    void update_keys(char c)
    {
        key0_.process_byte(static_cast<unsigned char>(c));
        key1_ += (key0_.checksum() & 0xFF);
        key1_ = key1_ * 134775813 + 1;
        key2_.process_byte(static_cast<unsigned char>(key1_ >> 24));
    }

    unsigned char decrypt_byte()
    {
        boost::uint32_t temp = (key2_.checksum() | 2) & 0xFFFF;
        return static_cast<unsigned char>((temp * (temp ^ 1)) >> 8);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ZIP_ENCRYPTION_KEYS_HPP
