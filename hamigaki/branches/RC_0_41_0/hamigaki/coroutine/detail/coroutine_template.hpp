// coroutine_template.hpp: coroutine<R,T1,T2,...,Tn>

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#define HAMIGAKI_COROUTINE_NUM_ARGS BOOST_PP_ITERATION()

#define HAMIGAKI_COROUTINE_TEMPLATE_PARMS \
    BOOST_PP_ENUM_PARAMS(HAMIGAKI_COROUTINE_NUM_ARGS, typename T)

#define HAMIGAKI_COROUTINE_TEMPLATE_ARGS \
    BOOST_PP_ENUM_PARAMS(HAMIGAKI_COROUTINE_NUM_ARGS, T)

#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
    #define HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN void
#else
    #define HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN HAMIGAKI_COROUTINE_TEMPLATE_ARGS
#endif

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

#define HAMIGAKI_COROUTINE_SH_COROUTINE \
    BOOST_JOIN(shared_coroutine,HAMIGAKI_COROUTINE_NUM_ARGS)

#define HAMIGAKI_COROUTINE_DATA \
    BOOST_JOIN(coroutine_data,HAMIGAKI_COROUTINE_NUM_ARGS)

#define HAMIGAKI_COROUTINE_BASE \
    BOOST_JOIN(coroutine_base,HAMIGAKI_COROUTINE_NUM_ARGS)

#define HAMIGAKI_COROUTINE_FUNCTION \
    BOOST_JOIN(function,BOOST_PP_INC(HAMIGAKI_COROUTINE_NUM_ARGS))

#define HAMIGAKI_COROUTINE_IS_COROUTINE \
    BOOST_JOIN(is_coroutine,HAMIGAKI_COROUTINE_NUM_ARGS)

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
    class ContextImpl>
class HAMIGAKI_COROUTINE_COROUTINE;

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
class HAMIGAKI_COROUTINE_SH_COROUTINE;

namespace detail
{

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class Pointer>
class HAMIGAKI_COROUTINE_BASE;

template<class T>
struct HAMIGAKI_COROUTINE_IS_COROUTINE : boost::mpl::false_ {};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class Pointer>
struct HAMIGAKI_COROUTINE_IS_COROUTINE<
    HAMIGAKI_COROUTINE_BASE<
        R HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        Pointer
    >
> : boost::mpl::true_ {};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
struct HAMIGAKI_COROUTINE_IS_COROUTINE<
    HAMIGAKI_COROUTINE_COROUTINE<
        R HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        ContextImpl
    >
> : boost::mpl::true_ {};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
struct HAMIGAKI_COROUTINE_IS_COROUTINE<
    HAMIGAKI_COROUTINE_SH_COROUTINE<
        R HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        ContextImpl
    >
> : boost::mpl::true_ {};

template<class Signature, class ContextImpl>
struct HAMIGAKI_COROUTINE_IS_COROUTINE<
    coroutine<Signature,ContextImpl>
> : boost::mpl::true_ {};

template<class Signature, class ContextImpl>
struct HAMIGAKI_COROUTINE_IS_COROUTINE<
    shared_coroutine<Signature,ContextImpl>
> : boost::mpl::true_ {};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
class HAMIGAKI_COROUTINE_DATA : private boost::noncopyable
{
private:
    class self_with_result;
    class self_no_result;
    friend class self_with_result;
    friend class self_no_result;

    class self_with_result
    {
    public:
        self_with_result(HAMIGAKI_COROUTINE_DATA* c) : coro_(c) {}

        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield(R r)
        {
            (*coro_->result_ptr_) = r;
            swap_context(
                coro_->callee_, coro_->caller_, detail::default_hint());
            if (coro_->state_ == coro_detail::exiting)
                throw exit_exception();
            return coro_->arguments();
        }

        #define HAMIGAKI_COROUTINE_YIELD_GEN(Z,N,D) \
        template<BOOST_PP_ENUM_PARAMS(N,typename TT)> \
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield( \
            BOOST_PP_ENUM_BINARY_PARAMS(N,TT,a) \
        ) \
        { \
            return this->yield(R(BOOST_PP_ENUM_PARAMS(N,a))); \
        }
        /**/

        BOOST_PP_REPEAT_FROM_TO(
            2, BOOST_FUNCTION_MAX_ARGS, HAMIGAKI_COROUTINE_YIELD_GEN, _)

        #undef HAMIGAKI_COROUTINE_YIELD_GEN

        template<class Coroutine>
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to(Coroutine& c)
        {
            return this->yield_to_impl(c);
        }

