// iso_directory_reader.hpp: ISO 9660 directory extent reader

// Copyright Takeshi Mouri 2007-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_READER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_READER_HPP

#include <hamigaki/archivers/detail/iso_directory_record.hpp>
#include <hamigaki/archivers/iso/ce_system_use_entry_data.hpp>
#include <hamigaki/archivers/iso/directory_record.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

class iso_directory_reader : private boost::noncopyable
{
public:
    typedef iso_directory_record directory_record;

    explicit iso_directory_reader(boost::uint32_t lbn_shift)
        : lbn_shift_(lbn_shift)
    {
    }

    template<class Source>
    void read(Source& src, std::vector<directory_record>& records)
    {
        std::vector<directory_record> tmp;

        const std::size_t bin_size =
            struct_size<iso::directory_record>::value;

        std::size_t block_size = static_cast<std::size_t>(1) << lbn_shift_;
        boost::scoped_array<char> block(new char[block_size]);

        iostreams::blocking_read(src, &block[0], block_size);

        iso::directory_record raw;
        hamigaki::binary_read(&block[0], raw);
        if (raw.record_size < bin_size + 1)
            throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 directory records");

        directory_record self;
        self.data_pos = raw.data_pos;
        self.data_size = raw.data_size;
        self.recorded_time = raw.recorded_time;
        self.flags = raw.flags;
        self.file_id.assign(1, '\x00');
        if (std::size_t su_len = raw.record_size - (bin_size + 1))
            self.system_use.assign(&block[bin_size + 1], su_len);
        tmp.push_back(self);

        const boost::uint32_t lbn_mask =
            static_cast<boost::uint32_t>(block_size - 1);
        boost::uint32_t pos = raw.record_size;
        while (pos < self.data_size)
        {
            boost::uint32_t offset = pos & lbn_mask;
            if (offset == 0)
                iostreams::blocking_read(src, &block[0], block_size);

            if (block[offset] != 0)
            {
                boost::uint32_t next = offset + bin_size;
                if (next > block_size)
                    break;

                hamigaki::binary_read(&block[offset], raw);

                boost::uint32_t id_size = raw.file_id_size;
                if ((id_size & 1) == 0)
                    ++id_size;

                if (bin_size + id_size > raw.record_size)
                    break;

                next += id_size;
                if (next > block_size)
                    break;

                directory_record rec;
                rec.data_pos = raw.data_pos;
                rec.data_size = raw.data_size;
                rec.recorded_time = raw.recorded_time;
                rec.flags = raw.flags;
                rec.file_id.assign(&block[offset+bin_size], raw.file_id_size);
                if (std::size_t su_len = raw.record_size - (bin_size + id_size))
                {
                    rec.system_use.assign(
                        &block[offset+bin_size + id_size], su_len);
                }
                tmp.push_back(rec);

                pos += raw.record_size;
            }
            else
                pos += static_cast<boost::uint32_t>(block_size - offset);
        }

        if (pos != self.data_size)
            throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 directory records");

        for (std::size_t i = 0; i < tmp.size(); ++i)
            this->read_continuation_area(src, tmp[i]);

        records.swap(tmp);
    }

private:
    const boost::uint32_t lbn_shift_;

    template<class Source>
    void read_continuation_area(Source& src, directory_record& rec)
    {
        std::string& su = rec.system_use;
        if (su.empty())
            return;

        const std::size_t head_size =
            hamigaki::struct_size<iso::system_use_entry_header>::value;

        const std::size_t ce_size =
            head_size +
            hamigaki::struct_size<iso::ce_system_use_entry_data>::value;

        std::size_t pos = 0;
        iso::ce_system_use_entry_data ce;
        ce.next_size = 0;
        while (pos + head_size <= su.size())
        {
            iso::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "CE", 2) == 0)
            {
                if ((head.entry_size == ce_size) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    hamigaki::binary_read(su.c_str()+pos+head_size, ce);
                }
            }
            pos += head.entry_size;
        }

        // cut off padding
        su.resize(pos);

        // TODO: support multiple "CE" System Use Entries
        if (ce.next_size != 0)
        {
            boost::uint64_t off =
                static_cast<boost::uint64_t>(ce.next_pos) << lbn_shift_;
            off += ce.next_offset;

            boost::iostreams::seek(
                src,
                static_cast<boost::iostreams::stream_offset>(off),
                BOOST_IOS::beg);

            boost::scoped_array<char> buffer(new char[ce.next_size]);
            iostreams::blocking_read(src, buffer.get(), ce.next_size);
            su.append(buffer.get(), ce.next_size);
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_READER_HPP
