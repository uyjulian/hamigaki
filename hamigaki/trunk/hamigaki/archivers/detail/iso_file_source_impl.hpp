//  iso_file_source_impl.hpp: ISO file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/joliet_reader.hpp>
#include <hamigaki/archivers/detail/rock_ridge_reader.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

class iso_file_reader_base
{
public:
    virtual ~iso_file_reader_base(){};

    bool next_entry()
    {
        return do_next_entry();
    }

    iso::header header() const
    {
        return do_header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return do_read(s, n);
    }

private:
    virtual bool do_next_entry() = 0;
    virtual iso::header do_header() const = 0;
    virtual std::streamsize do_read(char* s, std::streamsize n) = 0;
};

template<class Impl>
class iso_file_reader : public iso_file_reader_base
{
public:
    typedef typename Impl::source_type source_type;

    iso_file_reader(const source_type& src, const iso::volume_descriptor& desc)
        : impl_(src, desc)
    {
    }

private:
    Impl impl_;

    bool do_next_entry() // virtual
    {
        return impl_.next_entry();
    }

    iso::header do_header() const // virtual
    {
        return impl_.header();
    }

    std::streamsize do_read(char* s, std::streamsize n) // virtual
    {
        return impl_.read(s, n);
    }
};

template<class Source>
class basic_iso_file_source_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

public:
    explicit basic_iso_file_source_impl(const Source& src) : src_(src)
    {
        iso::volume_descriptor volume_desc;

        boost::iostreams::seek(src_, logical_sector_size*16, BOOST_IOS::beg);

        char block[logical_sector_size];
        iso::volume_descriptor desc;
        while (true)
        {
            iostreams::blocking_read(src_, block, sizeof(block));
            if (block[0] == '\xFF')
                break;

            if ((block[0] == '\x01') || (block[0] == '\x02'))
            {
                hamigaki::binary_read(block, desc);
                volume_descs_.push_back(desc);
            }
        }

        if (volume_descs_.empty())
        {
            throw BOOST_IOSTREAMS_FAILURE(
                "ISO 9660 volume descriptor not found");
        }
    }

    bool next_entry()
    {
        if (!reader_.get())
            select_volume_descriptor(0);

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

    const std::vector<iso::volume_descriptor>& volume_descriptors() const
    {
        return volume_descs_;
    }

    void select_volume_descriptor(std::size_t index)
    {
        const iso::volume_descriptor& desc = volume_descs_.at(index);
        if (desc.is_joliet())
        {
            typedef iso_file_reader<joliet_reader<Source> > reader_type;
            reader_.reset(new reader_type(src_, desc));
        }
        else
        {
            typedef iso_file_reader<rock_ridge_reader<Source> > reader_type;
            reader_.reset(new reader_type(src_, desc));
        }
    }

private:
    Source src_;
    std::vector<iso::volume_descriptor> volume_descs_;
    std::auto_ptr<iso_file_reader_base> reader_;

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

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP
