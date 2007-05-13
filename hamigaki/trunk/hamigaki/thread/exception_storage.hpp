// exception_storage.hpp: a storage object for exception's infomation

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/thread for library home page.

#ifndef HAMIGAKI_THREAD_EXCEPTION_STORAGE_HPP
#define HAMIGAKI_THREAD_EXCEPTION_STORAGE_HPP

#include <stdexcept>
#include <string>

namespace hamigaki { namespace thread {

struct null_exception_storage
{
    void clear() const {}
    void store() const {}
    void rethrow() const {}
};

class exception_storage
{
public:
    void clear()
    {
        what_.clear();
    }

    void store()
    {
        try
        {
            throw;
        }
        catch (const std::exception& e)
        {
            try
            {
                what_ = e.what();
            }
            catch (...)
            {
            }
        }
        catch (...)
        {
        }
    }

    void rethrow() const
    {
        if (!what_.empty())
            throw std::runtime_error(what_);
    }

private:
    std::string what_;
};

} } // End namespaces thread, hamigaki.

#endif // HAMIGAKI_THREAD_EXCEPTION_STORAGE_HPP
