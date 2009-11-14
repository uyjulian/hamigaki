// zip_crypt_test.cpp: test case for ZIP encryption

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

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

void zip_crypt_test()
{
    std::string data("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    ar::zip::header head;
    head.path = "zip_test.dat";
    head.encrypted = true;
    head.method = ar::zip::method::deflate;
    head.update_time = std::time(0);
    head.file_size = static_cast<boost::uint32_t>(data.size());
    head.attributes = ar::msdos::attributes::read_only;
    head.permissions = 0123;
    head.comment = "test comment";

    io_ex::tmp_file archive;
    ar::basic_zip_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    sink.password("test password");
    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_zip_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    std::string data2;
    BOOST_REQUIRE_THROW(
        (io::copy(src, io::back_inserter(data2))),
        ar::password_incorrect
    );

    src.password("test password");
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );

    BOOST_CHECK(!src.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("ZIP encryption test");
    test->add(BOOST_TEST_CASE(&zip_crypt_test));
    return test;
}
