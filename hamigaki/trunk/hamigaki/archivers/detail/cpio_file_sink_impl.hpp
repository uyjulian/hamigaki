// cpio_file_sink_impl.hpp: POSIX cpio file sink implementation

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/raw_cpio_file_sink_impl.hpp>
#include <hamigaki/checksum/sum16.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
class basic_cpio_file_sink_impl : private boost::noncopyable
{
public:
    explicit basic_cpio_file_sink_impl(const Sink& sink)
        : raw_(sink)
    {
    }

    void create_entry(const cpio::header& head)
    {
        raw_.create_entry(head);
        sum16_.reset();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = raw_.write(s, n);
        sum16_.process_bytes(s, amt);
        return amt;
    }

    void close()
    {
        raw_.close(sum16_.checksum());
    }

    void close_archive()
    {
        raw_.close_archive();
    }

private:
    basic_raw_cpio_file_sink_impl<Sink> raw_;
    checksum::sum16 sum16_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SINK_IMPL_HPP
