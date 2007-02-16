//  rock_ridge_file_source_impl.hpp: IEEE P1281 file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/rock_ridge_reader.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_rock_ridge_file_source_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

public:
    explicit basic_rock_ridge_file_source_impl(const Source& src)
    {
        Source s(src);

        iso::volume_descriptor desc;
        basic_rock_ridge_file_source_impl::read_volume_descriptor(s, desc);

        reader_.reset(new rock_ridge_reader<Source>(s, desc));
    }

    bool next_entry()
    {
        return reader_->next_entry();
    }

    iso::header header() const
    {
        return reader_->header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return reader_->read(s, n);
    }

private:
    std::auto_ptr<rock_ridge_reader<Source> > reader_;

    template<class Source>
    static void read_volume_descriptor(
        Source& src, iso::volume_descriptor& desc)
    {
        boost::iostreams::seek(src, logical_sector_size*16, BOOST_IOS::beg);

        char block[logical_sector_size];
        while (true)
        {
            iostreams::blocking_read(src, block, sizeof(block));
            if (block[0] == '\xFF')
                break;

            if (block[0] == '\x01')
            {
                hamigaki::binary_read(block, desc);
                return;
            }
        }

        throw BOOST_IOSTREAMS_FAILURE(
            "ISO 9660 volume descriptor not found");
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_FILE_SOURCE_IMPL_HPP
