//  simple_unzip.cpp: a simple zip decompressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <boost/config.hpp>

#include <hamigaki/iostreams/device/zip_file.hpp>
#include <exception>
#include <iostream>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <signal.h>
    #include <termios.h>
#endif

namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

#if defined(BOOST_WINDOWS)
bool disable_echo()
{
    ::HANDLE h = ::GetStdHandle(STD_INPUT_HANDLE);
    ::DWORD mode;
    if (::GetConsoleMode(h, &mode) && ((mode & ENABLE_ECHO_INPUT) != 0))
    {
        ::SetConsoleMode(h, mode & ~ENABLE_ECHO_INPUT);
        return true;
    }
    return false;
}

void enable_echo()
{
    ::HANDLE h = ::GetStdHandle(STD_INPUT_HANDLE);
    ::DWORD mode;
    ::GetConsoleMode(h, &mode);
    ::SetConsoleMode(h, mode | ENABLE_ECHO_INPUT);
}
#elif defined(BOOST_HAS_UNISTD_H)
void enable_echo_impl()
{
    ::termios data;
    ::tcgetattr(0, &data);
    data.c_lflag |= ECHO;
    ::tcsetattr(0, TCSAFLUSH, &data);
}

void on_interrupt(int)
{
    enable_echo_impl();
    ::_exit(0);
}

bool disable_echo()
{
    ::termios data;
    ::tcgetattr(0, &data);
    if (data.c_lflag & ECHO)
    {
        struct sigaction sa;
        sa.sa_handler = &on_interrupt;
        ::sigfillset(&sa.sa_mask);
        sa.sa_flags = 0;
        ::sigaction(SIGINT, &sa, 0);

        data.c_lflag &= ~ECHO;
        ::tcsetattr(0, TCSAFLUSH, &data);
        return true;
    }
    else
        return false;
}

void enable_echo()
{
    enable_echo_impl();

    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    ::sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    ::sigaction(SIGINT, &sa, 0);
}
#else
bool disable_echo()
{
    return false;
}

void enable_echo()
{
}
#endif

class scoped_echo_off
{
public:
    scoped_echo_off() : changed_(disable_echo())
    {
    }

    ~scoped_echo_off()
    {
        if (changed_)
            enable_echo();
    }

private:
    bool changed_;
};

std::string get_password()
{
    std::string pswd;
    scoped_echo_off echo_off;
    std::getline(std::cin, pswd);
    return pswd;
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
            return 1;

        bool need_pswd = true;
        io_ex::zip_file_source zip(argv[1]);

        while (zip.next_entry())
        {
            const io_ex::zip::header& head = zip.header();

            if (head.encrypted && need_pswd)
            {
                std::cout << "password: " << std::flush;
                std::string pswd = get_password();
                std::cout << '\n';

                zip.password(pswd);
                need_pswd = false;
            }

            std::cout << head.path.string() << '\n';
            if (head.is_symbolic_link())
                std::cout << "-> " << head.link_path.string() << '\n';
            else if (!head.is_directory())
            {
                char buf[256];
                std::streamsize n;
                while (n = zip.read(buf, sizeof(buf)), n >= 0)
                {
                    if (n)
                        std::cout.write(buf, n);
                }
                std::cout << '\n';
            }

            std::cout << "--------------------------------" << std::endl;
        }

        std::cout.flush();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
