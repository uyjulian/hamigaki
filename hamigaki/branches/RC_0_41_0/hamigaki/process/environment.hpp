// environment.hpp: environment variables

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_ENVIRONMENT_HPP
#define HAMIGAKI_PROCESS_ENVIRONMENT_HPP

#include <hamigaki/process/detail/config.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

namespace hamigaki { namespace process {

class HAMIGAKI_PROCESS_DECL environment
{
public:
    environment();
    ~environment();

    void clear();
    void set(const std::string& name, const std::string& value);
    void unset(const std::string& name);
    const char* get(const std::string& name) const;
    std::size_t size() const;

    char* data();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces process, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_PROCESS_ENVIRONMENT_HPP
