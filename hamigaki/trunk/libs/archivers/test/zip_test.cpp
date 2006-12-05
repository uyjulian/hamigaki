//  zip_test.cpp: test case for ZIP

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

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

void zip_test()
{
    std::string data("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    ar::zip::header head;
    head.path = "zip_test.dat";
    head.encrypted = false;
    head.method = ar::zip::method::deflate;
    head.update_time = std::time(0);
    head.file_size = data.size();
    head.attributes = ar::msdos::attributes::read_only;
    head.permission = 0123;
    head.comment = "test comment";

    io_ex::tmp_file archive;
    ar::basic_zip_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_zip_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK(head.link_path.empty());
    BOOST_CHECK_EQUAL(head.encrypted, src.header().encrypted);
    BOOST_CHECK(head.method == src.header().method);
    BOOST_CHECK_EQUAL(head.update_time, src.header().update_time);
    BOOST_CHECK_EQUAL(head.attributes, src.header().attributes);
    BOOST_CHECK_EQUAL(head.permission, src.header().permission);
    BOOST_CHECK_EQUAL(head.comment, src.header().comment);

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

    BOOST_CHECK(!src.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("ZIP test");
    test->add(BOOST_TEST_CASE(&zip_test));
    return test;
}
