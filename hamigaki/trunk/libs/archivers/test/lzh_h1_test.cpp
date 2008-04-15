// lzh_h1_test.cpp: test case for LZH with level-1 header

// Copyright Takeshi Mouri 2006-2008.
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

void check_header(const ar::lha::header& old, const ar::lha::header& now)
{
    BOOST_CHECK(old.level == now.level);
    BOOST_CHECK(old.update_time == now.update_time);
    BOOST_CHECK_EQUAL(old.attributes, now.attributes);
    BOOST_CHECK_EQUAL(old.path.string(), now.path.string());
    BOOST_CHECK_EQUAL(old.link_path.string(), now.link_path.string());
    BOOST_CHECK_EQUAL(old.os, now.os);
}

void h1_test()
{
    std::string data("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    ar::lha::header head;
    head.level = 1;
    head.update_time = std::time(0);
    head.attributes = ar::msdos::attributes::read_only;
    head.path = "h0_test.dat";
    head.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

    BOOST_CHECK(!src.next_entry());
}

void abs_path_test()
{
    ar::lha::header head;
    head.level = 1;
    head.update_time = std::time(0);
    head.attributes = ar::msdos::attributes::directory;
    head.path = "/dir/file";
    head.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test_aux(const fs::path& link, const fs::path& target, bool dir)
{
    ar::lha::header head;
    head.level = 1;
    head.update_time = std::time(0);
    head.attributes = ar::msdos::attributes::hidden;
    if (dir)
        head.attributes |= ar::msdos::attributes::directory;
    head.path = link;
    head.link_path = target;
    head.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test()
{
    for (int i = 0; i < 2; ++i)
    {
        bool b = (i != 0);

        symlink_test_aux("link", "target", b);
        symlink_test_aux("link", "dir/target", b);
        symlink_test_aux("dir/link", "target", b);
        symlink_test_aux("dir1/link", "dir2/target", b);
    }
}

void fname_size_test_aux(std::size_t size)
{
    ar::lha::header head;
    head.level = 1;
    head.update_time = std::time(0);
    head.attributes = ar::msdos::attributes::read_only;
    head.path = std::string(size, 'A');
    head.os = 'M';

    io_ex::tmp_file archive;
    ar::basic_lzh_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_lzh_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    typedef std::char_traits<char> traits_type;
    BOOST_CHECK(traits_type::eq_int_type(io::get(src), traits_type::eof()));

    BOOST_CHECK(!src.next_entry());
}

void fname_size_test()
{
    fname_size_test_aux(232);
    fname_size_test_aux(233);
    fname_size_test_aux(255);
    fname_size_test_aux(256);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("LZH h1 test");
    test->add(BOOST_TEST_CASE(&h1_test));
    test->add(BOOST_TEST_CASE(&abs_path_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    test->add(BOOST_TEST_CASE(&fname_size_test));
    return test;
}
