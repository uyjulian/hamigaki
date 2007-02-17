//  joliet_reader.hpp: Joliet reader

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_READER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_READER_HPP

#include <boost/config.hpp>
#include <hamigaki/archivers/detail/iso_data_reader.hpp>
#include <hamigaki/archivers/detail/iso_logical_block_number.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <stack>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    #define HAMIGAKI_ARCHIVERS_WINDOWS

    extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
        unsigned, unsigned long, const wchar_t*, int,
        char*, int, const char*, int*);
#endif

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class joliet_reader : private boost::noncopyable
{
private:
    typedef typename iso_data_reader<Source>::directory_record directory_record;

    static const std::size_t logical_sector_size = 2048;

public:
    typedef Source source_type;

    joliet_reader(const Source& src, const iso::volume_descriptor& desc)
        : data_reader_(src, calc_lbn_shift(desc.logical_block_size))
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
            h.path = dir_path_ / joliet_reader::make_path(rec.file_id);
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

    static std::size_t wide_to_narrow(
        char* s, const wchar_t* pwcs, std::size_t n)
    {
#if defined(HAMIGAKI_ARCHIVERS_WINDOWS)
        int res = ::WideCharToMultiByte(
            0, 0, pwcs, -1, s, static_cast<int>(n), 0, 0);
        if (res == 0)
            throw std::runtime_error("failed WideCharToMultiByte()");
        return static_cast<std::size_t>(res - 1);
#else
        std::size_t res = std::wcstombs(s, pwcs, n);
        if (res == static_cast<std::size_t>(-1))
            throw std::runtime_error("failed wcstombs()");
        return res;
#endif
    }

    static boost::filesystem::path make_path(const std::string& data)
    {
        using namespace boost::filesystem;

        const char* s = data.c_str();
        std::size_t n = data.size();

        std::size_t src_size = n/2;
        boost::scoped_array<wchar_t> src(new wchar_t[src_size + 1]);
        for (std::size_t i = 0; i < n; i += 2)
            src[i/2] = hamigaki::decode_int<big,2>(s + i);
        src[src_size] = 0;

        std::size_t size = joliet_reader::wide_to_narrow(0, src.get(), 0);
        boost::scoped_array<char> buf(new char[size+1]);
        joliet_reader::wide_to_narrow(buf.get(), src.get(), size+1);
        return path(std::string(buf.get(), size), no_check);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_READER_HPP
