/*=============================================================================
#     FileName: sp_counted_base.h
#         Desc: 
#       Author: Jeffrey Pu
#        Email: pujunying@gmail.com
#     HomePage: https://github.com/jfpu
#      Version: 0.0.1
#   LastChange: 2013-12-18 23:53:10
#      History:
=============================================================================*/

#ifndef _SP_COUNTED_BASE_H_
#define _SP_COUNTED_BASE_H_

#include <type_traits>
// #include <tr1/type_traits>
// #include <boost/type_traits.hpp>
#include <debug/macros.h>

// #define _GLIBCXX_DEBUG_ASSERT(_Condition) __glibcxx_assert(_Condition)

namespace jfpu {


// Use C++0x's static_assert if possible.
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#define PB_DS_STATIC_ASSERT(UNIQUE, E)  static_assert(E, #UNIQUE)
#else
    template<bool>
    struct __static_assert;

    template<>
    struct __static_assert<true> { };

    template<int>
    struct __static_assert_dumclass { enum { v = 1 }; };

#define PB_DS_STATIC_ASSERT(UNIQUE, E)  \
    typedef __gnu_pbds::detail::__static_assert_dumclass<sizeof(__gnu_pbds::detail::__static_assert<bool(E)>)> UNIQUE##__static_assert_type

#endif

#if 0
// COMMON CHECKS
#define __glibcxx_function_requires(...)                                 \
            __gnu_cxx::__function_requires< __gnu_cxx::__VA_ARGS__ >();
#endif

struct __static_cast_tag{};
struct __const_cast_tag{};
struct __dynamic_cast_tag{};

/**
  *  @brief  Exception possibly thrown by @c shared_ptr.
  *  @ingroup exceptions
  */
class bad_weak_ptr : public std::exception {
public:
    virtual char const* what() const throw() {
        return "tr1::bad_weak_ptr";
    }
};

// Substitute for bad_weak_ptr object in the case of -fno-exceptions.
inline void
__throw_bad_weak_ptr()
{
#if __EXCEPTIONS
    throw bad_weak_ptr();
#else
    __builtin_abort();
#endif
}



template<typename T>
struct sp_deleter {
    typedef void result_type;
    typedef T* argument_type;
    
    void operator()(T* p) const { delete p; }
};



class sp_counted_base {
    sp_counted_base(sp_counted_base const& );
    sp_counted_base& operator=(sp_counted_base const& );

    // http://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_concurrency.html
    _Atomic_word _uc;
    _Atomic_word _wc;

public:
    sp_counted_base() : _uc(1), _wc(1) {}
    virtual ~sp_counted_base() {};
    virtual void dispose() = 0;
    virtual void destroy() { delete this;}
    virtual void* get_deleter(const std::type_info&) = 0;
    
    void add_ref_copy() {
        // ++_uc;
         __gnu_cxx::__atomic_add_dispatch(&_uc, 1);
    }

    // single, mutex, atomic
    void add_ref_lock();

    void release() {
        // if(0 == --_uc) {
        if(__gnu_cxx::__exchange_and_add_dispatch(&_uc, -1) == 1) {
            dispose();
            weak_release();
        }
    }

    void weak_add_ref() {
        // ++_wc;
        __gnu_cxx::__atomic_add_dispatch(&_wc, 1);
    }

    void weak_release() {
        // if(0 == --_wc) {
        if(__gnu_cxx::__exchange_and_add_dispatch(&_wc, -1) == 1) {
            destroy();
        }
    }

    long use_count() const {
        return const_cast<const volatile _Atomic_word&>(_uc);
    }
};

inline void sp_counted_base::add_ref_lock() {
    // Perform lock-free add-if-not-zero operation.
    _Atomic_word __count;

    do {
        __count = _uc;
        if(0 == __count)
            __throw_bad_weak_ptr();
        
        // Replace the current counter value with the old value + 1, as
        // long as it's not changed meanwhile. 
	} while (!__sync_bool_compare_and_swap(&_uc, __count, __count + 1));
}



template<typename Ptr, typename Deleter>
class sp_counted_impl : public sp_counted_base {
    // typedef sp_counted_impl<Ptr, Deleter> this_type;
    Ptr _px;
    Deleter _del;
    
    sp_counted_impl(sp_counted_impl const& );
    sp_counted_impl& operator=(sp_counted_impl const& );

public:
    explicit sp_counted_impl(Ptr px, Deleter del)
      : _px(px), _del(del) {}
    void dispose() {
	    _del(_px);
    }
    
    void* get_deleter(const std::type_info& ti) {
#ifdef _GXX_RTTI
        return ti == typeid(Deleter) ? _del : NULL;
#else
        return NULL;
#endif
    }
    #if 0
    void* operator new(std::size_t ) {
        return std::allocator<this_type>().allocate(1, static_cast<this_type*>(0));
    }
    
