// release.cpp: Hamigaki release tool

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/release/ for library home page.

#include "make_iso.hpp"
#include "make_tbz.hpp"
#include "make_zip.hpp"
#include <hamigaki/filesystem/operations.hpp>
#include <hamigaki/process/context.hpp>
#include <hamigaki/process/environment.hpp>
#include <hamigaki/process/shell.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/copy.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace fs_ex = hamigaki::filesystem;
namespace proc = hamigaki::process;

#if BOOST_WINDOWS
std::string to_cygwin_path(const std::string& s)
{
    const std::string& tmp = algo::replace_all_copy(s, "\\", "\\\\");
    return proc::shell_expand("C:\\cygwin\\bin\\cygpath.exe " + tmp);
}
#endif

std::string get_hamigaki_version()
{
    fs::path ph(fs::initial_path<fs::path>()/"hamigaki/hamigaki/version.hpp");
    fs::ifstream is(ph);
    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("HAMIGAKI_LIB_VERSION") != std::string::npos)
        {
            std::string::size_type beg = line.find('"') + 1;
            std::string::size_type end = line.find('"', beg);

            std::string ver(line, beg, end-beg);
            if (std::count(ver.begin(), ver.end(), '_') == 1)
                ver += "_0";
            return ver;
        }
    }
    return "0_0_0";
}

void wait_child(proc::child& c, const std::string& name)
{
    const proc::status st = c.wait();
    if (st.get_type() != proc::status::exited)
        throw std::runtime_error(name + " exited abnormally");
    else if (st.code() != 0)
    {
        std::ostringstream os;
        os << name << " exited by " << st.code();
        throw std::runtime_error(os.str());
    }
}

struct eol_type
{
    enum values { lf, crlf };
};

void export_hamigaki(eol_type::values eol)
{
    std::string cmd;
    cmd += "svn export ";
    if (eol == eol_type::lf)
        cmd += "--native-eol LF ";
    else
        cmd += "--native-eol CRLF ";
    cmd += "http://svn.sourceforge.jp/svnroot/hamigaki/hamigaki/trunk ";
    cmd += "hamigaki";

    proc::child c = proc::launch_shell(cmd);
    ::wait_child(c, "svn");
}

void build_documents()
{
    const fs::path work = fs::initial_path<fs::path>() / "hamigaki/doc";

    proc::context ctx;
    ctx.work_directory(work.directory_string());
    proc::child c = proc::launch_shell("bjam", ctx);
    ::wait_child(c, "bjam");
}

void update_regression_logs()
{
    const fs::path work = fs::initial_path<fs::path>() / "hamigaki/status";

    proc::context ctx;
    ctx.work_directory(work.directory_string());
#if BOOST_WINDOWS
    ctx.stdout_behavior(proc::capture_stream());

    proc::environment env;
    env.set("PWD", ::to_cygwin_path(ctx.work_directory()));

    std::vector<std::string> args;
    args.push_back("perl");
    args.push_back("../tools/regression/regression-logs.pl");

    proc::child c("C:\\cygwin\\bin\\perl.exe", args, env, ctx);

    fs::ofstream os(work / "index.html");
    io::copy(c.stdout_source(), os);
#else
    proc::child c = proc::launch_shell(
        "perl ../tools/regression/regression-logs.pl > index.html", ctx);
#endif
    ::wait_child(c, "perl");
}

int main(int argc, char* argv[])
{
    try
    {
        const fs::path root = fs::initial_path<fs::path>();

        fs_ex::remove_all(root/"hamigaki");
        export_hamigaki(eol_type::lf);
        build_documents();
        fs_ex::remove_all(root/"hamigaki/bin.v2");
        update_regression_logs();
        const std::string& ver = get_hamigaki_version();
        fs::path dir = root / ("hamigaki_"+ver);
        fs_ex::remove_all(dir);
        fs::rename(root/"hamigaki", dir);

        ::make_tbz2_archive(std::cout, ver);

        export_hamigaki(eol_type::crlf);
        build_documents();
        fs_ex::remove_all(root/"hamigaki/bin.v2");
        update_regression_logs();
        if (get_hamigaki_version() != ver)
            throw std::runtime_error("version mismatch");
        fs_ex::remove_all(dir);
        fs::rename(root/"hamigaki", dir);

        ::make_zip_archive(std::cout, ver);
        ::make_iso_image(std::cout, ver);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
