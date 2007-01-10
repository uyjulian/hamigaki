//  iso9660_id.hpp: ISO 9660 file/directory ID

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_ID_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_ID_HPP

#include <boost/filesystem/path.hpp>
#include <boost/operators.hpp>
#include <algorithm>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

class iso9660_id : boost::totally_ordered<iso9660_id>
{
public:
    iso9660_id()
        : base_size_(0)
        , ext_offset_(1), ext_size_(0)
        , ver_offset_(1), ver_size_(0)
    {
    }

    explicit iso9660_id(char c)
        : data_(1, c), base_size_(1)
        , ext_offset_(1), ext_size_(0)
        , ver_offset_(1), ver_size_(0)
    {
    }

    explicit iso9660_id(const std::string& s)
        : data_(s), base_size_(0)
        , ext_offset_(data_.size()), ext_size_(0)
        , ver_offset_(data_.size()), ver_size_(0)
    {
        std::size_t dot = data_.find_first_of(".;");
        if (dot != std::string::npos)
        {
            base_size_ = dot;

            if (data_[dot] == '.')
                ext_offset_ = dot + 1;
            else
                ext_offset_ = dot;

            if (ext_offset_ < data_.size())
            {
                std::size_t sco = data_.find(';', ext_offset_);
                if (sco != std::string::npos)
                {
                    ext_size_ = sco - ext_offset_;
                    ver_offset_ = sco + 1;
                    ver_size_ = data_.size() - ver_offset_;
                }
                else
                    ext_size_ = data_.size() - ext_offset_;
            }
        }
        else
            base_size_ = data_.size();
    }

    int filename_compare(const iso9660_id& rhs) const
    {
        if (int cmp = basename_compare(rhs))
            return cmp;
        else
            return extension_compare(rhs);
    }

    int version_compare(const iso9660_id& rhs) const
    {
        std::size_t size = (std::max)(ver_size_, rhs.ver_size_);
        for (std::size_t i = 0; i < size; ++i)
        {
            unsigned char lc = static_cast<unsigned char>(version(i, size));
            unsigned char rc = static_cast<unsigned char>(rhs.version(i, size));

            if (lc < rc)
                return -1;
            else if (lc > rc)
                return 1;
        }

        if (ver_size_ < rhs.ver_size_)
            return -1;
        else if (ver_size_ > rhs.ver_size_)
            return 1;
        else
            return 0;
    }

    int compare(const iso9660_id& rhs) const
    {
        if (int cmp = filename_compare(rhs))
            return cmp;
        else
            return version_compare(rhs);
    }

    bool operator==(const iso9660_id& rhs) const
    {
        return compare(rhs) == 0;
    }

    bool operator<(const iso9660_id& rhs) const
    {
        return compare(rhs) < 0;
    }

    boost::filesystem::path to_path() const
    {
        return boost::filesystem::path(data_, boost::filesystem::no_check);
    }

private:
    std::string data_;
    std::size_t base_size_;
    std::size_t ext_offset_;
    std::size_t ext_size_;
    std::size_t ver_offset_;
    std::size_t ver_size_;

    char basename(std::size_t i) const
    {
        if (i < base_size_)
            return data_[i];
        else
            return ' ';
    }

    char extension(std::size_t i) const
    {
        if (i < ext_size_)
            return data_[ext_offset_+i];
        else
            return ' ';
    }

    char version(std::size_t i, std::size_t max_size) const
    {
        std::size_t off = max_size - ver_size_;
        if (i < off)
            return '0';
        else
            return data_[ver_offset_+(i-off)];
    }

    int basename_compare(const iso9660_id& rhs) const
    {
        std::size_t size = (std::max)(base_size_, rhs.base_size_);
        for (std::size_t i = 0; i < size; ++i)
        {
            unsigned char lc = static_cast<unsigned char>(basename(i));
            unsigned char rc = static_cast<unsigned char>(rhs.basename(i));

            if (lc < rc)
                return -1;
            else if (lc > rc)
                return 1;
        }

        if (base_size_ < rhs.base_size_)
            return -1;
        else if (base_size_ > rhs.base_size_)
            return 1;
        else
            return 0;
    }

    int extension_compare(const iso9660_id& rhs) const
    {
        std::size_t size = (std::max)(ext_size_, rhs.ext_size_);
        for (std::size_t i = 0; i < size; ++i)
        {
            unsigned char lc = static_cast<unsigned char>(extension(i));
            unsigned char rc = static_cast<unsigned char>(rhs.extension(i));

            if (lc < rc)
                return -1;
            else if (lc > rc)
                return 1;
        }

        if (ext_size_ < rhs.ext_size_)
            return -1;
        else if (ext_size_ > rhs.ext_size_)
            return 1;
        else
            return 0;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_ID_HPP