        template<class Coroutine, class TT1>
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to(Coroutine& c, TT1 a1)
        {
            c.pimpl_->arg_ = a1;
            return this->yield_to_impl(c);
        }

        #define HAMIGAKI_COROUTINE_YIELD_TO_GEN(Z,N,D) \
        template<class Coroutine, BOOST_PP_ENUM_PARAMS(N,typename TT)> \
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to( \
            Coroutine& c, \
            BOOST_PP_ENUM_BINARY_PARAMS(N,TT,a) \
        ) \
        { \
            c.pimpl_->arg_ = boost::make_tuple(BOOST_PP_ENUM_PARAMS(N,a)); \
            return this->yield_to_impl(c); \
        }
        /**/

        BOOST_PP_REPEAT_FROM_TO(
            2, BOOST_FUNCTION_MAX_ARGS, HAMIGAKI_COROUTINE_YIELD_TO_GEN, _)

        #undef HAMIGAKI_COROUTINE_YIELD_TO_GEN

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

        R result()
        {
            return coro_->result_.get();
        }

    private:
        HAMIGAKI_COROUTINE_DATA* coro_;

        template<class Coroutine>
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to_impl(Coroutine& c)
        {
            c.pimpl_->result_ptr_ = coro_->result_ptr_;

            std::swap(coro_->caller_, c.pimpl_->caller_);
            swap_context(
                coro_->callee_, c.pimpl_->callee_, detail::default_hint());
            if (coro_->state_ == coro_detail::exiting)
                throw exit_exception();

#if HAMIGAKI_COROUTINE_NUM_ARGS != 0
            return coro_->arg_;
#endif
        }
    };

    class self_no_result
    {
    public:
        self_no_result(HAMIGAKI_COROUTINE_DATA* c) : coro_(c) {}

        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield()
        {
            swap_context(
                coro_->callee_, coro_->caller_, detail::default_hint());
            if (coro_->state_ == coro_detail::exiting)
                throw exit_exception();
            return coro_->arguments();
        }

        template<class Coroutine>
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to(Coroutine& c)
        {
            return this->yield_to_impl(c);
        }

        template<class Coroutine, class TT1>
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to(Coroutine& c, TT1 a1)
        {
            c.pimpl_->arg_ = a1;
            return this->yield_to_impl(c);
        }

        #define HAMIGAKI_COROUTINE_YIELD_TO_GEN(Z,N,D) \
        template<class Coroutine, BOOST_PP_ENUM_PARAMS(N,typename TT)> \
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to( \
            Coroutine& c, \
            BOOST_PP_ENUM_BINARY_PARAMS(N,TT,a) \
        ) \
        { \
            c.pimpl_->arg_ = boost::make_tuple(BOOST_PP_ENUM_PARAMS(N,a)); \
            return this->yield_to_impl(c); \
        }
        /**/

        BOOST_PP_REPEAT_FROM_TO(
            2, BOOST_FUNCTION_MAX_ARGS, HAMIGAKI_COROUTINE_YIELD_TO_GEN, _)

        #undef HAMIGAKI_COROUTINE_YIELD_TO_GEN

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

        void result() {}

    private:
        HAMIGAKI_COROUTINE_DATA* coro_;

        template<class Coroutine>
        HAMIGAKI_COROUTINE_CORO_ARG_TYPE yield_to_impl(Coroutine& c)
        {
            std::swap(coro_->caller_, c.pimpl_->caller_);
            swap_context(
                coro_->callee_, c.pimpl_->callee_, detail::default_hint());
            if (coro_->state_ == coro_detail::exiting)
                throw exit_exception();

#if HAMIGAKI_COROUTINE_NUM_ARGS != 0
            return coro_->arg_;
#endif
        }
    };

public:
    typedef typename boost::mpl::if_<
        boost::is_same<R, void>,
        self_no_result,
        self_with_result
    >::type self;

    typedef typename boost::mpl::if_<
        boost::is_same<R, void>,
        void,
        boost::optional<R>
    >::type optional_result_type;

    typedef typename boost::mpl::if_<
        boost::is_same<R, void>,
        boost::blank,
        boost::optional<R>
    >::type result_storage_type;

    template<class Functor>
    HAMIGAKI_COROUTINE_DATA(Functor func, std::ptrdiff_t stack_size)
        : func_(func), func_storage_(this), state_(coro_detail::normal)
        , abnormal_exit_(false), type_ptr_(0)
        , callee_(func_storage_, stack_size)
    {
    }

    ~HAMIGAKI_COROUTINE_DATA()
    {
        exit();
    }

