// lzh_replace_test.cpp: test case for LZH replace

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/lzh_file.hpp>
#include <hamigaki/archivers/raw_lzh_file.hpp>
#include <hamigaki/iostreams/device/tmp_file.hpp>
#include <hamigaki/iostreams/dont_close.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace ar = hamigaki::archivers;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void check_header(const ar::lha::header& old, const ar::lha::header& now)
{
    BOOST_CHECK(old.level == now.level);
    BOOST_CHECK(old.update_time == now.update_time);
    BOOST_CHECK_EQUAL(old.attributes, now.attributes);
    BOOST_CHECK_EQUAL(old.path.string(), now.path.string());
    BOOST_CHECK_EQUAL(old.link_path.string(), now.link_path.string());
    BOOST_CHECK_EQUAL(old.os, now.os);
}

void replace_test()
{
    std::string data_a(32u, 'a');
    std::string data_a2(33u, 'A');
    std::string data_b(16u, 'b');

    ar::lha::header head_a;
    head_a.update_time = std::time(0);
    head_a.path = "a.dat";
    head_a.os = 'M';

    ar::lha::header head_a2;
    head_a2.update_time = std::time(0)+1;
    head_a2.path = "A.dat";
    head_a2.os = 'M';

    ar::lha::header head_b;
    head_b.update_time = std::time(0);
    head_b.path = "b.dat";
    head_b.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head_a);
    io_ex::blocking_write(sink, &data_a[0], data_a.size());
    sink.close();

    sink.create_entry(head_b);
    io_ex::blocking_write(sink, &data_b[0], data_b.size());
    sink.close();

    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_raw_lzh_file_source<io_ex::tmp_file> src(archive);

    io_ex::tmp_file archive2;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink2(io_ex::dont_close(archive2));

    BOOST_CHECK(src.next_entry());
    BOOST_CHECK_EQUAL(head_a.path.string(), src.header().path.string());

    sink2.create_entry(head_a2);
    io_ex::blocking_write(sink2, &data_a2[0], data_a2.size());
    sink2.close();

    BOOST_CHECK(src.next_entry());
    BOOST_CHECK_EQUAL(head_b.path.string(), src.header().path.string());

    sink2.create_entry(src.header());
    io::copy(src, sink2);

    BOOST_CHECK(!src.next_entry());

    sink2.close_archive();

    io::seek(archive2, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file> src2(archive2);

    BOOST_CHECK(src2.next_entry());
    check_header(head_a2, src2.header());

    std::string buf;
    io::copy(src2, io::back_inserter(buf));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data_a2.begin(), data_a2.end(), buf.begin(), buf.end()
    );

    BOOST_CHECK(src2.next_entry());
    check_header(head_b, src2.header());

    buf.clear();
    io::copy(src2, io::back_inserter(buf));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data_b.begin(), data_b.end(), buf.begin(), buf.end()
    );

    BOOST_CHECK(!src2.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("LZH replace test");
    test->add(BOOST_TEST_CASE(&replace_test));
    return test;
}
