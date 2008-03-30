// registry.hpp: registry wrapper class

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_REGISTRY_HPP
#define HAMIGAKI_DETAIL_WINDOWS_REGISTRY_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

class registry_key : boost::noncopyable
{
public:
    registry_key(
        ::HKEY parent, const std::string& sub_key, ::REGSAM sam)
    {
        long err = ::RegOpenKeyExA(
            parent, sub_key.c_str(), 0, sam, &handle_);
        if (err != ERROR_SUCCESS)
            throw std::runtime_error("cannot open registry key");
    }

    ~registry_key()
    {
        ::RegCloseKey(handle_);
    }

    boost::optional<std::string> sub_key_name(::DWORD index) const
    {
        char buf[256];
        ::DWORD size = sizeof(buf);

        ::FILETIME ft;
        long err = ::RegEnumKeyExA(
            handle_, index, buf, &size, 0, 0, 0, &ft);

        if (err == ERROR_SUCCESS)
        {
            if (buf[size-1] == '\0')
                --size;
            return std::string(buf, size);
        }
        else
        {
            if (err != ERROR_NO_MORE_ITEMS)
                throw std::runtime_error("RegEnumKeyExA failed()");

            return boost::optional<std::string>();
        }
    }

    std::string get_value(const std::string& name) const
    {
        ::DWORD size;
        long err = ::RegQueryInfoKeyA(handle_,
            0, 0, 0, 0, 0, 0, 0, 0, &size, 0, 0);
        if (err != ERROR_SUCCESS)
            throw std::runtime_error("RegQueryInfoKey failed()");

        ::DWORD type = REG_SZ;
        std::string buf;
        buf.resize(size+1);

        err = ::RegQueryValueExA(handle_, name.c_str(), 0,
            &type, reinterpret_cast< ::BYTE*>(&buf[0]), &size);
        if (err != ERROR_SUCCESS)
            throw std::runtime_error("RegQueryValueEx failed()");

        if (buf[size-1] == '\0')
            --size;
        buf.resize(size);
        return buf;
    }

private:
    ::HKEY handle_;
};

class registry_key_iterator
    : public boost::iterator_facade<
        registry_key_iterator,
        const std::string,
        boost::single_pass_traversal_tag
    >
{
    friend class boost::iterator_core_access;

public:
    registry_key_iterator() : ptr_(0), index_(0)
    {
    }

    explicit registry_key_iterator(const registry_key& key)
        : ptr_(&key), index_(0)
    {
        increment();
    }

private:
    const registry_key* ptr_;
    ::DWORD index_;
    std::string name_;

    const std::string& dereference() const
    {
        return name_;
    }

    void increment()
    {
        BOOST_ASSERT(ptr_ != 0);

        if (boost::optional<std::string> next = ptr_->sub_key_name(index_))
        {
            name_ = *next;
            ++index_;
        }
        else
        {
            ptr_ = 0;
            index_ = 0;
        }
    }

    bool equal(const registry_key_iterator& rhs) const
    {
        return (ptr_ == rhs.ptr_) && (index_ == rhs.index_);
    }
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_REGISTRY_HPP
