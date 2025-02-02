// child_test.cpp: test case for child processes

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#include <hamigaki/process/child.hpp>
#include <hamigaki/process/environment.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>

namespace proc = hamigaki::process;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void sort_test()
{
    proc::context ctx;
    ctx.stdin_behavior(proc::capture_stream());
    ctx.stdout_behavior(proc::capture_stream());
    ctx.stderr_behavior(proc::silence_stream());

#if defined(BOOST_WINDOWS)
    proc::child c("C:\\WINDOWS\\system32\\sort.exe", ctx);
#else
    proc::child c("/bin/sort", ctx);
#endif

    proc::pipe_sink& sink = c.stdin_sink();
    sink.write("q\r\nw\r\ne\r\n", 9);
    sink.close();

    std::string dst;
    io::copy(c.stdout_source(), io::back_inserter(dst));

    BOOST_CHECK_EQUAL(dst, std::string("e\r\nq\r\nw\r\n"));

    const proc::status& st = c.wait();
    BOOST_CHECK(st.get_type() == proc::status::exited);
    BOOST_CHECK_EQUAL(st.code(), 0u);
}

void terminate_test()
{
#if defined(BOOST_WINDOWS)
    proc::child c("C:\\WINDOWS\\system32\\sort.exe");
#else
    proc::child c("/bin/sort");
#endif
}

void env_test()
{
    proc::context ctx;
    ctx.stdin_behavior(proc::silence_stream());
    ctx.stdout_behavior(proc::capture_stream());
    ctx.stderr_behavior(proc::silence_stream());

    std::vector<std::string> args;

    proc::environment env;
    env.set("ENV_TEST", "test");

#if defined(BOOST_WINDOWS)
    args.push_back("cmd");
    args.push_back("/c");
    args.push_back("set");

    proc::child c("C:\\WINDOWS\\system32\\cmd.exe", args, env, ctx);
#else
    proc::child c("/bin/env", env, ctx);
#endif

    std::string dst;
    io::copy(c.stdout_source(), io::back_inserter(dst));

    BOOST_CHECK(dst.find("ENV_TEST=test") != std::string::npos);

    const proc::status& st = c.wait();
    BOOST_CHECK(st.get_type() == proc::status::exited);
    BOOST_CHECK_EQUAL(st.code(), 0u);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("child process test");
    test->add(BOOST_TEST_CASE(&sort_test));
    test->add(BOOST_TEST_CASE(&terminate_test));
    test->add(BOOST_TEST_CASE(&env_test));
    return test;
}
