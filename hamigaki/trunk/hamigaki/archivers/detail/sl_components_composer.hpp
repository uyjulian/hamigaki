// sl_components_composer.hpp: IEEE P1282 "SL" components composer

// Copyright Takeshi Mouri 2007-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_SL_COMPONENTS_COMPOSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_SL_COMPONENTS_COMPOSER_HPP

#include <hamigaki/archivers/iso/system_use_entry_header.hpp>
#include <hamigaki/binary/binary_io.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class sl_components_composer
{
private:
    static const std::size_t head_size =
        hamigaki::struct_size<iso::system_use_entry_header>::value;

public:
    sl_components_composer()
    {
    }

    void compose(const std::string& s)
    {
        std::string tmp;
        if (s == ".")
            tmp.assign("\x02\x00", 2u);
        else if (s == "..")
            tmp.assign("\x04\x00", 2u);
        else if (s == "/")
            tmp.assign("\x08\x00", 2u);
        else
        {
            std::size_t rest = (0xFFu-head_size-1u) - buffer_.size() - 2u;
            std::size_t pos = 0;
            while (s.size() - pos > rest)
            {
                unsigned char size = static_cast<unsigned char>(rest);
                buffer_.push_back('\x01');
                buffer_.push_back(static_cast<char>(size));
                buffer_.append(s, pos, rest);
                flush(false);
                pos += rest;
            }

            unsigned char size = static_cast<unsigned char>(s.size() - pos);
            tmp.push_back('\0');
            tmp.push_back(static_cast<char>(size));
            tmp.append(s, pos, std::string::npos);
        }

        if (head_size + 1u + buffer_.size() + tmp.size() > 0xFFu)
            flush(false);
        buffer_.append(tmp);
    }

    std::string entry_string()
    {
        if (!buffer_.empty())
            flush(true);

        return entry_;
    }

private:
    std::string buffer_;
    std::string entry_;

    void flush(bool end)
    {
        iso::system_use_entry_header head;
        head.signature[0] = 'S';
        head.signature[1] = 'L';
        head.entry_size =
            static_cast<boost::uint8_t>(head_size + 1u + buffer_.size());
        head.version = 1u;
        hamigaki::binary_write(entry_, head);

        entry_.push_back(end ? '\x00' : '\x01');

        entry_.append(buffer_);
        buffer_.clear();
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_SL_COMPONENTS_COMPOSER_HPP
