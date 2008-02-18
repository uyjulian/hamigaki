// hello.cpp: Hello library

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define HELLO_SOURCE
#include "hello.hpp"
#include <iostream>

HELLO_DECL void hello()
{
    std::cout << "Hello, world!" << std::endl;
}