    void operator delete(void* p) {
        std::allocator<this_type>().deallocate(static_cast<this_type*>(p), 1);
    }
    #endif
};



class weak_count;

class shared_count {
    sp_counted_base* _pi;
    
    friend class weak_count;

public:
    shared_count() : _pi(NULL) {}
    ~shared_count() {
        if(NULL != _pi) _pi->release();
    }
    
    shared_count(shared_count const& r) : _pi(r._pi) {
        if(NULL != _pi)
            _pi->add_ref_copy();
    }

    // g++ -o use_shared_ptr -std=c++0x use_shared_ptr.cc
    // remove_pointer included in <type_traits>
    // http://en.cppreference.com/w/cpp/types/remove_pointer
    template<typename Ptr>
    explicit shared_count(Ptr p) : _pi(NULL) {
        typedef typename std::remove_pointer<Ptr>::type native_type;
        // typedef typename std::tr1::remove_pointer<Ptr>::type native_type;
        // typedef typename boost::remove_pointer<Ptr>::type native_type;
        try {
            _pi = new sp_counted_impl<Ptr, sp_deleter<native_type> >(p, sp_deleter<native_type>());
        } catch(...) {
            delete p;
            throw;
        }
    }
    
    template<typename Ptr, typename Deleter>
    shared_count(Ptr p, Deleter d) : _pi(NULL) {
        try {
            _pi = new sp_counted_impl<Ptr, Deleter>(p, d);
        } catch(...) {
            d(p);
            throw;
        }
    }
    
    // template<class P, class D, class A> shared_count( P p, D d, A a )
    #if 0
    template<typename T>
    explicit shared_count(std::auto_ptr<T>& r)
      : _pi(new sp_counted_impl<T*, sp_deleter<T> >(r.get(), sp_deleter<T>())){
        r.release();
    }
    #endif
    // explicit shared_count( std::unique_ptr<Y, D> & r ): pi_( 0 )
    shared_count(weak_count const& r);
    
    #if 0
    shared_count(weak_count const& r, sp_nothrow_tag);
    #endif
    shared_count& operator=(shared_count const& r) {
        sp_counted_base* tmp = r._pi;
        if(_pi != tmp) {
            if(NULL != tmp) tmp->add_ref_copy();
            if(NULL != _pi) _pi->release();
            _pi = tmp;
        }
        return *this;
    }
    
    friend inline bool operator==(shared_count const& a, shared_count const& b) {
        return a._pi == b._pi;
    }

    friend inline bool operator<(shared_count const& a, shared_count const& b) {
        return a._pi < b._pi;
    }
    
    void swap(shared_count& r) {
        sp_counted_base* tmp = r._pi;
        r._pi = _pi;
        _pi = tmp;
    }
    
    long use_count() const {
        return (NULL != _pi) ? _pi->use_count() : 0;
    }

    bool unique() const {
        return 1 == use_count();
    }

    bool empty() const {
        return NULL == _pi;
    }
    
    // http://www.cplusplus.com/reference/typeinfo/type_info/
    void* get_deleter(std::type_info const& ti ) const {
        return _pi ? _pi->get_deleter(ti) : 0;
    }
};



class weak_count {
    sp_counted_base* _pi;

    friend class shared_count;
public:
    weak_count() : _pi(NULL) {}
    ~weak_count() {
        if(NULL != _pi)
            _pi->weak_release();
    }
    
    weak_count(const shared_count& r) :_pi(r._pi) {
        if(NULL != _pi)
            _pi->weak_add_ref();
    }

    weak_count(const weak_count& r) : _pi(r._pi) {
        if(NULL != _pi)
            _pi->weak_add_ref();
    }
    
    weak_count& operator=(shared_count const& r) {
        sp_counted_base* tmp = r._pi;
        if(NULL != tmp) tmp->weak_add_ref();
        if(NULL != _pi) _pi->weak_release();
        _pi = tmp;
        return *this;
    }
    
    weak_count& operator=(weak_count const& r) {
        sp_counted_base* tmp = r._pi;
        if(NULL != tmp) tmp->weak_add_ref();
        if(NULL != _pi) _pi->weak_release();
        _pi = tmp;
        return *this;
    }

    friend inline bool operator==(weak_count const& a, weak_count const& b) {
        return a._pi == b._pi;
    }

    friend inline bool operator<(weak_count const& a, weak_count const& b) {
        return a._pi < b._pi;
    }
    
    void swap(weak_count& r) {
        sp_counted_base* tmp = r._pi;
        r._pi = _pi;
        _pi = tmp;
    }
    
    long use_count() const {
        return (NULL != _pi) ? _pi->use_count() : 0;
    }
};



inline shared_count::shared_count(weak_count const& r) : _pi(r._pi) {
    if(NULL != _pi)
        _pi->add_ref_lock();
    else
        __throw_bad_weak_ptr();
}



}

#endif

