//  iso9660_reader.hpp: ISO 9660 reader

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_READER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_READER_HPP

#include <boost/config.hpp>
#include <hamigaki/archivers/detail/iso_data_reader.hpp>
#include <hamigaki/archivers/detail/iso_logical_block_number.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <stack>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class iso9660_reader : private boost::noncopyable
{
private:
    typedef typename iso_directory_record directory_record;

    static const std::size_t logical_sector_size = 2048;

public:
    typedef Source source_type;

    iso9660_reader(Source& src,
            const iso::volume_info& info, const iso::volume_desc& desc)
        : data_reader_(src, calc_lbn_shift(info.logical_block_size))
    {
        data_reader_.select_directory(desc.root_record.data_pos);
    }

    bool next_entry()
    {
        using namespace boost::filesystem;

        if (header_.is_directory())
        {
            dir_path_ = header_.path;
            stack_.push(data_reader_.entry_index());
            data_reader_.select_directory(data_reader_.record().data_pos);
        }

        std::size_t next_index = data_reader_.entry_index();
        if (next_index)
            ++next_index;
        else
            next_index = 2;

        while (next_index == data_reader_.entries().size())
        {
            if (stack_.empty())
                return false;

            const directory_record& parent = data_reader_.entries().at(1);
            data_reader_.select_directory(parent.data_pos);

            dir_path_ = dir_path_.branch_path();
            next_index = stack_.top() + 1;
            stack_.pop();
        }

        data_reader_.select_entry(next_index);

        const directory_record& rec = data_reader_.record();

        iso::header h;
        if (rec.file_id.size() == 1)
        {
            if (rec.file_id[0] == '\0')
                h.path = dir_path_;
            else
                h.path = dir_path_ / "..";
        }
        else
            h.path = dir_path_ / path(rec.file_id, no_check);
        h.file_size = rec.data_size;
        h.recorded_time = rec.recorded_time;
        h.flags = rec.flags;
        h.system_use = rec.system_use;

        header_ = h;
        return true;
    }

    iso::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return data_reader_.read(s, n);
    }

private:
    iso_data_reader<Source> data_reader_;
    iso::header header_;
    boost::filesystem::path dir_path_;
    std::stack<boost::uint32_t> stack_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_READER_HPP
