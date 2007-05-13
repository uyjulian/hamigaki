// iso_file_reader.hpp: ISO image file reader

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_READER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_READER_HPP

#include <hamigaki/archivers/detail/iso_directory_parser.hpp>
#include <hamigaki/archivers/detail/iso_directory_reader.hpp>
#include <hamigaki/archivers/detail/iso_logical_block_number.hpp>
#include <hamigaki/archivers/iso/file_flags.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/dec_format.hpp>
#include <memory>
#include <stack>

namespace hamigaki { namespace archivers { namespace detail {

inline void parse_iso_file_version(iso::header& h)
{
    using namespace boost::filesystem;

    const std::string id = h.path.string();
    std::size_t delim = id.rfind(';');
    if (delim == std::string::npos)
        return;

    std::size_t size = id.size();
    if (size - (delim+1) > (sizeof("32767")-1))
        return;

    for (std::size_t i = delim+1; i < size; ++i)
    {
        char c = id[i];
        if ((c < '0') || (c > '9'))
            return;
    }

    const char* s = id.c_str();
    const char* beg = s + delim + 1;
    const char* end = s + size;

    boost::uint32_t ver = hamigaki::from_dec<boost::uint32_t>(beg, end);
    if (ver > 32767u)
        return;

    h.path = path(id.substr(0, delim), no_check);
    h.version = static_cast<boost::uint16_t>(ver);
}

template<class Source>
class iso_file_reader : private boost::noncopyable
{
private:
    typedef iso_directory_record directory_record;

public:
    typedef Source source_type;

    iso_file_reader(Source& src, std::auto_ptr<iso_directory_parser>& parser,
            const iso::volume_info& info, const iso::volume_desc& desc)
        : src_(src), lbn_shift_(calc_lbn_shift(info.logical_block_size))
        , parser_(parser), pos_(0)
    {
        select_directory(desc.root_record.data_pos);

        index_ = 1;
    }

    bool next_entry()
    {
        if (header_.is_directory())
        {
            dir_path_ = header_.path;
            stack_.push(index_);
            select_directory(records_[index_].data_pos);
        }
        else
            ++index_;

        while (index_ == records_.size())
        {
            if (stack_.empty())
                return false;

            const directory_record& parent = records_.at(1);
            select_directory(parent.data_pos);

            dir_path_ = dir_path_.branch_path();
            index_ = stack_.top() + 1;
            stack_.pop();
        }

        header_ = parser_->make_header(records_[index_]);
        detail::parse_iso_file_version(header_);
        header_.path = dir_path_ / header_.path;
        if (header_.is_directory())
            header_.file_size = 0;
        pos_ = 0;
        return true;
    }

    iso::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if (!header_.is_regular() && !header_.is_associated())
            return -1;

        if (pos_ == 0)
            seek_logical_block(header_.data_pos);

        boost::uint64_t rest = header_.file_size - pos_;
        if (rest == 0)
            return -1;

        std::streamsize amt = auto_min(n, rest);
        iostreams::blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

    boost::uint64_t total_size() const
    {
        boost::uint64_t total = header_.file_size;
        std::size_t i = index_;
        while ((records_[i].flags & iso::file_flags::multi_extent) != 0)
            total += records_.at(++i).data_size;
        return total;
    }

private:
    Source& src_;
    const boost::uint32_t lbn_shift_;
    std::auto_ptr<iso_directory_parser> parser_;
    std::vector<directory_record> records_;
    iso::header header_;
    boost::filesystem::path dir_path_;
    std::stack<boost::uint32_t> stack_;
    std::size_t index_;
    boost::uint64_t pos_;

    void select_directory(boost::uint32_t data_pos)
    {
        seek_logical_block(data_pos);

        iso_directory_reader dir_reader(lbn_shift_);
        dir_reader.read(src_, records_);
        parser_->fix_records(records_);

        index_ = 2;
    }

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

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_READER_HPP
