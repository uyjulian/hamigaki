//  joliet_id.hpp: Joliet file/directory ID

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP

#include <algorithm>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

inline std::string::size_type
ucs2be_find(const std::string& s, char c, std::string::size_type off = 0)
{
    std::string::size_type size = s.size();

    for (std::string::size_type i = off; i < size; i += 2)
    {
        if ((s[i] == '\0') && (s[i+1] == c))
            return i;
    }

    return std::string::npos;
}

class joliet_id_accessor
{
public:
    typedef std::string::size_type size_type;

    explicit joliet_id_accessor(const std::string& s)
        : str_(s), name_size_(0)
        , ext_offset_(str_.size()), ext_size_(0)
        , ver_offset_(str_.size()), ver_size_(0)

    {
        if (s.size() == 1)
        {
            name_size_ = 1;
            return;
        }

        size_type dot = ucs2be_find(str_, '.');
        if (dot == std::string::npos)
            dot = ucs2be_find(str_, ';');

        if (dot != std::string::npos)
        {
            name_size_ = dot;

            if (str_[dot+1] == '.')
                ext_offset_ = dot + 2;
            else
                ext_offset_ = dot;

            if (ext_offset_ < str_.size())
            {
                size_type sco = ucs2be_find(str_, ';', ext_offset_);
                if (sco != std::string::npos)
                {
                    ext_size_ = sco - ext_offset_;
                    ver_offset_ = sco + 2;
                    ver_size_ = str_.size() - ver_offset_;
                }
                else
                    ext_size_ = str_.size() - ext_offset_;
            }
        }
        else
            name_size_ = str_.size();
    }

    static int name_compare(
        const joliet_id_accessor& lhs, const joliet_id_accessor& rhs)
    {
        if ((lhs.name_size_ == 1) && (rhs.name_size_ != 1))
            return -1;
        else if ((lhs.name_size_ != 1) && (rhs.name_size_ == 1))
            return 1;
        else if ((lhs.name_size_ == 1) && (rhs.name_size_ == 1))
            return lhs.str_.compare(rhs.str_);
        return lhs.str_.compare(0, lhs.name_size_, rhs.str_, 0, rhs.name_size_);
    }

    static int extension_compare(
        const joliet_id_accessor& lhs, const joliet_id_accessor& rhs)
    {
        return lhs.str_.compare(
            lhs.ext_offset_, lhs.ext_size_,
            rhs.str_, rhs.ext_offset_, rhs.ext_size_);
    }

    static int version_compare(
        const joliet_id_accessor& lhs, const joliet_id_accessor& rhs)
    {
        return lhs.str_.compare(
            lhs.ver_offset_, lhs.ver_size_,
            rhs.str_, rhs.ver_offset_, rhs.ver_size_);
    }

private:
    const std::string& str_;
    size_type name_size_;
    size_type ext_offset_;
    size_type ext_size_;
    size_type ver_offset_;
    size_type ver_size_;
};

inline int joliet_name_compare(
    const joliet_id_accessor& la, const joliet_id_accessor& ra)
{
    return joliet_id_accessor::name_compare(la, ra);
}

inline int joliet_extension_compare(
    const joliet_id_accessor& la, const joliet_id_accessor& ra)
{
    return joliet_id_accessor::extension_compare(la, ra);
}

inline int joliet_version_compare(
    const joliet_id_accessor& la, const joliet_id_accessor& ra)
{
    return joliet_id_accessor::version_compare(la, ra);
}

inline int joliet_id_compare(
    const std::string& lhs, const std::string& rhs)
{
    typedef std::string::size_type size_type;

    joliet_id_accessor la(lhs);
    joliet_id_accessor ra(rhs);

    if (int cmp = detail::joliet_name_compare(la, ra))
        return cmp;

    if (int cmp = detail::joliet_extension_compare(la, ra))
        return cmp;

    return detail::joliet_version_compare(la, ra);
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP
