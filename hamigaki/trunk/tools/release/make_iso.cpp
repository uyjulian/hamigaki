// make_iso.cpp: ISO image writer for Hamigaki release tool

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/release/ for library home page.

#include "make_iso.hpp"
#include "file_types.hpp"
#include <hamigaki/archivers/iso_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/lambda/lambda.hpp>
#include <algorithm>
#include <ctime>
#include <ostream>

namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;

namespace
{

fs::path remove_top_dir(const fs::path& ph)
{
    namespace bl = boost::lambda;

    fs::path tmp;
    if (!ph.empty())
        std::for_each(boost::next(ph.begin()), ph.end(), bl::var(tmp)/=bl::_1);

    if (tmp.empty())
        return ph;
    else
        return tmp;
}

void add_enetry(
    ar::iso_file_sink& iso, const fs::path& ph,
    boost::uint32_t serial_no, const ar::iso::binary_date_time& recorded_time)
{
    const fs_ex::file_status s = fs_ex::status(ph);

    ar::iso::header head;
    head.path = remove_top_dir(ph);
    head.type(s.type());
    head.recorded_time = recorded_time;

    if (fs_ex::is_regular(s))
        head.file_size = s.file_size();

    ar::iso::posix::file_attributes attr;
    attr.serial_no = serial_no;
    if (s.has_permissions())
        attr.permissions = s.permissions();
    else if (fs_ex::is_directory(s))
        attr.permissions = fs_ex::file_permissions::directory | 0555u;
    else
        attr.permissions = fs_ex::file_permissions::regular | 0444u;
    if (fs_ex::is_regular(s) && is_executable(fs::extension(ph)))
        attr.permissions |= 0111;
    head.attributes = attr;

    typedef ar::iso::date_time time_type;
    head.last_write_time =
        time_type::from_timestamp(s.last_write_time());
    head.last_access_time =
        time_type::from_timestamp(s.last_access_time());
    if (s.has_last_change_time())
    {
        head.last_change_time =
            time_type::from_timestamp(s.last_change_time());
    }

    iso.create_entry(head);

    if (is_regular(s))
    {
        fs::ifstream is(ph, std::ios_base::binary);
        io::copy(is, iso);
    }
    else
        iso.close();
}

} // namespace

void make_iso_image(std::ostream& logs, const std::string& ver)
{
    ar::iso_file_sink iso("hamigaki_" + ver + ".iso");

    {
        ar::iso::volume_desc desc;
        desc.rrip = ar::iso::rrip_1991a;
        desc.volume_id = "HAMIGAKI_" + ver;
        desc.application_id = "HAMIGAKI LIBRARIES RELEASE TOOL";
        iso.add_volume_desc(desc);
    }
    {
        ar::iso::volume_desc desc;
        desc.set_joliet();
        desc.volume_id = "hamigaki_" + ver;
        desc.application_id = "Hamigaki libraries release tool";
        iso.add_volume_desc(desc);
    }

    ar::iso::binary_date_time recorded_time;
    std::time_t t = std::time(0);
    if (std::tm* gt = std::gmtime(&t))
    {
        recorded_time.year     = gt->tm_year;
        recorded_time.month    = gt->tm_mon+1;
        recorded_time.day      = gt->tm_mday;
        recorded_time.hour     = gt->tm_hour;
        recorded_time.minute   = gt->tm_min;
        recorded_time.second   = gt->tm_sec;
        recorded_time.timezone = 0;
    }

    fs::recursive_directory_iterator beg("hamigaki_" + ver);
    fs::recursive_directory_iterator end;

    boost::uint32_t serial_no = 0;
    for (; beg != end; ++beg)
    {
        const fs::path& ph = beg->path();
        logs << ph << std::endl;
        add_enetry(iso, ph, ++serial_no, recorded_time);
    }

    const fs::path tbz_ph = "hamigaki_" + ver + ".tar.bz2";
    if (fs::exists(tbz_ph))
    {
        logs << tbz_ph << std::endl;
        add_enetry(iso, tbz_ph, ++serial_no, recorded_time);
    }

    const fs::path zip_ph = "hamigaki_" + ver + ".zip";
    if (fs::exists(zip_ph))
    {
        logs << zip_ph << std::endl;
        add_enetry(iso, zip_ph, ++serial_no, recorded_time);
    }

    iso.close_archive();
}
