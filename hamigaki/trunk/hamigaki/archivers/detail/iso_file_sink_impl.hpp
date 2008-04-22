// iso_file_sink_impl.hpp: ISO file sink implementation

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/raw_iso_file_sink_impl.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink, class Path>
class basic_iso_file_sink_impl : private boost::noncopyable
{
private:
    typedef basic_iso_file_sink_impl<Sink,Path> self;
    typedef basic_raw_iso_file_sink_impl<Sink,Path> impl_type;

    static const std::size_t logical_sector_size = 2048;

public:
    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;
    typedef iso::basic_volume_desc<Path> volume_desc;

    explicit basic_iso_file_sink_impl(
            const Sink& sink, const iso::volume_info& info=iso::volume_info(),
            boost::uint32_t max_extent_size = 0)
        : impl_(sink, info), pos_(0), size_(0)
    {
        if (max_extent_size == 0u)
        {
            max_single_extent_size_ = 0xFFFFFFFFull;
            max_multi_extent_size_  = 0x100000000ull - info.logical_block_size;
        }
        else
        {
            if ((max_extent_size & (info.logical_block_size-1u)) != 0)
                throw std::runtime_error("invalid max extent size");

            max_single_extent_size_ = max_extent_size;
            max_multi_extent_size_ = max_extent_size;
        }
    }

    void add_volume_desc(const volume_desc& desc)
    {
        impl_.add_volume_desc(desc);
    }

    void create_entry(const header_type& head)
    {
        pos_ = 0;
        size_ = head.file_size;
        header_ = head;

        if (size_ > max_single_extent_size_)
        {
            header_.flags |= iso::file_flags::multi_extent;
            header_.file_size = max_multi_extent_size_;
            size_ -= max_multi_extent_size_;
        }
        else
        {
            header_.flags &=
                static_cast<boost::uint8_t>(~iso::file_flags::multi_extent);
        }

        impl_.create_entry(header_);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if ((header_.flags & iso::file_flags::multi_extent) != 0)
        {
            std::streamsize total = 0;
            while (total < n)
            {
                std::streamsize amt =
                    hamigaki::auto_min(n-total, header_.file_size - pos_);
                if (amt == 0)
                {
                    if (size_ == 0)
                        break;

                    if (size_ > max_single_extent_size_)
                        size_ -= max_multi_extent_size_;
                    else
                    {
                        header_.file_size = size_;
                        size_ = 0;
                        header_.flags &=
                            static_cast<boost::uint8_t>(
                                ~iso::file_flags::multi_extent);
                    }
                    impl_.close();
                    impl_.create_entry(header_);
                    pos_ = 0;
                    amt = hamigaki::auto_min(n-total, header_.file_size - pos_);
                }

                impl_.write(s+total, amt);
                total += amt;
                pos_ += amt;
            }
            return total;
        }
        else
            return impl_.write(s, n);
    }

    void close()
    {
        if (((header_.flags & iso::file_flags::multi_extent) != 0) &&
            (pos_ != size_) )
        {
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file size mismatch");
        }

        impl_.close();
    }

    void close_archive()
    {
        impl_.close_archive();
    }

private:
    impl_type impl_;
    header_type header_;
    boost::uint64_t pos_;
    boost::uint64_t size_;
    boost::uint64_t max_single_extent_size_;
    boost::uint64_t max_multi_extent_size_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SINK_IMPL_HPP
