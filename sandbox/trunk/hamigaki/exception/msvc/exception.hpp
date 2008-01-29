// exception.hpp: Hamigaki.Exception for Microsoft Visual C++

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_EXCEPTION_MSVC_EXCEPTION_HPP
#define HAMIGAKI_EXCEPTION_MSVC_EXCEPTION_HPP

#include <boost/shared_array.hpp>
#include <cstring>
#include <malloc.h>

extern "C"
{

struct _tiddata;
_tiddata* __cdecl _getptd();

struct _EXCEPTION_RECORD;

} // extern "C"

namespace hamigaki
{

namespace eh_detail
{

const unsigned long cxx_exception_code = 0xE06D7363;

struct exception_record
{
    unsigned long code;
    unsigned long flags;
    ::_EXCEPTION_RECORD* prev;
    void* address;
    unsigned long reserved;
    unsigned long magic;
    void* object;
    const ::_s__ThrowInfo* info;
};

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

class exception_ptr
{
public:
    exception_ptr() : info_(0)
    {
    }

    exception_ptr(void* data, const ::_s__ThrowInfo* info) : info_(info)
    {
        const ::_s__CatchableType* t =
            info->pCatchableTypeArray->arrayOfCatchableTypes[0];

        data_.reset(new char[t->sizeOrOffset]);
        if (t->copyFunction)
        {
            if ((t->properties & 0x04) != 0)
                call_pmfn2(t->copyFunction, data_.get(), data, 1);
            else
                call_pmfn1(t->copyFunction, data_.get(), data);
        }
        else
            std::memcpy(data_.get(), data, t->sizeOrOffset);
    }

    ~exception_ptr()
    {
        // FIXME: use_count() is for debugging
        if (data_.use_count() == 1)
        {
            if (::_PMFN d = info_->pmfnUnwind)
                call_pmfn0(d, data_.get());
        }
    }

    bool operator==(const exception_ptr& rhs) const
    {
        return data_ == rhs.data_;
    }

    bool operator!=(const exception_ptr& rhs) const
    {
        return !(*this == rhs);
    }

    void rethrow()
    {
        const ::_s__CatchableType* t =
            info_->pCatchableTypeArray->arrayOfCatchableTypes[0];
        int sz = t->sizeOrOffset;
        ::_PMFN cp = t->copyFunction;
        bool is_virtual = (t->properties & 0x04) != 0;
        void* data = data_.get();
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

        ::_CxxThrowException(e, info_);

        throw;
    }

private:
    boost::shared_array<char> data_;
    const ::_s__ThrowInfo* info_;
};

inline const ::_EXCEPTION_RECORD* get_current_exception_record()
{
    ::_tiddata* p = ::_getptd();

#if _MSC_VER < 1400
    return reinterpret_cast< ::_EXCEPTION_RECORD**>(p)[31];
#else
    return reinterpret_cast< ::_EXCEPTION_RECORD**>(p)[34];
#endif
}

} // namespace eh_detail

typedef eh_detail::exception_ptr exception_ptr;

inline exception_ptr current_exception()
{
    const ::_EXCEPTION_RECORD* er = eh_detail::get_current_exception_record();
    if (er)
    {
        const eh_detail::exception_record* p
            = reinterpret_cast<const eh_detail::exception_record*>(er);

        if (p->code == eh_detail::cxx_exception_code)
            return exception_ptr(p->object, p->info);
    }
    return exception_ptr();
}

inline void rethrow_exception(exception_ptr p)
{
    p.rethrow();
}

template<class E>
inline exception_ptr copy_exception(E e)
{
    try
    {
        throw e;
    }
    catch (...)
    {
        return current_exception();
    }
    return exception_ptr();
}

} // namespace hamigaki

#endif // HAMIGAKI_EXCEPTION_MSVC_EXCEPTION_HPP
