//  joliet_id.hpp: Joliet file/directory ID

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP

#include <boost/config.hpp>

#include <hamigaki/endian.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_array.hpp>
#include <cstdlib>
#include <stdexcept>
#include <string>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    #define HAMIGAKI_ARCHIVERS_WINDOWS

    extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
        unsigned, unsigned long, const wchar_t*, int,
        char*, int, const char*, int*);
#endif

namespace hamigaki { namespace archivers { namespace detail {

class joliet_id
{
public:
    joliet_id()
    {
    }

    explicit joliet_id(char c) : data_(1, c)
    {
    }

    explicit joliet_id(const std::string& s) : data_(s)
    {
    }

    int filename_compare(const joliet_id& rhs) const
    {
        std::size_t lhs_size = data_.size();
        std::size_t rhs_size = rhs.data_.size();

        if ((lhs_size == 1) && (rhs_size != 1))
            return -1;
        else if ((lhs_size != 1) && (rhs_size == 1))
            return 1;
        else
            return data_.compare(rhs.data_);
    }

    int version_compare(const joliet_id& rhs) const
    {
        return 0;
    }

    int compare(const joliet_id& rhs) const
    {
        return filename_compare(rhs);
    }

    boost::filesystem::path to_path() const
    {
        using namespace boost::filesystem;

        const char* s = data_.c_str();
        std::size_t n = data_.size();

        std::size_t src_size = n/2;
        boost::scoped_array<wchar_t> src(new wchar_t[src_size + 1]);
        for (std::size_t i = 0; i < n; i += 2)
            src[i/2] = hamigaki::decode_int<big,2>(s + i);
        src[src_size] = 0;

        std::size_t size = wide_to_narrow(0, src.get(), 0);
        boost::scoped_array<char> buf(new char[size+1]);
        wide_to_narrow(buf.get(), src.get(), size+1);
        return path(std::string(buf.get(), size), no_check);
    }

private:
    std::string data_;

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
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP
