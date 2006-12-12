//  tar_gnu_test.cpp: test case for ustar formatted tar

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/tar_file.hpp>
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

void tar_test()
{
    std::string data("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    ar::tar::header head;
    head.format = ar::tar::ustar;
    head.path = "tar_test.dat";
    head.modified_time = fs_ex::timestamp::from_time_t(std::time(0));
    head.file_size = data.size();
    head.permissions = 0123;
    head.comment = "test comment";

    io_ex::tmp_file archive;
    ar::basic_tar_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_tar_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK(head.link_path.empty());
    BOOST_REQUIRE(src.header().modified_time);
    BOOST_CHECK_EQUAL(
        head.modified_time->to_time_t(),
        src.header().modified_time->to_time_t()
    );
    BOOST_CHECK_EQUAL(head.permissions, src.header().permissions);
    BOOST_CHECK(src.header().comment.empty());

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

    BOOST_CHECK(!src.next_entry());
}

void dir_test()
{
    ar::tar::header head;
    head.format = ar::tar::ustar;
    head.path = "dir";
    head.modified_time = fs_ex::timestamp::from_time_t(std::time(0));
    head.file_size = 0;
    head.permissions = 0123;
    head.comment = "test comment";

    io_ex::tmp_file archive;
    ar::basic_tar_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_tar_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK(head.link_path.empty());
    BOOST_REQUIRE(src.header().modified_time);
    BOOST_CHECK_EQUAL(
        head.modified_time->to_time_t(),
        src.header().modified_time->to_time_t()
    );
    BOOST_CHECK_EQUAL(head.permissions, src.header().permissions);
    BOOST_CHECK(src.header().comment.empty());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test()
{
    ar::tar::header head;
    head.format = ar::tar::ustar;
    head.path = "link";
    head.link_path = "target";
    head.modified_time = fs_ex::timestamp::from_time_t(std::time(0));
    head.file_size = 0;
    head.permissions = 0123;
    head.comment = "test comment";

    io_ex::tmp_file archive;
    ar::basic_tar_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_tar_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK_EQUAL(head.link_path.string(), src.header().link_path.string());
    BOOST_REQUIRE(src.header().modified_time);
    BOOST_CHECK_EQUAL(
        head.modified_time->to_time_t(),
        src.header().modified_time->to_time_t()
    );
    BOOST_CHECK_EQUAL(head.permissions, src.header().permissions);
    BOOST_CHECK(src.header().comment.empty());

    BOOST_CHECK(!src.next_entry());
}

void path_length_test_aux(const fs::path& ph)
{
    std::string data("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    ar::tar::header head;
    head.format = ar::tar::ustar;
    head.path = ph;
    head.modified_time = fs_ex::timestamp::from_time_t(std::time(0));
    head.file_size = data.size();
    head.permissions = 0123;
    head.comment = "test comment";

    io_ex::tmp_file archive;
    ar::basic_tar_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_tar_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK_EQUAL(head.path.string(), src.header().path.string());
    BOOST_CHECK(head.link_path.empty());
    BOOST_REQUIRE(src.header().modified_time);
    BOOST_CHECK_EQUAL(
        head.modified_time->to_time_t(),
        src.header().modified_time->to_time_t()
    );
    BOOST_CHECK_EQUAL(head.permissions, src.header().permissions);
    BOOST_CHECK(src.header().comment.empty());

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

    BOOST_CHECK(!src.next_entry());
}

void path_length_test()
{
    const fs::path dir155(std::string(155u, 'a'));
    const fs::path dir156(std::string(156u, 'a'));

    const std::string name100(100u, 'a');
    const std::string name101(101u, 'a');

    path_length_test_aux(dir155 / name100);
    BOOST_CHECK_THROW(path_length_test_aux(dir155 / name101), std::exception);
    BOOST_CHECK_THROW(path_length_test_aux(dir156 / name100), std::exception);
    BOOST_CHECK_THROW(path_length_test_aux(dir156 / name101), std::exception);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("tar ustar test");
    test->add(BOOST_TEST_CASE(&tar_test));
    test->add(BOOST_TEST_CASE(&dir_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    test->add(BOOST_TEST_CASE(&path_length_test));
    return test;
}
