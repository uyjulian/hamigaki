// joliet_wide_test.cpp: test case for ISO 9660 with Joliet Extension (Unicode)

// Copyright Takeshi Mouri 2008.
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

void check_header(const ar::iso::wheader& old, const ar::iso::wheader& now)
{
    BOOST_CHECK(old.path == now.path);
    BOOST_CHECK_EQUAL(
        static_cast<unsigned>(old.version), static_cast<unsigned>(now.version));
    BOOST_CHECK(old.link_path == now.link_path);
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

void check_desc(
    const ar::iso::wvolume_desc& old, const ar::iso::wvolume_desc& now)
{
    BOOST_CHECK_EQUAL(old.level, now.level);
    BOOST_CHECK(old.rrip == now.rrip);
    BOOST_CHECK_EQUAL(
        static_cast<unsigned>(old.type), static_cast<unsigned>(now.type));
    BOOST_CHECK_EQUAL(
        static_cast<unsigned>(old.version), static_cast<unsigned>(now.version));
    BOOST_CHECK_EQUAL(
        static_cast<unsigned>(old.flags), static_cast<unsigned>(now.flags));
    BOOST_CHECK(old.system_id == now.system_id);
    BOOST_CHECK(old.volume_id == now.volume_id);
    BOOST_CHECK(old.volume_set_id == now.volume_set_id);
    BOOST_CHECK(old.publisher_id == now.publisher_id);
    BOOST_CHECK(old.data_preparer_id == now.data_preparer_id);
    BOOST_CHECK(old.application_id == now.application_id);
    BOOST_CHECK(old.copyright_file_id == now.copyright_file_id);
    BOOST_CHECK(old.abstract_file_id == now.abstract_file_id);
    BOOST_CHECK(old.bibliographic_file_id == now.bibliographic_file_id);
    BOOST_CHECK_EQUAL(
        static_cast<unsigned>(old.file_structure_version),
        static_cast<unsigned>(now.file_structure_version));
    BOOST_CHECK(std::memcmp(old.application_use, now.application_use, 512)==0);
}

void unicode_test()
{
    ar::iso::wheader head;
    head.path = L"\x3053\x3093\x306B\x3061\x306F.txt";
    head.version = 1u;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        boost::filesystem::wpath
    > sink(io_ex::dont_close(archive));

    ar::iso::wvolume_desc desc;
    sink.add_volume_desc(desc);
    desc.set_joliet();
    desc.system_id = L"\x3053\x3093\x306B\x3061\x306F";
    desc.volume_id = L"\x4F60\x597D";
    desc.volume_set_id = L"\uC548\uB155\uD558\uC2ED\uB2C8\uAE4C";
    desc.publisher_id = L"\x3053\x3093\x306B\x3061\x306F";
    desc.data_preparer_id = L"\x4F60\x597D";
    desc.application_id = L"\uC548\uB155\uD558\uC2ED\uB2C8\uAE4C";
    desc.copyright_file_id = L"\x3053\x3093\x306B\x3061\x306F";
    desc.abstract_file_id = L"\x4F60\x597D";
    desc.bibliographic_file_id = L"\uC548\uB155\uD558\uC2ED\uB2C8\uAE4C";
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<
        io_ex::tmp_file,
        boost::filesystem::wpath
    > src(archive);

    check_desc(desc, src.volume_descs().at(1));
    src.select_volume_desc(1);

    BOOST_CHECK(src.next_entry());

    check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void narrow_to_wide_test()
{
    ar::iso::header head;
    head.path = "Hello.txt";
    head.version = 1u;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        fs::path
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    sink.add_volume_desc(desc);
    desc.set_joliet();
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file,fs::wpath> src(archive);
    src.select_volume_desc(1);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK(src.header().path == L"Hello.txt");

    BOOST_CHECK(!src.next_entry());
}

void wide_to_narrow_test()
{
    ar::iso::wheader head;
    head.path = L"Hello.txt";
    head.version = 1u;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>,
        fs::wpath
    > sink(io_ex::dont_close(archive));

    ar::iso::wvolume_desc desc;
    sink.add_volume_desc(desc);
    desc.set_joliet();
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file,fs::path> src(archive);
    src.select_volume_desc(1);

    BOOST_CHECK(src.next_entry());

    BOOST_CHECK(src.header().path == "Hello.txt");

    BOOST_CHECK(!src.next_entry());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("Joliet wide test");
    test->add(BOOST_TEST_CASE(&unicode_test));
    test->add(BOOST_TEST_CASE(&narrow_to_wide_test));
    test->add(BOOST_TEST_CASE(&wide_to_narrow_test));
    return test;
}
