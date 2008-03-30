// iso_date_time_test.cpp: test case for ISO 9660 date time

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/iso/date_time.hpp>
#include <boost/test/unit_test.hpp>

namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;
namespace ut = boost::unit_test;

void empty_test()
{
    BOOST_CHECK(ar::iso::date_time().empty());
    BOOST_CHECK(ar::iso::binary_date_time().empty());

    BOOST_CHECK(!ar::iso::date_time().to_timestamp());
    BOOST_CHECK(!ar::iso::binary_date_time().to_timestamp());
}

void unix_epoch_test()
{
    const ar::iso::date_time& dt =
        ar::iso::date_time::from_timestamp(fs_ex::timestamp());

    BOOST_CHECK(std::memcmp(dt.year, "1970", 4u) == 0);
    BOOST_CHECK(std::memcmp(dt.month, "01", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.day, "01", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.hour, "00", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.minute, "00", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.second, "00", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.centisecond, "00", 2u) == 0);
    BOOST_CHECK_EQUAL(static_cast<int>(dt.timezone), 0);

    const boost::optional<fs_ex::timestamp>& ts = dt.to_timestamp();
    BOOST_REQUIRE(ts);
    BOOST_CHECK_EQUAL(ts->seconds, static_cast<boost::int64_t>(0));
    BOOST_CHECK_EQUAL(ts->nanoseconds, static_cast<boost::uint32_t>(0u));
}

void bin_unix_epoch_test()
{
    const ar::iso::binary_date_time& dt =
        ar::iso::binary_date_time::from_timestamp(fs_ex::timestamp());

    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt.year), 70u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt.month), 1u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt.day), 1u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt.hour), 0u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt.minute), 0u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt.second), 0u);
    BOOST_CHECK_EQUAL(static_cast<int>(dt.timezone), 0);

    const boost::optional<fs_ex::timestamp>& ts = dt.to_timestamp();
    BOOST_REQUIRE(ts);
    BOOST_CHECK_EQUAL(ts->seconds, static_cast<boost::int64_t>(0));
    BOOST_CHECK_EQUAL(ts->nanoseconds, static_cast<boost::uint32_t>(0u));
}

void date_time_test()
{
    ar::iso::date_time dt;

    std::memcpy(dt.year, "2007", 4u);
    std::memcpy(dt.month, "03", 2u);
    std::memcpy(dt.day, "13", 2u);
    std::memcpy(dt.hour, "23", 2u);
    std::memcpy(dt.minute, "39", 2u);
    std::memcpy(dt.second, "31", 2u);
    std::memcpy(dt.centisecond, "27", 2u);
    dt.timezone = 36;

    const boost::optional<fs_ex::timestamp>& ts = dt.to_timestamp();
    BOOST_REQUIRE(ts);

    const ar::iso::date_time& dt2 = ar::iso::date_time::from_timestamp(*ts);

    BOOST_CHECK(std::memcmp(dt2.year, "2007", 4u) == 0);
    BOOST_CHECK(std::memcmp(dt2.month, "03", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt2.day, "13", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt2.hour, "14", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt2.minute, "39", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt2.second, "31", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt2.centisecond, "27", 2u) == 0);
    BOOST_CHECK_EQUAL(static_cast<int>(dt2.timezone), 0);
}

void bin_date_time_test()
{
    ar::iso::binary_date_time dt;

    dt.year = 107u;
    dt.month = 3u;
    dt.day = 13u;
    dt.hour = 23u;
    dt.minute = 39u;
    dt.second = 31u;
    dt.timezone = 36;

    const boost::optional<fs_ex::timestamp>& ts = dt.to_timestamp();
    BOOST_REQUIRE(ts);

    const ar::iso::binary_date_time& dt2 =
        ar::iso::binary_date_time::from_timestamp(*ts);

    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt2.year), 107u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt2.month), 3u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt2.day), 13u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt2.hour), 14u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt2.minute), 39u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(dt2.second), 31u);
    BOOST_CHECK_EQUAL(static_cast<int>(dt2.timezone), 0);
}

void date_time_cvt_test()
{
    ar::iso::binary_date_time bin;

    bin.year = 107u;
    bin.month = 3u;
    bin.day = 13u;
    bin.hour = 23u;
    bin.minute = 39u;
    bin.second = 31u;
    bin.timezone = 36;

    const ar::iso::date_time& dt = bin.to_date_time();

    BOOST_CHECK(std::memcmp(dt.year, "2007", 4u) == 0);
    BOOST_CHECK(std::memcmp(dt.month, "03", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.day, "13", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.hour, "23", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.minute, "39", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.second, "31", 2u) == 0);
    BOOST_CHECK(std::memcmp(dt.centisecond, "00", 2u) == 0);
    BOOST_CHECK_EQUAL(static_cast<int>(dt.timezone), 36);

    const ar::iso::binary_date_time& bin2 =
        ar::iso::binary_date_time::from_date_time(dt);

    BOOST_CHECK_EQUAL(static_cast<unsigned>(bin2.year), 107u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(bin2.month), 3u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(bin2.day), 13u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(bin2.hour), 23u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(bin2.minute), 39u);
    BOOST_CHECK_EQUAL(static_cast<unsigned>(bin2.second), 31u);
    BOOST_CHECK_EQUAL(static_cast<int>(bin2.timezone), 36);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("ISO 9660 date time test");
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&unix_epoch_test));
    test->add(BOOST_TEST_CASE(&bin_unix_epoch_test));
    test->add(BOOST_TEST_CASE(&date_time_test));
    test->add(BOOST_TEST_CASE(&bin_date_time_test));
    test->add(BOOST_TEST_CASE(&date_time_cvt_test));
    return test;
}
