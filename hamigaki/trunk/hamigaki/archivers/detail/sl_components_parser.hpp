// sl_components_parser.hpp: IEEE P1282 "SL" components parser

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_SL_COMPONENTS_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_SL_COMPONENTS_PARSER_HPP

#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class sl_components_parser
{
private:
    static const boost::uint8_t continue_       = 0x01;
    static const boost::uint8_t current         = 0x02;
    static const boost::uint8_t parent          = 0x04;
    static const boost::uint8_t root            = 0x08;

public:
    sl_components_parser() : ends_(false), bad_(false)
    {
    }

    void parse(const char* s, std::size_t size)
    {
        if (ends_ || bad_)
            return;

        if (size < 1)
        {
            bad_ = true;
            return;
        }

        boost::uint8_t flags = static_cast<boost::uint8_t>(*s);

        std::size_t pos = 1;
        while (pos < size)
            pos += parse_components(s+pos, size-pos);

        ends_ = (flags & continue_) == 0;
    }

    bool valid() const
    {
        return !bad_ && !ph_.empty() && leaf_.empty();
    }

    boost::filesystem::path path() const
    {
        return ph_;
    }

private:
    bool ends_;
    bool bad_;
    std::string leaf_;
    boost::filesystem::path ph_;

    std::size_t parse_components(const char* s, std::size_t size)
    {
        if (size < 2)
        {
            bad_ = true;
            return size;
        }

        boost::uint8_t flags = static_cast<boost::uint8_t>(s[0]);
        std::size_t amt = static_cast<boost::uint8_t>(s[1]);

        if (2u + amt > size)
        {
            bad_ = true;
            return size;
        }

        if ((flags & current) != 0)
        {
            const boost::uint8_t mask = continue_ | parent | root;

            if (((flags & mask) != 0) || !leaf_.empty())
            {
                bad_ = true;
                return size;
            }
            ph_ /= ".";
        }
        else if ((flags & parent) != 0)
        {
            const boost::uint8_t mask = continue_ | current | root;

            if (((flags & mask) != 0) || !leaf_.empty())
            {
                bad_ = true;
                return size;
            }
            ph_ /= "..";
        }
        else if ((flags & root) != 0)
        {
            const boost::uint8_t mask = continue_ | current | parent;

            if (((flags & mask) != 0) || !leaf_.empty() || !ph_.empty())
            {
                bad_ = true;
                return size;
            }
            ph_ = "/";
        }
        else
        {
            leaf_.append(s+2, amt);

            if ((flags & continue_) == 0)
            {
                ph_ /= leaf_;
                leaf_.clear();
            }
        }

        return 2+amt;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_SL_COMPONENTS_PARSER_HPP