    boost::HAMIGAKI_COROUTINE_FUNCTION<
        R,self& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS> func() const
    {
        return func_;
    }

    void exit()
    {
        if (state_ != coro_detail::exited)
        {
            state_ = coro_detail::exiting;
            swap_context(caller_, callee_, detail::default_hint());
        }
    }

    bool exited() const
    {
        return state_ == coro_detail::exited;
    }

    R call(
        const boost::false_type& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_PARMS)
    {
        if (state_ == coro_detail::exited)
            error();
        this->call_impl(HAMIGAKI_COROUTINE_ARGS);
        return *result_;
    }

    void call(
        const boost::true_type& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_PARMS)
    {
        if (state_ == coro_detail::exited)
            error();
        this->call_impl(HAMIGAKI_COROUTINE_ARGS);
    }

    optional_result_type call_nothrow(
        const boost::false_type& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_PARMS)
    {
        if (state_ != coro_detail::exited)
            this->call_nothrow_impl(HAMIGAKI_COROUTINE_ARGS);
        if (state_ == coro_detail::exited)
            result_ = boost::none;
        return result_;
    }

    void call_nothrow(
        const boost::true_type& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_PARMS)
    {
        this->call_nothrow_impl(HAMIGAKI_COROUTINE_ARGS);
    }

private:
    class functor;
    friend class functor;
    class functor
    {
    public:
        explicit functor(HAMIGAKI_COROUTINE_DATA* c) : coro_(c)
        {
        }

        void operator()() const
        {
            coro_->startup(boost::is_same<R,void>());
            coro_->cleanup();
        }

    private:
        HAMIGAKI_COROUTINE_DATA* coro_;
    };

    result_storage_type result_;
    result_storage_type* result_ptr_;
#if HAMIGAKI_COROUTINE_NUM_ARGS != 0
    HAMIGAKI_COROUTINE_CORO_ARG_TYPE arg_;
#endif
    boost::HAMIGAKI_COROUTINE_FUNCTION<
        R,self& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS> func_;
    functor func_storage_;
    coro_detail::state state_;
    bool abnormal_exit_;
    const std::type_info* type_ptr_;
    typename ContextImpl::context_impl_base caller_;
    ContextImpl callee_;

    void startup(const boost::false_type&)
    {
        if (state_ == coro_detail::exiting)
            return;

        self self(this);
        try
        {
#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
            (*result_ptr_) = func_(self);
#elif HAMIGAKI_COROUTINE_NUM_ARGS == 1
            (*result_ptr_) = func_(self, arg_);
#else
            (*result_ptr_) = func_(self, HAMIGAKI_COROUTINE_GET_ARGS);
#endif

            swap_context(callee_, caller_, detail::default_hint());
        }
        catch (const exit_exception&)
        {
        }
        catch (const std::exception& e)
        {
            abnormal_exit_ = true;
            type_ptr_ = &typeid(e);
        }
        catch (...)
        {
            abnormal_exit_ = true;
        }
        result_ = boost::none;
    }

    void startup(const boost::true_type&)
    {
        if (state_ == coro_detail::exiting)
            return;

        self self(this);
        try
        {
#if HAMIGAKI_COROUTINE_NUM_ARGS == 0
            func_(self);
#elif HAMIGAKI_COROUTINE_NUM_ARGS == 1
            func_(self, arg_);
#else
            func_(self, HAMIGAKI_COROUTINE_GET_ARGS);
#endif

            swap_context(callee_, caller_, detail::default_hint());
        }
        catch (const exit_exception&)
        {
        }
        catch (const std::exception& e)
        {
            abnormal_exit_ = true;
            type_ptr_ = &typeid(e);
        }
        catch (...)
        {
            abnormal_exit_ = true;
        }
    }

    void cleanup()
    {
        state_ = coro_detail::exited;
        swap_context(callee_, caller_, detail::default_hint());
    }

    void call_nothrow_impl(HAMIGAKI_COROUTINE_PARMS)
    {
        result_ptr_ = &result_;

#if HAMIGAKI_COROUTINE_NUM_ARGS == 1
        arg_ = HAMIGAKI_COROUTINE_ARGS;
#elif HAMIGAKI_COROUTINE_NUM_ARGS != 0
        arg_ = boost::make_tuple(HAMIGAKI_COROUTINE_ARGS);
#endif

        swap_context(caller_, callee_, detail::default_hint());
    }

    void call_impl(HAMIGAKI_COROUTINE_PARMS)
    {
        this->call_nothrow_impl(HAMIGAKI_COROUTINE_ARGS);
        if (state_ == coro_detail::exited)
            error();
    }

    HAMIGAKI_COROUTINE_CORO_ARG_TYPE arguments()
    {
#if HAMIGAKI_COROUTINE_NUM_ARGS != 0
        return arg_;
#endif
    }

    void error()
    {
        if (abnormal_exit_)
            throw abnormal_exit(type_ptr_);
        else
            throw coroutine_exited();
    }
};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class Pointer>
class HAMIGAKI_COROUTINE_BASE
{
private:
    typedef typename Pointer::element_type data_type;
    typedef typename data_type::optional_result_type optional_result_type;

public:
    typedef R result_type;
    typedef typename data_type::self self;

    HAMIGAKI_COROUTINE_BASE() {}

    template<class Functor>
    HAMIGAKI_COROUTINE_BASE(Functor func, std::ptrdiff_t stack_size)
        : pimpl_(new data_type(func, stack_size))
    {
    }

    void restart(std::ptrdiff_t stack_size = -1)
    {
        BOOST_ASSERT(pimpl_.get());
        Pointer tmp(new data_type(pimpl_->func(), stack_size));
        pimpl_ = tmp;
    }

    R operator()(HAMIGAKI_COROUTINE_PARMS) const
    {
        BOOST_ASSERT(pimpl_.get());
        return pimpl_->call(
            boost::is_same<R,void>() HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_ARGS
        );
    }

    optional_result_type operator()(
        const std::nothrow_t& HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_PARMS) const
    {
        BOOST_ASSERT(pimpl_.get());
        return pimpl_->call_nothrow(
            boost::is_same<R,void>() HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_ARGS);
    }

    void exit()
    {
        BOOST_ASSERT(pimpl_.get());
        pimpl_->exit();
    }

    bool exited() const
    {
        BOOST_ASSERT(pimpl_.get());
        return pimpl_->exited();
    }

    bool empty() const
    {
        return pimpl_.get() == 0;
    }

// TODO
//protected:
    Pointer pimpl_;
};

} // namespace detail

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
class HAMIGAKI_COROUTINE_SH_COROUTINE;

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
class HAMIGAKI_COROUTINE_COROUTINE
    : public detail::HAMIGAKI_COROUTINE_BASE<
        R HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        std::auto_ptr<
            detail::HAMIGAKI_COROUTINE_DATA<
                R HAMIGAKI_COROUTINE_COMMA
                HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                ContextImpl
            >
        >
    >
{
    friend class HAMIGAKI_COROUTINE_SH_COROUTINE<
        R HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        ContextImpl>;

public:
    HAMIGAKI_COROUTINE_COROUTINE() {}

    HAMIGAKI_COROUTINE_COROUTINE(
        coroutine<R (HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN), ContextImpl>& x
    )
        : detail::HAMIGAKI_COROUTINE_BASE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            std::auto_ptr<
                detail::HAMIGAKI_COROUTINE_DATA<
                    R HAMIGAKI_COROUTINE_COMMA
                    HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                    ContextImpl
                >
            >
        >(x)
    {
    }

    template<class Functor>
    HAMIGAKI_COROUTINE_COROUTINE(Functor func, std::ptrdiff_t stack_size=-1)
        : detail::HAMIGAKI_COROUTINE_BASE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            std::auto_ptr<
                detail::HAMIGAKI_COROUTINE_DATA<
                    R HAMIGAKI_COROUTINE_COMMA
                    HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                    ContextImpl
                >
            >
        >(func, stack_size)
    {
    }

private:
    std::auto_ptr<
        detail::HAMIGAKI_COROUTINE_DATA<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl
        >
    >&
    get_ptr()
    {
        return this->pimpl_;
    }
};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl>
class HAMIGAKI_COROUTINE_SH_COROUTINE
    : public detail::HAMIGAKI_COROUTINE_BASE<
        R HAMIGAKI_COROUTINE_COMMA
        HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        boost::shared_ptr<
            detail::HAMIGAKI_COROUTINE_DATA<
                R HAMIGAKI_COROUTINE_COMMA
                HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                ContextImpl
            >
        >
    >
{
public:
    HAMIGAKI_COROUTINE_SH_COROUTINE()
    {
    }

    HAMIGAKI_COROUTINE_SH_COROUTINE(
        shared_coroutine<R(HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN), ContextImpl>& x
    )
        : detail::HAMIGAKI_COROUTINE_BASE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            boost::shared_ptr<
                detail::HAMIGAKI_COROUTINE_DATA<
                    R HAMIGAKI_COROUTINE_COMMA
                    HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                    ContextImpl
                >
            >
        >(x)
    {
    }

    explicit HAMIGAKI_COROUTINE_SH_COROUTINE(
        HAMIGAKI_COROUTINE_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl>& x
    )
    {
        this->pimpl_ = x.get_ptr();
    }

    explicit HAMIGAKI_COROUTINE_SH_COROUTINE(
        coroutine<R (HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN), ContextImpl>& x
    )
    {
        this->pimpl_ = x.get_ptr();
    }

    template<class Functor>
    HAMIGAKI_COROUTINE_SH_COROUTINE(Functor func, std::ptrdiff_t stack_size=-1)
        : detail::HAMIGAKI_COROUTINE_BASE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            boost::shared_ptr<
                detail::HAMIGAKI_COROUTINE_DATA<
                    R HAMIGAKI_COROUTINE_COMMA
                    HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                    ContextImpl
                >
            >
        >(func, stack_size)
    {
    }

    HAMIGAKI_COROUTINE_SH_COROUTINE&
    operator=(
        HAMIGAKI_COROUTINE_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl>& x
    )
    {
        this->pimpl_ = x.get_ptr();
        return *this;
    }
};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl
>
class coroutine<R(HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN), ContextImpl>
    : public HAMIGAKI_COROUTINE_COROUTINE<
        R HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        ContextImpl>
{
private:
    typedef
        HAMIGAKI_COROUTINE_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl
        > base_type;

public:
    coroutine()
    {
    }

    coroutine(
        HAMIGAKI_COROUTINE_COROUTINE<
                R HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
                ContextImpl>& x
    )
        : base_type(x)
    {
    }

    template<class Functor>
    coroutine(Functor func, std::ptrdiff_t stack_size=-1)
        : base_type(func, stack_size)
    {
    }
};

