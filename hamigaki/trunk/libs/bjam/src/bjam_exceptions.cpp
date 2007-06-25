// bjam_exceptions.cpp: the exception classes for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <cstring>

namespace hamigaki { namespace bjam {

struct exception_data::impl
{
    std::string name;
    std::string what;
};

exception_data::exception_data(const char* msg, const std::string& name)
    : msg_(msg)
{
    try
    {
        pimpl_.reset(new impl);
        pimpl_->name = name;

        pimpl_->what = msg;
        pimpl_->what += ": ";
        pimpl_->what += name;
    }
    catch (...)
    {
        pimpl_.reset();
        throw;
    }
}

exception_data::~exception_data()
{
}

const char* exception_data::what() const
{
    if (const impl* ptr = pimpl_.get())
        return ptr->what.c_str();
    else
        return msg_;
}

const char* exception_data::name() const
{
    if (const impl* ptr = pimpl_.get())
        return ptr->name.c_str();
    else
        return "";
}


cannot_open_file::cannot_open_file(const std::string& name)
    : data_("cannot open file", name)
{
}

cannot_open_file::~cannot_open_file() throw()
{
}

const char* cannot_open_file::what() const throw()
{
    return data_.what();
}

const char* cannot_open_file::filename() const
{
    return data_.name();
}


module_not_found::module_not_found(const std::string& name)
    : data_("module not found", name)
{
}

module_not_found::~module_not_found() throw()
{
}

const char* module_not_found::what() const throw()
{
    return data_.what();
}

const char* module_not_found::module() const
{
    return data_.name();
}


rule_not_found::rule_not_found(const std::string& name)
    : data_("rule not found", name)
{
}

rule_not_found::~rule_not_found() throw()
{
}

const char* rule_not_found::what() const throw()
{
    return data_.what();
}

const char* rule_not_found::rule() const
{
    return data_.name();
}


already_defined_class::already_defined_class(const std::string& name)
    : data_("the class is already defined", name)
{
}

already_defined_class::~already_defined_class() throw()
{
}

const char* already_defined_class::what() const throw()
{
    return data_.what();
}

const char* already_defined_class::class_name() const
{
    return data_.name();
}


exit_exception::exit_exception(const std::string& msg, int code)
    : code_(code)
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

exit_exception::~exit_exception() throw() // virtual
{
}

const char* exit_exception::what() const throw() // virtual
{
    if (const char* p = msg_ptr_.get())
        return p;
    else
        return "terminate by EXIT";
}

} } // End namespaces bjam, hamigaki.
