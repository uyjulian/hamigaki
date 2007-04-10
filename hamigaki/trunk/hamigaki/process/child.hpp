//  child.hpp: child process

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_CHILD_HPP
#define HAMIGAKI_PROCESS_CHILD_HPP

#include <hamigaki/process/detail/config.hpp>
#include <hamigaki/process/context.hpp>
#include <hamigaki/process/pipe_device.hpp>
#include <hamigaki/process/status.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

namespace hamigaki { namespace process {

class HAMIGAKI_PROCESS_DECL child
{
public:
    child(
        const std::string& path, const std::vector<std::string>& args,
        const context& ipc = context()
    );

    explicit child(const std::string& path, const context& ctx = context());

    status wait();
    void terminate();

    pipe_sink& stdin_sink();
    pipe_source& stdout_source();
    pipe_source& stderr_source();

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

#endif // HAMIGAKI_PROCESS_CHILD_HPP