template<class R HAMIGAKI_COROUTINE_COMMA
    HAMIGAKI_COROUTINE_TEMPLATE_PARMS,
    class ContextImpl
>
class shared_coroutine<R(HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN), ContextImpl>
    : public HAMIGAKI_COROUTINE_SH_COROUTINE<
        R HAMIGAKI_COROUTINE_COMMA HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
        ContextImpl>
{
private:
    typedef
        HAMIGAKI_COROUTINE_SH_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl
        > base_type;

public:
    shared_coroutine() {}

    shared_coroutine(const shared_coroutine& x)
    {
        base_type::operator=(x);
    }

    template<class Functor>
    shared_coroutine(Functor func, std::ptrdiff_t stack_size=-1)
        : base_type(func, stack_size)
    {
    }

    shared_coroutine(
        const HAMIGAKI_COROUTINE_SH_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl
        >& x)
        : base_type(x)
    {
    }

    shared_coroutine(
        HAMIGAKI_COROUTINE_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl
        >& x)
        : base_type(x)
    {
    }

    shared_coroutine(
        coroutine<R(HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN), ContextImpl>& x
    )
        : base_type(x)
    {
    }

    shared_coroutine& operator=(
        HAMIGAKI_COROUTINE_COROUTINE<
            R HAMIGAKI_COROUTINE_COMMA
            HAMIGAKI_COROUTINE_TEMPLATE_ARGS,
            ContextImpl
        >& x)
    {
        base_type::operator=(x);
        return *this;
    }
};

} } // End namespaces coroutines, hamigaki.

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#undef HAMIGAKI_COROUTINE_NUM_ARGS
#undef HAMIGAKI_COROUTINE_TEMPLATE_PARMS
#undef HAMIGAKI_COROUTINE_TEMPLATE_ARGS
#undef HAMIGAKI_COROUTINE_TMP_ARGS_FOR_FUN
#undef HAMIGAKI_COROUTINE_PARM
#undef HAMIGAKI_COROUTINE_PARMS
#undef HAMIGAKI_COROUTINE_ARGS
#undef HAMIGAKI_COROUTINE_COMMA
#undef HAMIGAKI_COROUTINE_COROUTINE
#undef HAMIGAKI_COROUTINE_SH_COROUTINE
#undef HAMIGAKI_COROUTINE_DATA
#undef HAMIGAKI_COROUTINE_BASE
#undef HAMIGAKI_COROUTINE_FUNCTION
#undef HAMIGAKI_COROUTINE_IS_COROUTINE
#undef HAMIGAKI_COROUTINE_CORO_ARG_TYPE
#undef HAMIGAKI_COROUTINE_GET_ARG
#undef HAMIGAKI_COROUTINE_GET_ARGS
