// huffman_test.cpp: test case for Huffman coding

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/utility/huffman.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

namespace io_ex = hamigaki::iostreams;
namespace ut = boost::unit_test;

typedef io_ex::huffman_decoder<boost::uint16_t,16> huffman_dec;
typedef io_ex::huffman_code_length_decoder<boost::uint16_t> length_dec;

typedef io_ex::huffman_encoder<boost::uint16_t,16> huffman_enc;
typedef io_ex::huffman<boost::uint16_t> huffman;

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

void huffman_code_check(
    const huffman_enc& table, char c, boost::uint16_t code, std::size_t bits)
{
    if (static_cast<std::size_t>(c) >= table.size())
    {
        BOOST_ERROR("out of huffman_enc");
        return;
    }

    huffman_enc::const_iterator pos = table.begin() + c;

    BOOST_CHECK_EQUAL(pos->code, code);
    BOOST_CHECK_EQUAL(pos->bits, bits);
}

void huffman_encode_test()
{
    huffman huff;

    for (int i = 0 ; i < 2; ++i)
        huff.insert('A');
    for (int i = 0 ; i < 5; ++i)
        huff.insert('B');
    for (int i = 0 ; i < 3; ++i)
        huff.insert('C');
    huff.insert('D');
    huff.insert('E');

    huffman_enc table;
    huff.make_encoder(table);

    huffman_code_check(table, 'A', 6, 3);
    huffman_code_check(table, 'B', 0, 1);
    huffman_code_check(table, 'C', 2, 2);
    huffman_code_check(table, 'D', 14, 4);
    huffman_code_check(table, 'E', 15, 4);
}

void huffman_encode_test2()
{
    huffman huff;

    huff.insert(2);
    huff.insert(3);
    huff.insert(4);
    huff.insert(2);
    huff.insert(4);

    huffman_enc table;
    huff.make_encoder(table);

    huffman_code_check(table, 2, 0, 1);
    huffman_code_check(table, 3, 2, 2);
    huffman_code_check(table, 4, 3, 2);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("Huffman test");
    test->add(BOOST_TEST_CASE(&huffman_test));
    test->add(BOOST_TEST_CASE(&bad_huffman_test));
    test->add(BOOST_TEST_CASE(&bad_huffman_test2));
    test->add(BOOST_TEST_CASE(&bad_huffman_test3));
    test->add(BOOST_TEST_CASE(&huffman_encode_test));
    test->add(BOOST_TEST_CASE(&huffman_encode_test2));
    return test;
}
