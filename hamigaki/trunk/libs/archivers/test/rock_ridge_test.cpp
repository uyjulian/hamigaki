// rock_ridge_test.cpp: test case for ISO 9660 with Rock Ridge

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

bool date_time_equal_sec(
    const ar::iso::date_time& lhs, const ar::iso::date_time& rhs)
{
    return
        (std::memcmp(lhs.year,        rhs.year,         4) == 0) &&
        (std::memcmp(lhs.month,       rhs.month,        2) == 0) &&
        (std::memcmp(lhs.day,         rhs.day,          2) == 0) &&
        (std::memcmp(lhs.hour,        rhs.hour,         2) == 0) &&
        (std::memcmp(lhs.minute,      rhs.minute,       2) == 0) &&
        (std::memcmp(lhs.second,      rhs.second,       2) == 0) &&
//      (std::memcmp(lhs.centisecond, rhs.centisecond,  2) == 0) &&
        (lhs.timezone == rhs.timezone) ;
}

void check_header(const ar::iso::header& old, const ar::iso::header& now)
{
    BOOST_CHECK_EQUAL(old.path.string(), now.path.string());
    BOOST_CHECK_EQUAL(static_cast<unsigned>(now.version), 0u);
    BOOST_CHECK_EQUAL(old.link_path.string(), now.link_path.string());
    BOOST_CHECK(now.data_pos != 0u);
    BOOST_CHECK_EQUAL(old.file_size, now.file_size);
    BOOST_CHECK(::date_time_equal_sec(old.recorded_time, now.recorded_time));
    BOOST_CHECK_BITWISE_EQUAL(old.flags, now.flags);
    BOOST_CHECK(old.attributes == now.attributes);
    if (old.attributes && now.attributes)
    {
        BOOST_CHECK_EQUAL(
            old.attributes->permissions, now.attributes->permissions);
        BOOST_CHECK_EQUAL(old.attributes->links, now.attributes->links);
        BOOST_CHECK_EQUAL(old.attributes->uid, now.attributes->uid);
        BOOST_CHECK_EQUAL(old.attributes->gid, now.attributes->gid);
        BOOST_CHECK_EQUAL(old.attributes->serial_no, now.attributes->serial_no);
    }
    BOOST_CHECK(old.device_number == now.device_number);
    BOOST_CHECK(date_time_equal_sec(old.creation_time, now.creation_time));
    BOOST_CHECK(date_time_equal_sec(old.last_write_time, now.last_write_time));
    BOOST_CHECK(
        date_time_equal_sec(old.last_access_time, now.last_access_time));
    BOOST_CHECK(
        date_time_equal_sec(old.last_change_time, now.last_change_time));
    BOOST_CHECK(
        date_time_equal_sec(old.last_backup_time, now.last_backup_time));
    BOOST_CHECK(date_time_equal_sec(old.expiration_time, now.expiration_time));
    BOOST_CHECK(date_time_equal_sec(old.effective_time, now.effective_time));
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

void empty_test_aux(ar::iso::rrip_type rrip)
{
    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.rrip = rrip;
    sink.add_volume_desc(desc);

    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    BOOST_CHECK(!src.next_entry());
}

void empty_test()
{
    ::empty_test_aux(ar::iso::rrip_1991a);
    ::empty_test_aux(ar::iso::ieee_p1282);
}

void rock_ridge_test_aux(ar::iso::rrip_type rrip)
{
    std::string data(2049u, 'a');

    ar::iso::header head;
    head.path = "rock_ridge_test.txt";
    head.version = 1u;
    head.file_size = static_cast<boost::uint32_t>(data.size());

    head.recorded_time.year     = 2000u-1900u;
    head.recorded_time.month    = 12u;
    head.recorded_time.day      = 31u;
    head.recorded_time.hour     = 23u;
    head.recorded_time.minute   = 59u;
    head.recorded_time.second   = 59u;
    head.recorded_time.timezone = 9*4;

    ar::iso::posix::file_attributes attr;
    attr.permissions = 0100644u;
    attr.links = 0u;
    attr.uid = 1234u;
    attr.gid = 5678u;
    attr.serial_no = 838861u;
    head.attributes = attr;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.rrip = rrip;
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    io_ex::blocking_write(sink, &data[0], data.size());
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    head.attributes->links = 1u;
    if (rrip == ar::iso::rrip_1991a)
        head.attributes->serial_no = 0u;

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    ::check_file(src, head, data);

    BOOST_CHECK(!src.next_entry());
}

void rock_ridge_test()
{
    ::rock_ridge_test_aux(ar::iso::rrip_1991a);
    ::rock_ridge_test_aux(ar::iso::ieee_p1282);
}

void rock_ridge_dir_test_aux(ar::iso::rrip_type rrip)
{
    ar::iso::header head;
    head.path = "rock_ridge_test";
    head.flags = ar::iso::file_flags::directory;

    head.recorded_time.year     = 1970u-1900u;
    head.recorded_time.month    = 1u;
    head.recorded_time.day      = 1u;
    head.recorded_time.hour     = 0u;
    head.recorded_time.minute   = 0u;
    head.recorded_time.second   = 0u;
    head.recorded_time.timezone = 0;

    ar::iso::posix::file_attributes attr;
    attr.permissions = 040755u;
    attr.links = 0u;
    attr.uid = 1234u;
    attr.gid = 5678u;
    attr.serial_no = 838861u;
    head.attributes = attr;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.rrip = rrip;
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    head.attributes->links = 2u;
    if (rrip == ar::iso::rrip_1991a)
        head.attributes->serial_no = 0u;

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    BOOST_REQUIRE(src.next_entry());
    ::check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void rock_ridge_dir_test()
{
    ::rock_ridge_dir_test_aux(ar::iso::rrip_1991a);
    ::rock_ridge_dir_test_aux(ar::iso::ieee_p1282);
}

void symlink_test_aux(
    ar::iso::rrip_type rrip, const boost::filesystem::path& ph)
{
    ar::iso::header head;
    head.path = "rock_ridge_test";
    head.link_path = ph;

    head.recorded_time.year     = 1970u-1900u;
    head.recorded_time.month    = 1u;
    head.recorded_time.day      = 1u;
    head.recorded_time.hour     = 0u;
    head.recorded_time.minute   = 0u;
    head.recorded_time.second   = 0u;
    head.recorded_time.timezone = 0;

    ar::iso::posix::file_attributes attr;
    attr.permissions = 0120777u;
    attr.links = 0u;
    attr.uid = 1234u;
    attr.gid = 5678u;
    attr.serial_no = 838861u;
    head.attributes = attr;

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    ar::iso::volume_desc desc;
    desc.rrip = rrip;
    sink.add_volume_desc(desc);

    sink.create_entry(head);
    sink.close();
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    head.attributes->links = 1u;
    if (rrip == ar::iso::rrip_1991a)
        head.attributes->serial_no = 0u;

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    BOOST_REQUIRE(src.next_entry());
    ::check_header(head, src.header());

    BOOST_CHECK(!src.next_entry());
}

void symlink_test()
{
    ::symlink_test_aux(ar::iso::rrip_1991a, "dir");
    ::symlink_test_aux(ar::iso::ieee_p1282, "dir");
    ::symlink_test_aux(ar::iso::rrip_1991a, ".");
    ::symlink_test_aux(ar::iso::ieee_p1282, ".");
    ::symlink_test_aux(ar::iso::rrip_1991a, "..");
    ::symlink_test_aux(ar::iso::ieee_p1282, "..");
    ::symlink_test_aux(ar::iso::rrip_1991a, "/");
    ::symlink_test_aux(ar::iso::ieee_p1282, "/");
    ::symlink_test_aux(ar::iso::rrip_1991a, "/dir/.././link_target");
    ::symlink_test_aux(ar::iso::ieee_p1282, "/dir/.././link_target");
}

void deep_dir_test_aux(ar::iso::rrip_type rrip, bool is_enhanced)
{
    static const std::size_t dir_count = 16u;

    ar::iso::header heads[dir_count];
    boost::filesystem::path ph;
    for (std::size_t i = 0; i < dir_count; ++i)
    {
        ph /= hamigaki::to_dec<char>(i);
        heads[i].path = ph;
        heads[i].flags = ar::iso::file_flags::directory;

        ar::iso::posix::file_attributes attr;
        attr.permissions = 040755u;
        attr.links = 0u;
        attr.uid = 1234u;
        attr.gid = 5678u;
        attr.serial_no = static_cast<boost::uint32_t>(i);
        heads[i].attributes = attr;
    }

    io_ex::tmp_file archive;
    ar::basic_iso_file_sink<
        io_ex::dont_close_device<io_ex::tmp_file>
    > sink(io_ex::dont_close(archive));

    if (is_enhanced)
    {
        ar::iso::volume_desc desc;
        sink.add_volume_desc(desc);
        desc.set_enhanced();
        desc.rrip = rrip;
        sink.add_volume_desc(desc);
    }
    else
    {
        ar::iso::volume_desc desc;
        desc.rrip = rrip;
        sink.add_volume_desc(desc);
    }

    for (std::size_t i = 0; i < dir_count; ++i)
    {
        sink.create_entry(heads[i]);
        sink.close();
    }
    sink.close_archive();

    io::seek(archive, 0, BOOST_IOS::beg);

    for (std::size_t i = 0; i < dir_count-1; ++i)
        heads[i].attributes->links = 3u;
    heads[dir_count-1].attributes->links = 2u;

    if (rrip == ar::iso::rrip_1991a)
    {
        for (std::size_t i = 0; i < dir_count; ++i)
            heads[i].attributes->serial_no = 0u;
    }

    ar::basic_iso_file_source<io_ex::tmp_file> src(archive);

    if (is_enhanced)
        src.select_volume_desc(1u);

    for (std::size_t i = 0; i < dir_count; ++i)
    {
        BOOST_REQUIRE(src.next_entry());
        ::check_header(heads[i], src.header());
    }

    if (!is_enhanced)
    {
        BOOST_REQUIRE(src.next_entry());
        BOOST_CHECK_EQUAL(src.header().path.string(), std::string("rr_moved"));
    }

    BOOST_CHECK(!src.next_entry());
}

void deep_dir_test()
{
    ::deep_dir_test_aux(ar::iso::rrip_1991a, false);
    ::deep_dir_test_aux(ar::iso::ieee_p1282, false);
    ::deep_dir_test_aux(ar::iso::rrip_1991a, true);
    ::deep_dir_test_aux(ar::iso::ieee_p1282, true);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("Rock Ridge test");
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&rock_ridge_test));
    test->add(BOOST_TEST_CASE(&rock_ridge_dir_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    test->add(BOOST_TEST_CASE(&deep_dir_test));
    return test;
}
