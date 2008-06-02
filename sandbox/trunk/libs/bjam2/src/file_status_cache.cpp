// file_status_cache.cpp: cache for file status

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM2_SOURCE
#include <hamigaki/bjam2/util/file_status_cache.hpp>

namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam2 {

namespace
{

fs::path path_complete(const std::string& path, const fs::path& work)
{
    fs::path ph(path);

    if (ph.empty())
        return work;
    else if (ph.has_root_name() && !ph.has_root_directory())
        return ph /= "";
    else
        return fs::complete(ph, work);
}

} // namespace

file_status_cache::file_status_cache()
    : work_(fs::current_path<fs::path>())
{
}

void file_status_cache::working_directory(const std::string& dir)
{
    if (work_ != dir)
    {
        work_ = dir;
        table_.clear();
    }
}

const boost::filesystem::file_status&
file_status_cache::status(const std::string& filename)
{
    file_info& st = table_[filename];
    if (!fs::status_known(st.status))
        st.status = fs::status(path_complete(filename, work_));
    return st.status;
}

const std::vector<std::string>&
file_status_cache::directory_entries(const std::string& dirname)
{
    file_info& st = table_[dirname];

    std::vector<std::string>* result = st.entries.get_ptr();
    if (!result)
    {
        const fs::path& ph = path_complete(dirname, work_);

        if (!fs::status_known(st.status))
            st.status = fs::status(ph);

        st.entries = std::vector<std::string>();
        result = st.entries.get_ptr();
        if (fs::is_directory(st.status))
        {
            fs::directory_iterator beg(ph);
            fs::directory_iterator end;
            for ( ; beg != end; ++beg)
                result->push_back(beg->path().leaf());
        }
    }
    return *result;
}

} } // End namespaces bjam2, hamigaki.
