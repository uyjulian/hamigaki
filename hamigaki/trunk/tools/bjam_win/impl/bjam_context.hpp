// bjam_context.hpp: the context information for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_CONTEXT_HPP
#define IMPL_BJAM_CONTEXT_HPP

#include "./bjam_target.hpp"
#include <boost/filesystem/path.hpp>

struct bjam_context
{
    boost::filesystem::path working_directory;
    std::vector<bjam_target> targets;
};

#endif // IMPL_BJAM_CONTEXT_HPP
