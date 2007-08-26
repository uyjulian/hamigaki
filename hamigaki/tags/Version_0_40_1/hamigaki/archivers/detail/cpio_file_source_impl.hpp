// cpio_file_source_impl.hpp: POSIX cpio file source implementation

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/raw_cpio_file_source_impl.hpp>
#include <hamigaki/checksum/sum16.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_cpio_file_source_impl : private boost::noncopyable
{
public:
    explicit basic_cpio_file_source_impl(const Source& src)
        : raw_(src)
    {
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        sum16_.reset();
        head_ = raw_.header();
        return true;
    }

    cpio::header header() const
    {
        return head_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = raw_.read(s, n);
        if (head_.format == cpio::svr4_chksum)
        {
            if (amt == n)
                sum16_.process_bytes(s, amt);
            else if (sum16_.checksum() != *head_.checksum)
                throw BOOST_IOSTREAMS_FAILURE("cpio checksum missmatch");
        }
        return amt;
    }

private:
    basic_raw_cpio_file_source_impl<Source> raw_;
    cpio::header head_;
    checksum::sum16 sum16_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SOURCE_IMPL_HPP
