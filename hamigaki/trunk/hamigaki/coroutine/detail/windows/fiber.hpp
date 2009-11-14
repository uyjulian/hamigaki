// fiber.hpp: Win32 fiber API declaration

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_WINDOWS_FIBER_HPP
#define HAMIGAKI_COROUTINE_DETAIL_WINDOWS_FIBER_HPP

#include <boost/config.hpp>

#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 13012035) && defined(_M_IX86)
    #define HAMIGAKI_COROUTINE_HAS_READFSDWORD
    extern "C" unsigned long __readfsdword(unsigned long);
    #pragma intrinsic(__readfsdword)
#endif

#if defined(_MSC_VER) && defined(_M_AMD64)
    #define HAMIGAKI_COROUTINE_HAS_READGSQWORD
    extern "C" unsigned long long __readgsqword(unsigned long);
    #pragma intrinsic(__readgsqword)
#endif

#if defined(BOOST_HAS_DECLSPEC) && !defined(HAMIGAKI_COROUTINE_NO_DLLIMPORT)
    #define HAMIGAKI_COROUTINE_DLLIMPORT __declspec(dllimport)
#else
    #define HAMIGAKI_COROUTINE_DLLIMPORT
#endif

extern "C" {

typedef void (__stdcall* start_routine)(void*);

#if !defined(_WIN64)
HAMIGAKI_COROUTINE_DLLIMPORT
void* __stdcall CreateFiber(unsigned long, start_routine, void*);
#else
HAMIGAKI_COROUTINE_DLLIMPORT
void* __stdcall CreateFiber(unsigned long long, start_routine, void*);
#endif

HAMIGAKI_COROUTINE_DLLIMPORT void __stdcall DeleteFiber(void*);
HAMIGAKI_COROUTINE_DLLIMPORT void* __stdcall ConvertThreadToFiber(void*);
HAMIGAKI_COROUTINE_DLLIMPORT void __stdcall SwitchToFiber(void*);

} // extern "C"

namespace hamigaki { namespace coroutines { namespace detail {

namespace windows
{

#if defined(HAMIGAKI_COROUTINE_HAS_READFSDWORD)
inline void* get_current_fiber()
{
    return reinterpret_cast<void*>(
        static_cast< ::uintptr_t>(::__readfsdword(0x10))
    );
}
#elif defined(HAMIGAKI_COROUTINE_HAS_READGSQWORD)
inline void* get_current_fiber()
{
    return reinterpret_cast<void*>(
        static_cast< ::uintptr_t>(::__readgsqword(0x20))
    );
}
#elif (defined(_MSC_VER) || defined(__MWERKS__)) && defined(_M_IX86)
inline void* get_current_fiber()
{
    __asm mov eax, fs:[0x10]
}
#elif defined(__BORLANDC__) && defined(_M_IX86)
inline void* get_current_fiber()
{
    // mov eax, fs:[0x10]
    __emit__(0x64, 0xA1, 0x10, 0x00, 0x00, 0x00);

    return reinterpret_cast<void*>(_EAX);
}
#elif defined(__GNUC__) && defined(__i386__)
inline void* get_current_fiber()
{
    void* result;
    __asm__
    (
        "movl %%fs:0x10,%0"
        : "=r" (result)
    );
    return result;
}
#else
    #error unsupported platform
#endif

inline bool is_fiber()
{
    void* p = get_current_fiber();
    return (p != 0) && (p != reinterpret_cast<void*>(0x00001E00));
}

} // namespace windows

} } } // End namespaces detail, coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_WINDOWS_FIBER_HPP
