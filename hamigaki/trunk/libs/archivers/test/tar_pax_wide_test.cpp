// tar_pax_wide_test.cpp: test case for pax formatted tar (Unicode)

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/tar_file.hpp>
#include <hamigaki/iostreams/device/tmp_file.hpp>
#include <hamigaki/iostreams/dont_close.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void check_header(const ar::tar::wheader& old, const ar::tar::wheader& now)
{
    BOOST_CHECK_EQUAL(old.format, now.format);
    BOOST_CHECK_EQUAL(old.type_flag, now.type_flag);
    BOOST_CHECK(old.path == now.path);
    BOOST_CHECK(old.link_path == now.link_path);
    BOOST_CHECK(old.modified_time == now.modified_time);
    BOOST_CHECK(old.access_time == now.access_time);
    BOOST_CHECK(old.change_time == now.change_time);
    BOOST_CHECK_EQUAL(old.permissions, now.permissions);
    BOOST_CHECK(old.comment == now.comment);
    BOOST_CHECK(old.charset == now.charset);
}

void unicode_test()
{
    ar::tar::wheader head;
    head.format = ar::tar::pax;
    head.type_flag = ar::tar::type_flag::regular;
    head.path = L"\x4F60\x597D.txt";
    head.modified_time = fs_ex::timestamp(std::time(0), 123456789);
    head.access_time = fs_ex::timestamp(1, 123456789);
    head.change_time = fs_ex::timestamp(12345, 0);
    head.permissions = 0123;
    head.comment = L"\xC548\xB155\xD558\xC2ED\xB2C8\xAE4C";
    head.charset = L"BINARY";

    io_ex::tmp_file archive;
    ar::basic_tar_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>, fs::wpath
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_tar_file_source<io_ex::tmp_file,fs::wpath> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test()
{
    ar::tar::wheader head;
    head.format = ar::tar::pax;
    head.type_flag = ar::tar::type_flag::symlink;
    head.path = L"\x4F60\x597D.txt";
    head.link_path = L"\xC548\xB155\xD558\xC2ED\xB2C8\xAE4C.txt";
    head.modified_time = fs_ex::timestamp(std::time(0), 123456789);

    io_ex::tmp_file archive;
    ar::basic_tar_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>, fs::wpath
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_tar_file_source<io_ex::tmp_file,fs::wpath> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("tar pax wide test");
    test->add(BOOST_TEST_CASE(&unicode_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    return test;
}
