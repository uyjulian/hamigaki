//  lzh_h1_test.cpp: test case for LZH with level-1 header

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

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

    BOOST_CHECK(head.level == src.header().level);
    BOOST_CHECK((src.header().update_time - head.update_time) >= 0);
    BOOST_CHECK((src.header().update_time - head.update_time) <= 1);
    BOOST_CHECK_EQUAL(head.attributes, src.header().attributes);
    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK_EQUAL(head.os, src.header().os);

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

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

    BOOST_CHECK(head.level == src.header().level);
    BOOST_CHECK((src.header().update_time - head.update_time) >= 0);
    BOOST_CHECK((src.header().update_time - head.update_time) <= 1);
    BOOST_CHECK_EQUAL(head.attributes, src.header().attributes);
    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK_EQUAL(head.link_path.string(), src.header().link_path.string());
    BOOST_CHECK_EQUAL(head.os, src.header().os);

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

        symlink_test_aux("link", "target", b);
        symlink_test_aux("link", "dir/target", b);
        symlink_test_aux("dir/link", "target", b);
        symlink_test_aux("dir1/link", "dir2/target", b);
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("LZH h1 test");
    test->add(BOOST_TEST_CASE(&h1_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    return test;
}
