// zip_replace_test.cpp: test case for ZIP replace

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/raw_zip_file.hpp>
#include <hamigaki/archivers/zip_file.hpp>
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

void check_header(const ar::zip::header& old, const ar::zip::header& now)
{
    BOOST_CHECK_EQUAL(old.path.string(), now.path.string());
    BOOST_CHECK_EQUAL(old.link_path.string(), now.link_path.string());
    BOOST_CHECK_EQUAL(old.encrypted, now.encrypted);
    BOOST_CHECK(old.method == now.method);
    BOOST_CHECK((now.update_time - old.update_time) >= 0);
    BOOST_CHECK((now.update_time - old.update_time) <= 1);
    BOOST_CHECK_EQUAL(old.attributes, now.attributes);
    BOOST_CHECK_EQUAL(old.permissions, now.permissions);
    BOOST_CHECK_EQUAL(old.comment, now.comment);
}

void replace_test_aux(bool encrypted)
{
    std::string data_a(32u, 'a');
    std::string data_a2(33u, 'A');
    std::string data_b(16u, 'b');

    ar::zip::header head_a;
    head_a.path = "a.dat";
    head_a.encrypted = encrypted;
    head_a.update_time = std::time(0);
    head_a.file_size = data_a.size();

    ar::zip::header head_a2;
    head_a2.path = "A.dat";
    head_a2.encrypted = encrypted;
    head_a2.update_time = std::time(0)+1;
    head_a2.file_size = data_a2.size();

    ar::zip::header head_b;
    head_b.path = "b.dat";
    head_b.encrypted = encrypted;
    head_b.update_time = std::time(0);
    head_b.file_size = data_b.size();

    io_ex::tmp_file archive;
    ar::basic_zip_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));
    sink.password("password");

    sink.create_entry(head_a);
    io_ex::blocking_write(sink, &data_a[0], data_a.size());
    sink.close();

    sink.create_entry(head_b);
    io_ex::blocking_write(sink, &data_b[0], data_b.size());
    sink.close();

    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_raw_zip_file_source<io_ex::tmp_file> src(archive);

    io_ex::tmp_file archive2;
    ar::basic_zip_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink2(io_ex::dont_close(archive2));
    sink2.password("password");

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

    ar::basic_zip_file_source<io_ex::tmp_file> src2(archive2);
    src2.password("password");

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

    BOOST_CHECK(!src.next_entry());
}

void replace_test()
{
    replace_test_aux(false);
}

void crypt_test()
{
    replace_test_aux(true);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("ZIP replace test");
    test->add(BOOST_TEST_CASE(&replace_test));
    test->add(BOOST_TEST_CASE(&crypt_test));
    return test;
}
