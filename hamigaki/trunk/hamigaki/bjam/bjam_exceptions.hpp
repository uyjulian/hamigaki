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
#include <boost/shared_ptr.hpp>
#include <exception>
#include <string>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251 4275)
#endif

namespace hamigaki { namespace bjam {

class HAMIGAKI_BJAM_DECL exception_data
{
public:
    exception_data(const char* msg, const std::string& name);
    ~exception_data();
    const char* what() const;
    const char* name() const;

private:
    struct impl;
    boost::shared_ptr<impl> pimpl_;
    const char* msg_;
};

class HAMIGAKI_BJAM_DECL cannot_open_file : public std::exception
{
public:
    explicit cannot_open_file(const std::string& name);
    ~cannot_open_file() throw(); // virtual
    const char* what() const throw(); // virtual
    const char* filename() const;

private:
    exception_data data_;
};

class HAMIGAKI_BJAM_DECL module_not_found : public std::exception
{
public:
    explicit module_not_found(const std::string& name);
    ~module_not_found() throw(); // virtual
    const char* what() const throw(); // virtual
    const char* module() const;

private:
    exception_data data_;
};

class HAMIGAKI_BJAM_DECL rule_not_found : public std::exception
{
public:
    explicit rule_not_found(const std::string& name);
    ~rule_not_found() throw(); // virtual
    const char* what() const throw(); // virtual
    const char* rule() const;

private:
    exception_data data_;
};

class HAMIGAKI_BJAM_DECL already_defined_class : public std::exception
{
public:
    explicit already_defined_class(const std::string& name);
    ~already_defined_class() throw(); // virtual
    const char* what() const throw(); // virtual
    const char* class_name() const;

private:
    exception_data data_;
};

class HAMIGAKI_BJAM_DECL exit_exception : public std::exception
{
public:
    exit_exception(const std::string& msg, int code);
    ~exit_exception() throw(); // virtual
    const char* what() const throw(); // virtual

    int code() const
    {
        return code_;
    }

private:
    boost::shared_array<char> msg_ptr_;
    int code_;
};

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_BJAM_EXCEPTIONS_HPP
