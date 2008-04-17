// lzh_h2_wide_test.cpp: test case for LZH with level-2 header (Unicode)

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/lzh_file.hpp>
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

void check_header(const ar::lha::wheader& old, const ar::lha::wheader& now)
{
    BOOST_CHECK(old.level == now.level);
    BOOST_CHECK(old.update_time == now.update_time);
    BOOST_CHECK_EQUAL(old.attributes, now.attributes);
    BOOST_CHECK(old.path == now.path);
    BOOST_CHECK(old.link_path == now.link_path);
    BOOST_CHECK_EQUAL(old.os, now.os);
}

void unicode_test()
{
    ar::lha::wheader head;
    head.level = 2;
    head.update_time = std::time(0);
    head.attributes = ar::msdos::attributes::read_only;
    head.path = L"\x3053\x3093\x306B\x3061\x306F/\x4F60\x597D.txt";
    head.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        fs::wpath
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file,fs::wpath> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void narrow_to_wide_test()
{
    ar::lha::header head;
    head.level = 2;
    head.code_page = 20932;
    head.path =
        "\xA4\xB3\xA4\xF3\xA4\xCB\xA4\xC1\xA4\xCF";

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        fs::path
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file,fs::wpath> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK(src.header().path == L"\u3053\u3093\u306B\u3061\u306F");

    BOOST_CHECK(!src.next_entry());
}

void wide_to_narrow_test()
{
    ar::lha::wheader head;
    head.level = 2;
    head.code_page = 20932;
    head.path = L"\u3053\u3093\u306B\u3061\u306F";

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        fs::wpath
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file,fs::path> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK(src.header().path ==
        "\xA4\xB3\xA4\xF3\xA4\xCB\xA4\xC1\xA4\xCF");

    BOOST_CHECK(!src.next_entry());
}

void symlink_test_aux(const fs::wpath& link, const fs::wpath& target, bool dir)
{
    ar::lha::wheader head;
    head.level = 2;
    head.update_time = std::time(0);
    head.attributes = ar::msdos::attributes::hidden;
    if (dir)
        head.attributes |= ar::msdos::attributes::directory;
    head.path = link;
    head.link_path = target;
    head.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        fs::wpath
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file,fs::wpath> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test()
{
    for (int i = 0; i < 2; ++i)
    {
        bool b = (i != 0);

        symlink_test_aux(L"\x3053", L"\x4F60", b);
        symlink_test_aux(L"\x3053", L"\x4F60/\xC548", b);
        symlink_test_aux(L"\x3053/link", L"\x4F60", b);
        symlink_test_aux(L"\x3053/link", L"\x4F60/\xC548", b);
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("LZH h2 wide test");
    test->add(BOOST_TEST_CASE(&unicode_test));
    test->add(BOOST_TEST_CASE(&narrow_to_wide_test));
    test->add(BOOST_TEST_CASE(&wide_to_narrow_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    return test;
}
