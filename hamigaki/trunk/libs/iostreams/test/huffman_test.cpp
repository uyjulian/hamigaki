//  huffman_test.cpp: test case for Huffman coding

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/utility/huffman.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

namespace io_ex = hamigaki::iostreams;
namespace ut = boost::unit_test;

typedef io_ex::huffman_decoder<boost::uint16_t,16> huffman_dec;
typedef io_ex::huffman_code_length_decoder<boost::uint16_t> length_dec;

void huffman_test()
{
    huffman_dec tree;

    length_dec decoder;
    for (int i = 0; i < 32768; ++i)
        decoder.push_back(15);
    decoder.decode(tree);
}

void bad_huffman_test()
{
    huffman_dec tree;

    length_dec decoder;
    decoder.push_back(1);
    decoder.push_back(1);
    decoder.decode(tree);

    decoder.push_back(1);
    BOOST_CHECK_THROW(decoder.decode(tree), std::runtime_error);
}

void bad_huffman_test2()
{
    huffman_dec tree;

    length_dec decoder;
    decoder.push_back(1);
    decoder.push_back(2);
    decoder.push_back(2);
    decoder.decode(tree);

    decoder.push_back(2);
    BOOST_CHECK_THROW(decoder.decode(tree), std::runtime_error);
}

void bad_huffman_test3()
{
    huffman_dec tree;

    length_dec decoder;
    decoder.push_back(1);
    for (int i = 0; i < 16384; ++i)
        decoder.push_back(15);
    decoder.decode(tree);

    decoder.push_back(15);
    BOOST_CHECK_THROW(decoder.decode(tree), std::runtime_error);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("Huffman test");
    test->add(BOOST_TEST_CASE(&huffman_test));
    test->add(BOOST_TEST_CASE(&bad_huffman_test));
    test->add(BOOST_TEST_CASE(&bad_huffman_test2));
    test->add(BOOST_TEST_CASE(&bad_huffman_test3));
    return test;
}
