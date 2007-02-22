//  parser_map.hpp: IEEE P1282 Rock Ridge parser mapping table

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_RRIP_PARSER_MAP_HPP
#define HAMIGAKI_ARCHIVERS_RRIP_PARSER_MAP_HPP

#include <hamigaki/archivers/rrip/cl_parser.hpp>
#include <hamigaki/archivers/rrip/nm_parser.hpp>
#include <hamigaki/archivers/rrip/pl_parser.hpp>
#include <hamigaki/archivers/rrip/re_parser.hpp>
#include <hamigaki/archivers/susp/signature.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/pair.hpp>

namespace hamigaki { namespace archivers { namespace rrip {

typedef boost::mpl::map<
      boost::mpl::pair<susp::signature<'C','L'>, cl_parser>
    , boost::mpl::pair<susp::signature<'N','M'>, nm_parser>
    , boost::mpl::pair<susp::signature<'P','L'>, pl_parser>
    , boost::mpl::pair<susp::signature<'R','E'>, re_parser>
> parser_map;

} } } // End namespaces rrip, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_RRIP_PARSER_MAP_HPP
