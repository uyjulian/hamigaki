// bjam_dll_path.cpp: an utility for bjam DLL path

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/filesystem/fstream.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/filesystem.hpp>
#include <iterator>
#include <set>

namespace fs = boost::filesystem;
namespace lambda = boost::lambda;
namespace xp = boost::xpressive;

namespace
{

fs::path find_root(const fs::path& ph)
{
    if (ph.empty())
        return ph;
    else if (fs::exists(ph/"project-root.jam"))
        return ph;
    else if (fs::exists(ph/"Jamroot"))
        return ph;
    else if (fs::exists(ph/"Jamroot.jam"))
        return ph;
    else
        return find_root(ph.branch_path());
}

fs::path complete_by_root(const fs::path& ph, const fs::path& root)
{
    if (ph.is_complete())
        return ph;

    fs::path result = root;
    fs::path::const_iterator i =
        std::find_if(ph.begin(), ph.end(), lambda::_1 != "..");

    for (; i != ph.end(); ++i)
        result /= *i;

    return result;
}

class search_dll_path
{
public:
    explicit search_dll_path(const std::string& exe)
    {
        {
            using namespace boost::xpressive;
            const std::locale& loc = std::locale::classic();
            pattern_ = imbue(loc)('"' >> (s1 = +_) >> ".lib\"" >> *space);
        }

        parse(exe + ".rsp", false);
    }

    std::string paths() const
    {
        typedef std::set<fs::path>::const_iterator iter_type;

        std::string result;
        for (iter_type i = paths_.begin(); i != paths_.end(); ++i)
        {
            if (!result.empty())
                result += ';';
            result += i->directory_string();
        }
        return result;
    }

private:
    std::set<fs::path> paths_;
    xp::sregex pattern_;

    void parse(const fs::path& rsp, bool is_dll)
    {
        const fs::path& dir = rsp.branch_path();
        if (is_dll && !paths_.insert(dir).second)
            return;

        const fs::path& root = find_root(dir);

        fs::fstream is(rsp);
        std::string line;
        while (std::getline(is, line))
        {
            xp::smatch result;
            if (xp::regex_match(line, result, pattern_))
            {
                fs::path new_rsp(result[1].str() + ".dll.rsp");
                if (new_rsp.has_branch_path())
                    parse(complete_by_root(new_rsp, root), true);
            }
        }
    }
};

} // namespace

std::string get_bjam_dll_paths(const std::string& filename)
{
    search_dll_path paths(filename);
    return paths.paths();
}
