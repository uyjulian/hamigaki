//  bjam_context.hpp: the context information for bjam_grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_CONTEXT_HPP
#define IMPL_BJAM_CONTEXT_HPP

#include <boost/filesystem/path.hpp>
#include <string>
#include <vector>

struct bjam_context
{
    boost::filesystem::path working_directory;
    std::vector<std::string> targets;
};

#endif // IMPL_BJAM_CONTEXT_HPP
