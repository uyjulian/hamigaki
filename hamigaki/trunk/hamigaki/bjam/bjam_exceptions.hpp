// bjam_exceptions.hpp: the exception classes for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_BJAM_EXCEPTIONS_HPP
#define HAMIGAKI_BJAM_BJAM_EXCEPTIONS_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <boost/shared_array.hpp>
#include <cstring>
#include <exception>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

class exit_exception : public std::exception
{
public:
    exit_exception(const std::string& msg, int code) : code_(code)
    {
        try
        {
            msg_ptr_.reset(new char[msg.size()+1]);
            std::memcpy(&msg_ptr_[0], msg.c_str(), msg.size()+1);
        }
        catch (...)
        {
            msg_ptr_.reset();
        }
    }

    ~exit_exception() throw() // virtual
    {
    }

    const char* what() const throw() // virtual
    {
        if (const char* p = msg_ptr_.get())
            return p;
        else
            return "terminate by EXIT";
    }

    int code() const
    {
        return code_;
    }

private:
    boost::shared_array<char> msg_ptr_;
    int code_;
};

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_BJAM_EXCEPTIONS_HPP
