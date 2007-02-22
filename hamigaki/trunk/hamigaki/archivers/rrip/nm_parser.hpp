//  nm_parser.hpp: IEEE P1282 "NM" System Use Entry parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_RRIP_NM_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_RRIP_NM_PARSER_HPP

#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace rrip {

class nm_parser
{
private:
    static const boost::uint8_t continue_       = 0x01;
    static const boost::uint8_t current         = 0x02;
    static const boost::uint8_t parent          = 0x04;

public:
    nm_parser() : ends_(false), bad_(false)
    {
    }

    template<class Header>
    bool operator()(Header& head, const char* s, std::size_t size)
    {
        using namespace boost::filesystem;

        if (ends_ || (size < 1u))
            bad_ = true;

        if (bad_)
            return true;

        boost::uint8_t flags = static_cast<boost::uint8_t>(*s);
        ++s;
        --size;

        if ((flags & current) != 0)
        {
            const boost::uint8_t mask = continue_ | parent;

            if (((flags & mask) != 0) || !buf_.empty())
                bad_ = true;
            else
            {
                head.path = path(".", no_check);
                ends_ = true;
            }
        }
        else if ((flags & parent) != 0)
        {
            const boost::uint8_t mask = continue_ | current;

            if (((flags & mask) != 0) || !buf_.empty())
                bad_ = true;
            else
            {
                head.path = path("..", no_check);
                ends_ = true;
            }
        }
        else
        {
            buf_.append(s, size);

            ends_ = (flags & continue_) == 0;
            if (ends_)
                head.path = path(buf_, no_check);
        }
        return true;
    }

private:
    bool ends_;
    bool bad_;
    std::string buf_;
};

} } } // End namespaces rrip, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_RRIP_NM_PARSER_HPP
