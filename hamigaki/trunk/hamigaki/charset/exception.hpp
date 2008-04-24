// exception.hpp: exception classes for Hamigaki.Charset

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#ifndef HAMIGAKI_CHARSET_EXCEPTION_HPP
#define HAMIGAKI_CHARSET_EXCEPTION_HPP

#include <boost/cstdint.hpp>
#include <stdexcept>

namespace hamigaki { namespace charset {

class encoding_error : public std::runtime_error
{
public:
    explicit encoding_error(const std::string& msg)
        : std::runtime_error(msg)
    {
    }

    ~encoding_error() throw() // virtual
    {
    }
};

class invalid_utf8 : public encoding_error
{
public:
    explicit invalid_utf8(char c)
        : encoding_error("invalid UTF-8 character")
        , char_(c)
    {
    }

    ~invalid_utf8() throw() // virtual
    {
    }

    char invalid_char() const
    {
        return char_;
    }

private:
    char char_;
};

class invalid_surrogate_pair : public encoding_error
{
public:
    invalid_surrogate_pair(boost::uint16_t high, boost::uint16_t low)
        : encoding_error("invalid surrogate pair")
        , high_(high), low_(low)
    {
    }

    ~invalid_surrogate_pair() throw() // virtual
    {
    }

    boost::uint16_t high() const
    {
        return high_;
    }

    boost::uint16_t low() const
    {
        return low_;
    }

private:
    boost::uint16_t high_;
    boost::uint16_t low_;
};

class missing_high_surrogate : public encoding_error
{
public:
    explicit missing_high_surrogate(boost::uint16_t low)
        : encoding_error("missing high surrogate")
    {
    }

    ~missing_high_surrogate() throw() // virtual
    {
    }

    boost::uint16_t low() const
    {
        return low_;
    }

private:
    boost::uint16_t low_;
};

class missing_low_surrogate : public encoding_error
{
public:
    explicit missing_low_surrogate(boost::uint16_t high)
        : encoding_error("missing low surrogate")
    {
    }

    ~missing_low_surrogate() throw() // virtual
    {
    }

    boost::uint16_t high() const
    {
        return high_;
    }

private:
    boost::uint16_t high_;
};

class missing_utf16_low_byte : public encoding_error
{
public:
    missing_utf16_low_byte()
        : encoding_error("missing UTF-16 low byte")
    {
    }

    ~missing_utf16_low_byte() throw() // virtual
    {
    }
};

class invalid_ucs4 : public encoding_error
{
public:
    explicit invalid_ucs4(boost::uint32_t u)
        : encoding_error("invalid UCS-4 character")
        , char_(u)
    {
    }

    ~invalid_ucs4() throw() // virtual
    {
    }

    boost::uint32_t invalid_char() const
    {
        return char_;
    }

private:
    boost::uint32_t char_;
};

} } // End namespaces charset, hamigaki.

#endif // HAMIGAKI_CHARSET_EXCEPTION_HPP
