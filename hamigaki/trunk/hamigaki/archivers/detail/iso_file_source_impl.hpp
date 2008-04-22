// iso_file_source_impl.hpp: ISO file source implementation

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/raw_iso_file_source_impl.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source, class Path=boost::filesystem::path>
class basic_iso_file_source_impl : private boost::noncopyable
{
private:
    typedef basic_iso_file_source_impl<Source,Path> self;
    typedef basic_raw_iso_file_source_impl<Source,Path> impl_type;

    static const std::size_t logical_sector_size = 2048;

public:
    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;
    typedef iso::basic_volume_desc<Path> volume_desc;

    explicit basic_iso_file_source_impl(const Source& src)
        : impl_(src), pos_(0), total_pos_(0)
    {
    }

    bool next_entry()
    {
        if ((header_.flags & iso::file_flags::multi_extent) != 0)
        {
            if (total_pos_ < header_.file_size)
                total_pos_ += impl_.header().file_size;

            while (total_pos_ < header_.file_size)
            {
                impl_.next_entry();
                total_pos_ += impl_.header().file_size;
            }
        }

        bool result = impl_.next_entry();
        header_ = impl_.header();
        if ((header_.flags & iso::file_flags::multi_extent) != 0)
            header_.file_size = impl_.total_size();
        pos_ = 0;
        total_pos_ = 0;
        return result;
    }

    header_type header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((header_.flags & iso::file_flags::multi_extent) != 0)
        {
            if (total_pos_ >= header_.file_size)
                return -1;

            std::streamsize total = 0;
            while (total < n)
            {
                std::streamsize amt = impl_.read(s+total, n-total);
                if (amt == -1)
                {
                    total_pos_ += pos_;
                    if (total_pos_ >= header_.file_size)
                        break;
                    impl_.next_entry();
                    pos_ = 0;
                }
                else
                {
                    total += amt;
                    pos_ += amt;
                }
            }
            return total != 0 ? total : static_cast<std::streamsize>(-1);
        }
        else
            return impl_.read(s, n);
    }

    const std::vector<volume_desc>& volume_descs() const
    {
        return impl_.volume_descs();
    }

    void select_volume_desc(std::size_t index, bool use_rrip=true)
    {
        impl_.select_volume_desc(index, use_rrip);
    }

private:
    impl_type impl_;
    header_type header_;
    boost::uint64_t pos_;
    boost::uint64_t total_pos_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP
