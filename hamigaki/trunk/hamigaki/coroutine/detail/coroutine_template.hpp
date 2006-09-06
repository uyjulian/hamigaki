//  coroutine_template.hpp: coroutine<R,T1,T2,...,Tn>

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#define HAMIGAKI_COROUTINE_NUM_ARGS BOOST_PP_ITERATION()

#define HAMIGAKI_COROUTINE_TEMPLATE_PARMS \
    BOOST_PP_ENUM_PARAMS(HAMIGAKI_COROUTINE_NUM_ARGS, typename T)

#define HAMIGAKI_COROUTINE_TEMPLATE_ARGS \
    BOOST_PP_ENUM_PARAMS(HAMIGAKI_COROUTINE_NUM_ARGS, T)

#define HAMIGAKI_COROUTINE_PARM(J,I,D) BOOST_PP_CAT(T,I) BOOST_PP_CAT(a,I)

#define HAMIGAKI_COROUTINE_PARMS \
    BOOST_PP_ENUM( \
        HAMIGAKI_COROUTINE_NUM_ARGS,HAMIGAKI_COROUTINE_PARM,BOOST_PP_EMPTY)

#define HAMIGAKI_COROUTINE_ARGS \
    BOOST_PP_ENUM_PARAMS(HAMIGAKI_COROUTINE_NUM_ARGS, a)

#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
    #define HAMIGAKI_COROUTINE_COMMA
#else
    #define HAMIGAKI_COROUTINE_COMMA ,
#endif

#define HAMIGAKI_COROUTINE_COROUTINE \
    BOOST_JOIN(coroutine,HAMIGAKI_COROUTINE_NUM_ARGS)

#define HAMIGAKI_COROUTINE_FUNCTION \
    BOOST_JOIN(function,BOOST_PP_INC(HAMIGAKI_COROUTINE_NUM_ARGS))

#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
    #define HAMIGAKI_COROUTINE_CORO_ARG_TYPE void
#elif HAMIGAKI_COROUTINE_NUM_ARGS == 1
    #define HAMIGAKI_COROUTINE_CORO_ARG_TYPE HAMIGAKI_COROUTINE_TEMPLATE_ARGS
#else
    #define HAMIGAKI_COROUTINE_CORO_ARG_TYPE \
        boost::tuple<HAMIGAKI_COROUTINE_TEMPLATE_ARGS>
#endif

#define HAMIGAKI_COROUTINE_GET_ARG(J,I,D) boost::get<I>(arg_)

#define HAMIGAKI_COROUTINE_GET_ARGS \
    BOOST_PP_ENUM( \
        HAMIGAKI_COROUTINE_NUM_ARGS,HAMIGAKI_COROUTINE_GET_ARG,BOOST_PP_EMPTY)

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4355)
#endif

namespace hamigaki { namespace coroutines {

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl=detail::default_context_impl>
class HAMIGAKI_COROUTINE_COROUTINE : private boost::noncopyable
{
public:
    class self;
    friend class self;

    class self
    {
    public:
        self(HAMIGAKI_COROUTINE_COROUTINE* c) : coro_(c) {}

        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield(R r)
        {
            coro_->result_ = r;
            swap_context(
                coro_->callee_, coro_->caller_, detail::default_hint());
            if (coro_->state_ == coro_detail::exiting)
                throw exit_exception();
#if HAMIGAKI_COROUTINE_NUM_ARGS != 0
            return coro_->arg_;
#endif
        }

#if defined(_MSC_VER) || defined(__BORLANDC__)
        __declspec(noreturn)
#endif
        void exit()
#if defined(__GNUC__)
        __attribute__((noreturn))
#endif
        {
            throw exit_exception();
        }

    private:
        HAMIGAKI_COROUTINE_COROUTINE* coro_;
    };

    template<class Functor>
    HAMIGAKI_COROUTINE_COROUTINE(Functor func, std::ptrdiff_t stack_size=-1)
        : func_(func), func_storage_(this), state_(coro_detail::normal)
        , callee_(func_storage_, stack_size)
    {
    }

