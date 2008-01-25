// propagate_test.cpp: test case for the propagation of the exception

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <cstddef>
#include <cstring>
#include <exception>
#include <typeinfo>
#include <unwind.h> // for _Unwind_Ptr, _Unwind_Exception

namespace __cxxabiv1
{

extern "C"
{

struct __cxa_eh_globals;
struct __cxa_exception;

void* __cxa_allocate_exception(std::size_t) throw();
void __cxa_free_exception(void*) throw();

void __cxa_throw(void*, std::type_info*, void (*)(void*))
__attribute__((noreturn));

__cxa_eh_globals* __cxa_get_globals() throw();


} // extern "C"

} // namespace __cxxabiv1

struct my_eh_globals
{
    __cxxabiv1::__cxa_exception* list;
    unsigned int count;
};

struct my_cxa_exception
{ 
    std::type_info* type;
    void (*destroy)(void*); 
    std::unexpected_handler on_unexpected;
    std::terminate_handler on_terminate;
    my_cxa_exception* next;
    int dummy0;
    int dummy1;
    const unsigned char* dummy2;
    const unsigned char* dummy3;
    ::_Unwind_Ptr dummy4;
    void* dummy5;
    ::_Unwind_Exception header;
};

void free_exception(my_cxa_exception* e)
{
    void* p = e + 1;

    if (e->destroy)
        e->destroy(p);
    __cxxabiv1::__cxa_free_exception(p);
}

my_cxa_exception* release_exception()
{
    my_eh_globals* g =
        reinterpret_cast<my_eh_globals*>(__cxxabiv1::__cxa_get_globals());

    my_cxa_exception* tmp =
        reinterpret_cast<my_cxa_exception*>(g->list);

    g->list = 0;
    g->count = 0;

    return tmp;
}

void rethrow_exception(my_cxa_exception* e) __attribute__((noreturn));
void rethrow_exception(my_cxa_exception* e)
{
    void* p = e + 1;
    std::type_info* type = e->type;
    void (*destroy)(void*) = e->destroy;

    std::memset(e, 0, sizeof(my_cxa_exception));

    __cxxabiv1::__cxa_throw(p, type, destroy);
}


#include <boost/bind.hpp>
#include <boost/thread.hpp>

class my_thread
{
public:
    my_thread() : e_(0), t_(boost::bind(&my_thread::work, this))
    {
    }

    void wait()
    {
        t_.join();
        if (e_)
            rethrow_exception(e_);
    }

private:
    boost::thread t_;
    my_cxa_exception* e_;

    void work()
    {
        try
        {
            throw std::runtime_error("error!");
        }
        catch (...)
        {
            e_ = release_exception();
        }
    }
};

int main()
{
    try
    {
        my_thread t;
        t.wait();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "caught runtime_error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "unknown exception" << std::endl;
    }
}
