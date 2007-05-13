// asio_source_impl.hpp: declaration of asio_source::impl

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_ASIO_SOURCE_IMPL_HPP
#define HAMIGAKI_AUDIO_ASIO_SOURCE_IMPL_HPP

#include "asio_device_impl.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include <hamigaki/detail/windows/auto_reset_event.hpp>
#include <hamigaki/detail/windows/critical_section.hpp>

namespace hamigaki { namespace audio {

class asio_source::impl : boost::noncopyable
{
public:
    impl(asio_device::impl& dev,
        sample_format_type type, std::size_t buffer_size);

    ~impl();

    std::streamsize read(char* s, std::streamsize n);
    void close();

    sample_format_type sample_format() const
    {
        return type_;
    }

    std::streamsize write(const char* s, std::streamsize n);

private:
    hamigaki::detail::windows::critical_section cs_;
    asio_device::impl* dev_ptr_;
    sample_format_type type_;
    std::vector<char> buffer_;
    boost::ptr_vector<hamigaki::detail::windows::auto_reset_event> events_;
    std::streamsize read_pos_;
    std::streamsize write_pos_;
    bool is_open_;
};

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_ASIO_SOURCE_IMPL_HPP
