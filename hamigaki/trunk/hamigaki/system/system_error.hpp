// system_error.hpp: operating system error class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef HAMIGAKI_SYSTEM_SYSTEM_ERROR_HPP
#define HAMIGAKI_SYSTEM_SYSTEM_ERROR_HPP

#include <exception>
#include <string>

namespace hamigaki { namespace system {

template<class Traits>
class system_error : public std::exception
{
public:
    typedef Traits traits_type;
    typedef typename Traits::value_type value_type;

    system_error() : code_(0), msg_(0)
    {
    }

    system_error(value_type code, const char* msg)
        : code_(code), msg_(msg)
    {
    }

    system_error(const system_error& e) : code_(e.code_), msg_(e.msg_)
    {
    }

    ~system_error() throw() // virtual
    {
    }

    system_error& operator=(const system_error& e)
    {
        code_ = e.code_;
        msg_ = e.msg_;
        what_.clear();
        return *this;
    }

    const char* what() const throw() // virtual
    {
        if (what_.empty())
        {
            try
            {
                std::string tmp;
                if (msg_)
                    tmp = msg_;
                if (!tmp.empty())
                    tmp += ": ";
                tmp += traits_type::message(code_);
                what_.swap(tmp);
            }
            catch (...)
            {
                return msg_;
            }
        }
        return what_.c_str();
    }

    value_type code() const
    {
        return code_;
    }

private:
    value_type code_;
    const char* msg_;
    mutable std::string what_;
};

} } // End namespaces system, hamigaki.

#endif // HAMIGAKI_SYSTEM_SYSTEM_ERROR_HPP