    ~HAMIGAKI_COROUTINE_COROUTINE()
    {
        if (state_ != coro_detail::exited)
        {
            state_ = coro_detail::exiting;
            swap_context(caller_, callee_, detail::default_hint());
        }
    }

    R operator()(HAMIGAKI_COROUTINE_PARMS)
    {
#if HAMIGAKI_COROUTINE_NUM_ARGS == 1
        arg_ = HAMIGAKI_COROUTINE_ARGS;
#elif HAMIGAKI_COROUTINE_NUM_ARGS != 0
        arg_ = boost::make_tuple(HAMIGAKI_COROUTINE_ARGS);
#endif
        swap_context(caller_, callee_, detail::default_hint());
        if (state_ == coro_detail::exited)
            throw coroutine_exited();
        return *result_;
    }

    boost::optional<R> operator()(
        const std::nothrow_t& HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_PARMS)
    {
#if HAMIGAKI_COROUTINE_NUM_ARGS == 1
        arg_ = HAMIGAKI_COROUTINE_ARGS;
#elif HAMIGAKI_COROUTINE_NUM_ARGS != 0
        arg_ = boost::make_tuple(HAMIGAKI_COROUTINE_ARGS);
#endif
        swap_context(caller_, callee_, detail::default_hint());
        return result_;
    }

    bool exited() const
    {
        return state_ == coro_detail::exited;
    }

private:
    class functor;
    friend class functor;
    class functor
    {
    public:
        explicit functor(HAMIGAKI_COROUTINE_COROUTINE* c) : coro_(c)
        {
        }

        void operator()() const
        {
            coro_->startup();
        }

    private:
        HAMIGAKI_COROUTINE_COROUTINE* coro_;
    };

    boost::optional<R> result_;
#if HAMIGAKI_COROUTINE_NUM_ARGS != 0
    HAMIGAKI_COROUTINE_CORO_ARG_TYPE arg_;
#endif
    boost::HAMIGAKI_COROUTINE_FUNCTION<
        R,self& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS> func_;
    functor func_storage_;
    coro_detail::state state_;
    typename ContextImpl::context_impl_base caller_;
    ContextImpl callee_;

    void startup()
    {
        self self(this);
        try
        {
#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
            result_ = func_(self);
#elif HAMIGAKI_COROUTINE_NUM_ARGS == 1
            result_ = func_(self, arg_);
#else
            result_ = func_(self, HAMIGAKI_COROUTINE_GET_ARGS);
#endif
            swap_context(callee_, caller_, detail::default_hint());
        }
        catch (...)
        {
        }
        result_ = boost::none;
        state_ = coro_detail::exited;
        swap_context(callee_, caller_, detail::default_hint());
    }
};

template<class R HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_TEMPLATE_PARMS>
#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
class coroutine<R(void)>
#else
class coroutine<R(HAMIGAKI_COROUTINE_TEMPLATE_ARGS)>
#endif
    : public HAMIGAKI_COROUTINE_COROUTINE<
        R HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_TEMPLATE_ARGS>
{
public:
    template<class Functor>
    coroutine(Functor func, std::ptrdiff_t stack_size=-1)
        : HAMIGAKI_COROUTINE_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_TEMPLATE_ARGS
        >(func, stack_size)
    {
    }
};

} } // End namespaces coroutines, hamigaki.

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#undef HAMIGAKI_COROUTINE_NUM_ARGS
#undef HAMIGAKI_COROUTINE_TEMPLATE_PARMS
#undef HAMIGAKI_COROUTINE_TEMPLATE_ARGS
#undef HAMIGAKI_COROUTINE_PARM
#undef HAMIGAKI_COROUTINE_PARMS
#undef HAMIGAKI_COROUTINE_ARGS
#undef HAMIGAKI_COROUTINE_COMMA
#undef HAMIGAKI_COROUTINE_COROUTINE
#undef HAMIGAKI_COROUTINE_FUNCTION
#undef HAMIGAKI_COROUTINE_CORO_ARG_TYPE
#undef HAMIGAKI_COROUTINE_GET_ARG
#undef HAMIGAKI_COROUTINE_GET_ARGS
