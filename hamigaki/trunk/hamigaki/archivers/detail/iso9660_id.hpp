//  iso9660_id.hpp: ISO 9660 file/directory ID

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_ID_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_ID_HPP

#include <algorithm>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

class iso9660_id_accessor
{
public:
    typedef std::string::size_type size_type;

    explicit iso9660_id_accessor(const std::string& s)
        : str_(s), name_size_(0)
        , ext_offset_(str_.size()), ext_size_(0)
        , ver_offset_(str_.size()), ver_size_(0)

    {
        size_type dot = str_.find_first_of(".;");
        if (dot != std::string::npos)
        {
            name_size_ = dot;

            if (str_[dot] == '.')
                ext_offset_ = dot + 1;
            else
                ext_offset_ = dot;

            if (ext_offset_ < str_.size())
            {
                size_type sco = str_.find(';', ext_offset_);
                if (sco != std::string::npos)
                {
                    ext_size_ = sco - ext_offset_;
                    ver_offset_ = sco + 1;
                    ver_size_ = str_.size() - ver_offset_;
                }
                else
                    ext_size_ = str_.size() - ext_offset_;
            }
        }
        else
            name_size_ = str_.size();
    }

    size_type name_size() const
    {
        return name_size_;
    }

    char name(size_type i) const
    {
        if (i < name_size_)
            return str_[i];
        else
            return ' ';
    }

    size_type extension_offset() const
    {
        return ext_offset_;
    }

    size_type extension_size() const
    {
        return ext_size_;
    }

    char extension(size_type i) const
    {
        if (i < ext_size_)
            return str_[ext_offset_+i];
        else
            return ' ';
    }

    size_type version_offset() const
    {
        return ver_offset_;
    }

    size_type version_size() const
    {
        return ver_size_;
    }

    char version(size_type i, size_type max_size) const
    {
        size_type off = max_size - ver_size_;
        if (i < off)
            return '0';
        else
            return str_[ver_offset_+(i-off)];
    }

private:
    const std::string& str_;
    size_type name_size_;
    size_type ext_offset_;
    size_type ext_size_;
    size_type ver_offset_;
    size_type ver_size_;
};

inline int iso9660_name_compare(
    const iso9660_id_accessor& la, const iso9660_id_accessor& ra)
{
    typedef std::string::size_type size_type;

    size_type name_size = (std::max)(la.name_size(), ra.name_size());
    for (size_type i = 0; i < name_size; ++i)
    {
        unsigned char lc = static_cast<unsigned char>(la.name(i));
        unsigned char rc = static_cast<unsigned char>(ra.name(i));

        if (lc < rc)
            return -1;
        else if (lc > rc)
            return 1;
    }

    return 0;
}

inline int iso9660_extension_compare(
    const iso9660_id_accessor& la, const iso9660_id_accessor& ra)
{
    typedef std::string::size_type size_type;

    size_type extension_size =
        (std::max)(la.extension_size(), ra.extension_size());
    for (size_type i = 0; i < extension_size; ++i)
    {
        unsigned char lc = static_cast<unsigned char>(la.extension(i));
        unsigned char rc = static_cast<unsigned char>(ra.extension(i));

        if (lc < rc)
            return -1;
        else if (lc > rc)
            return 1;
    }

    return 0;
}

inline int iso9660_version_compare(
    const iso9660_id_accessor& la, const iso9660_id_accessor& ra)
{
    typedef std::string::size_type size_type;

    size_type version_size =
        (std::max)(la.version_size(), ra.version_size());
    for (size_type i = 0; i < version_size; ++i)
    {
        unsigned char lc =
            static_cast<unsigned char>(la.version(i, version_size));
        unsigned char rc =
            static_cast<unsigned char>(ra.version(i, version_size));

        if (lc < rc)
            return -1;
        else if (lc > rc)
            return 1;
    }

    return 0;
}

inline int iso9660_id_compare(
    const std::string& lhs, const std::string& rhs)
{
    typedef std::string::size_type size_type;

    iso9660_id_accessor la(lhs);
    iso9660_id_accessor ra(rhs);

    if (int cmp = detail::iso9660_name_compare(la, ra))
        return cmp;

    if (int cmp = detail::iso9660_extension_compare(la, ra))
        return cmp;

    return detail::iso9660_version_compare(la, ra);
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_ID_HPP
