// iso_data_reader.hpp: ISO 9660 extent reader

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_DATA_READER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_DATA_READER_HPP

#include <hamigaki/archivers/detail/iso_directory_reader.hpp>
#include <hamigaki/archivers/iso/file_flags.hpp>
#include <hamigaki/integer/auto_min.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class iso_data_reader : private boost::noncopyable
{
public:
    typedef iso_directory_record directory_record;

    iso_data_reader(Source& src, boost::uint32_t lbn_shift)
        : src_(src), lbn_shift_(lbn_shift), dir_reader_(lbn_shift)
        , index_(0), pos_(0)
    {
        record_.flags = iso::file_flags::directory;
        record_.data_size = 0;
    }

    void select_directory(boost::uint32_t data_pos)
    {
        seek_logical_block(data_pos);
        dir_reader_.read(src_, entries_);
        index_ = 0;
    }

    const std::vector<directory_record>& entries() const
    {
        return entries_;
    }

    directory_record record() const
    {
        return record_;
    }

    std::size_t entry_index() const
    {
        return index_;
    }

    void select_entry(std::size_t n)
    {
        record_ = entries_.at(n);
        index_ = n;
        pos_ = 0;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((record_.flags & iso::file_flags::directory) != 0)
            return -1;

        if (pos_ == 0)
            seek_logical_block(record_.data_pos);

        boost::uint64_t rest = record_.data_size - pos_;
        if (rest == 0)
            return -1;

        std::streamsize amt = auto_min(n, rest);
        iostreams::blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

private:
    Source& src_;
    const boost::uint32_t lbn_shift_;
    iso_directory_reader dir_reader_;
    std::vector<directory_record> entries_;
    std::size_t index_;
    boost::uint64_t pos_;
    directory_record record_;

    void seek_logical_block(boost::uint32_t num)
    {
        boost::uint64_t off = static_cast<boost::uint64_t>(num) << lbn_shift_;

        boost::iostreams::seek(
            src_,
            static_cast<boost::iostreams::stream_offset>(off),
            BOOST_IOS::beg);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_DATA_READER_HPP
