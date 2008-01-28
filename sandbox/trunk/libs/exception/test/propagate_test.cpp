// propagate_test.cpp: test case for the propagation of the exception

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#if defined(__GNUC__)
#include <cassert>
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
    __cxxabiv1::__cxa_exception* next;
    int count;
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

    assert(tmp->count == 1);
    tmp->count = -1;

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
    my_cxa_exception* e_;
    boost::thread t_;

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
#elif defined(_MSC_VER)
#include <cstring>
#include <malloc.h>
#include <windows.h>

extern "C"
{

struct _tiddata;
_tiddata* __cdecl _getptd();

} // extern "C"

const ::DWORD cxx_exception_code = 0xE06D7363;

const ::EXCEPTION_RECORD* get_current_exception_record()
{
    _tiddata* p = _getptd();
#if _MSC_VER < 1400
    return reinterpret_cast< ::EXCEPTION_RECORD**>(p)[31];
#else
    return reinterpret_cast< ::EXCEPTION_RECORD**>(p)[34];
#endif
}

inline void call_pmfn0(_PMFN f, void* p)
{
    __asm
    {
        mov ecx, p
        call f
    }
}

inline void call_pmfn1(_PMFN f, void* p0, void* p1)
{
    __asm
    {
        mov ecx, p0
        push p1
        call f
    }
}

inline void call_pmfn2(_PMFN f, void* p0, void* p1, int p2)
{
    __asm
    {
        mov ecx, p0
        push p2
        push p1
        call f
    }
}

__declspec(noreturn)
void rethrow_exception(void* data, const _s__ThrowInfo* info)
{
    const _s__CatchableType* t =
        info->pCatchableTypeArray->arrayOfCatchableTypes[0];
    int sz = t->sizeOrOffset;
    _PMFN cp = t->copyFunction;
    bool is_virtual = (t->properties & 0x04) != 0;
    void* e = _alloca(sz);

    if (cp)
    {
        if (is_virtual)
            call_pmfn2(cp, e, data, 1);
        else
            call_pmfn1(cp, e, data);
    }
    else
        std::memcpy(e, data, sz);

    _CxxThrowException(e, info);

    throw;
}


#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>

#pragma warning(push)
#pragma warning(disable : 4251)
#include <boost/thread.hpp>
#pragma warning(pop)

class my_thread
{
public:
#pragma warning(push)
#pragma warning(disable : 4355)
    my_thread() : t_(boost::bind(&my_thread::work, this))
    {
    }
#pragma warning(pop)

    ~my_thread()
    {
        if (e_.get())
        {
            if (_PMFN d = ti_->pmfnUnwind)
                call_pmfn0(d, e_.get());
        }
    }

    void wait()
    {
        t_.join();
        if (e_.get())
            rethrow_exception(e_.get(), ti_);
    }

private:
    boost::scoped_array<char> e_;
    _s__ThrowInfo* ti_;
    boost::thread t_;

    void work()
    {
        try
        {
            throw std::runtime_error("error!");
        }
        catch (...)
        {
            const ::EXCEPTION_RECORD* er = get_current_exception_record();
            if (er)
            {
                if (er->ExceptionCode == cxx_exception_code)
                {
                    void* data =
                        reinterpret_cast<void*>(er->ExceptionInformation[1]);

                    ti_ =
                        reinterpret_cast<_s__ThrowInfo*>(
                            er->ExceptionInformation[2]
                        );

                    const _s__CatchableType* t =
                        ti_->pCatchableTypeArray->arrayOfCatchableTypes[0];

                    boost::scoped_array<char> e(new char[t->sizeOrOffset]);
                    if (t->copyFunction)
                    {
                        if ((t->properties & 0x04) != 0)
                            call_pmfn2(t->copyFunction, e.get(), data, 1);
                        else
                            call_pmfn1(t->copyFunction, e.get(), data);
                    }
                    else
                        std::memcpy(e.get(), data, t->sizeOrOffset);

                    e_.swap(e);
                }
            }
        }
    }
};
#else
    #error "Sorry, unsupported compiler"
#endif

void work()
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

struct my_exception{};

int main()
{
    try
    {
        try
        {
            throw my_exception();
        }
        catch (...)
        {
            work();
            throw;
        }
    }
    catch (const my_exception&)
    {
        std::cout << "OK" << std::endl;
    }
    catch (...)
    {
        std::cout << "NG" << std::endl;
    }
}
