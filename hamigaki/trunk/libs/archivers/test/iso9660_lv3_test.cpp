// iso9660_lv3_test.cpp: test case for ISO 9660 Level 3

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/iso_file.hpp>
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

bool date_time_equal_sec(
    const ar::iso::binary_date_time& lhs, const ar::iso::binary_date_time& rhs)
{
    return
        (lhs.year           == rhs.year     ) &&
        (lhs.month          == rhs.month    ) &&
        (lhs.day            == rhs.day      ) &&
        (lhs.hour           == rhs.hour     ) &&
        (lhs.minute         == rhs.minute   ) &&
        (lhs.second         == rhs.second   ) &&
        (lhs.timezone       == rhs.timezone ) ;
}

void check_header(const ar::iso::header& old, const ar::iso::header& now)
{
    BOOST_CHECK_EQUAL(old.path.string(), now.path.string());
    BOOST_CHECK_EQUAL(
        static_cast<unsigned>(old.version), static_cast<unsigned>(now.version));
    BOOST_CHECK_EQUAL(old.link_path.string(), now.link_path.string());
    BOOST_CHECK(now.data_pos != 0u);
    BOOST_CHECK_EQUAL(old.file_size, now.file_size);
    BOOST_CHECK(::date_time_equal_sec(old.recorded_time, now.recorded_time));
    BOOST_CHECK_BITWISE_EQUAL(old.flags, now.flags);
    BOOST_CHECK_EQUAL(old.system_use, now.system_use);
    BOOST_CHECK(!now.attributes);
    BOOST_CHECK(!now.device_number);
    BOOST_CHECK(now.creation_time.empty());
    BOOST_CHECK(now.last_write_time.empty());
    BOOST_CHECK(now.last_access_time.empty());
    BOOST_CHECK(now.last_change_time.empty());
    BOOST_CHECK(now.last_backup_time.empty());
    BOOST_CHECK(now.expiration_time.empty());
    BOOST_CHECK(now.effective_time.empty());
}

template<class Extractor>
void check_file(
    Extractor& src, const ar::iso::header& head, const std::string& data)
{
    BOOST_REQUIRE(src.next_entry());

    ::check_header(head, src.header());

    std::string data2;
    io::copy(src, io::back_inserter(data2));

    BOOST_CHECK_EQUAL_COLLECTIONS(
        data.begin(), data.end(), data2.begin(), data2.end()
    );
}

void empty_test()
{
    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.level = 3u;
    sink.add_volume_desc(desc);

    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(!src.next_entry());
}

void iso9660_test()
{
    std::string data(2049u, 'a');

    ar::iso::header head;
    head.path = "ISO_LEVEL_3_TEST_FILE.EXTENSION";
    head.version = 1u;
    head.file_size = static_cast<boost::uint32_t>(data.size());

    head.recorded_time.year     = 2000u-1900u;
    head.recorded_time.month    = 12u;
    head.recorded_time.day      = 31u;
    head.recorded_time.hour     = 23u;
    head.recorded_time.minute   = 59u;
    head.recorded_time.second   = 59u;
    head.recorded_time.timezone = 9*4;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.level = 3u;
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    ::check_file(src, head, data);

    BOOST_CHECK(!src.next_entry());
}

void iso9660_dir_test()
{
    ar::iso::header head;
    head.path = "ISO_LEVEL_3_TEST_DIRECTORY_NAME";
    head.flags = ar::iso::file_flags::directory;

    head.recorded_time.year     = 1970u-1900u;
    head.recorded_time.month    = 1u;
    head.recorded_time.day      = 1u;
    head.recorded_time.hour     = 0u;
    head.recorded_time.minute   = 0u;
    head.recorded_time.second   = 0u;
    head.recorded_time.timezone = 0;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.level = 3u;
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    BOOST_REQUIRE(src.next_entry());
    ::check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void multi_extent_test()
{
    std::string data(0x2000u, 'a');

    ar::iso::header head;
    head.path = "ISO_LEVEL_3_TEST_FILE.EXTENSION";
    head.version = 1u;
    head.file_size = static_cast<boost::uint32_t>(data.size());

    head.recorded_time.year     = 2000u-1900u;
    head.recorded_time.month    = 12u;
    head.recorded_time.day      = 31u;
    head.recorded_time.hour     = 23u;
    head.recorded_time.minute   = 59u;
    head.recorded_time.second   = 59u;
    head.recorded_time.timezone = 9*4;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive), ar::iso::volume_info(), 0x0800);

    ar::iso::volume_desc desc;
    desc.level = 3u;
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    head.flags = ar::iso::file_flags::multi_extent;
    ::check_file(src, head, data);

    BOOST_CHECK(!src.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("ISO 9660 Level 3 test");
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&iso9660_test));
    test->add(BOOST_TEST_CASE(&iso9660_dir_test));
    test->add(BOOST_TEST_CASE(&multi_extent_test));
    return test;
}
