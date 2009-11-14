// cpio_svr4_chksum_test.cpp: test case for svr4_chksum formatted cpio

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/cpio_file.hpp>
#include <hamigaki/iostreams/device/tmp_file.hpp>
#include <hamigaki/iostreams/dont_close.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void check_header(const ar::cpio::header& old, const ar::cpio::header& now)
{
    BOOST_CHECK_EQUAL(old.format, now.format);
    BOOST_CHECK_EQUAL(old.path.string(), now.path.string());
    BOOST_CHECK_EQUAL(old.link_path.string(), now.link_path.string());
    BOOST_CHECK_EQUAL(old.modified_time, now.modified_time);
    BOOST_CHECK_EQUAL(old.permissions, now.permissions);
}

void cpio_test()
{
    std::string data("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    ar::cpio::header head;
    head.format = ar::cpio::svr4_chksum;
    head.path = "cpio_test.dat";
    head.modified_time = std::time(0);
    head.file_size = static_cast<boost::uint32_t>(data.size());
    head.permissions = 0100123;

    io_ex::tmp_file archive;
    ar::basic_cpio_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_cpio_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

    BOOST_CHECK(!src.next_entry());
}

void dir_test()
{
    ar::cpio::header head;
    head.format = ar::cpio::svr4_chksum;
    head.path = "dir";
    head.modified_time = std::time(0);
    head.file_size = 0;
    head.permissions = 040123;

    io_ex::tmp_file archive;
    ar::basic_cpio_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_cpio_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test()
{
    ar::cpio::header head;
    head.format = ar::cpio::svr4_chksum;
    head.path = "link";
    head.link_path = "target";
    head.modified_time = std::time(0);
    head.file_size = 0;
    head.permissions = 0120123;

    io_ex::tmp_file archive;
    ar::basic_cpio_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_cpio_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("cpio svr4 chksum test");
    test->add(BOOST_TEST_CASE(&cpio_test));
    test->add(BOOST_TEST_CASE(&dir_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    return test;
}
