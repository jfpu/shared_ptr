/*=============================================================================
#     FileName: shared_ptr_base.h
#         Desc: 
#       Author: Jeffrey Pu
#        Email: pujunying@gmail.com
#     HomePage: https://github.com/jfpu
#      Version: 0.0.1
#   LastChange: 2013-12-18 23:53:43
#      History:
=============================================================================*/

#ifndef _SHARED_PTR_BASE_H_
#define _SHARED_PTR_BASE_H_

#include <iostream>
#include <memory>
#include <cassert>
#include <type_traits>
#include "sp_counted_base.h"

namespace jfpu {



template<typename T>
class __weak_ptr;



// A smart pointer with reference-counted copy semantics.  The
// object pointed to is deleted when the last shared_ptr pointing to
// it is destroyed or reset.
template<typename T>
class __shared_ptr {
public:
    typedef T element_type;
    typedef T* pointer_type;
    typedef __shared_ptr<T> this_type;
private:
    pointer_type px;
    shared_count pn;

    // used for access private member variable.
    template<typename Y> friend class __shared_ptr;
    template<typename Y> friend class __weak_ptr;
public:
    
    __shared_ptr() : px(NULL), pn() {}
    ~__shared_ptr() {};

    __shared_ptr(pointer_type p) : px(p), pn(p) {}

    __shared_ptr(__shared_ptr& r) : px(r.px), pn() {
        pn.swap(r.pn);
        r.px = NULL;
    }

    template<typename Y>
    __shared_ptr(const __shared_ptr<Y>& rhs, pointer_type p)
      : px(p), pn(rhs.pn) {
        __glibcxx_function_requires(_ConvertibleConcept<Y*, T*>)
      }

    // http://en.cppreference.com/w/cpp/memory/enable_shared_from_this
    template<typename Y>
    explicit __shared_ptr(Y* __p) : px(__p), pn(__p) {
        __glibcxx_function_requires(_ConvertibleConcept<Y*, T*>)
        #if 0
        typedef int _IsComplete[sizeof(Y)];
        __enable_shared_from_this_helper(_M_refcount, __p, __p);
        #endif
	}

    template<typename D>
    __shared_ptr(std::nullptr_t p, D d) : px(NULL), pn(p, d) {}
    
    template<typename Y, typename D>
    __shared_ptr(Y* p, D d) : px(p), pn(p, d) {
        __glibcxx_function_requires(_ConvertibleConcept<Y*, T*>)
    }
    
    // http://en.cppreference.com/w/cpp/types/is_convertible
    template<typename Y,
             typename Z = typename std::enable_if<std::is_convertible<Y*, T*>::value>::type >
    __shared_ptr(const __shared_ptr<Y>& r) : px(r.px), pn(r.pn) {}
    #if 0
    template<typename Y,
             typename Z = typename std::enable_if<std::is_convertible<Y*, T*>::value>::type >
    __shared_ptr(const __shared_ptr<Y>& r) : px(r.px), pn(r) {
        pn.swap(r.pn);
        r.px = NULL;
    }
    #endif

    template<typename Y>
    explicit __shared_ptr(const __weak_ptr<Y>& r) : pn(r.pn) {
        __glibcxx_function_requires(_ConvertibleConcept<Y*, T*>);
        px = r.px;
    }
    
    // If an exception is thrown this constructor has no effect.
    template<typename Y, typename D>
    __shared_ptr(std::unique_ptr<Y, D>&& r) : px(r.px), pn() {
        __glibcxx_function_requires(_ConvertibleConcept<Y*, T*>);
        Y* tmp = r.release();
        pn = shared_count(std::move(tmp));
        // __enable_shared_from_this_helper(pn, tmp, tmp);
    }
    
#if !defined(__GXX_EXPERIMENTAL_CXX0X__) || _GLIBCXX_USE_DEPRECATED
    // Postcondition: use_count() == 1 and __r.get() == 0
    // explicit __shared_ptr(std::auto_ptr<Y>& r) : px(r.get()), pn() {     --- gcc-4.7.0
    // explicit __shared_ptr(std::auto_ptr<Y>&& r) : px(r.get()), pn() {     --- gcc-4.6.2
    template<typename Y>
    explicit __shared_ptr(std::auto_ptr<Y>& r) : px(r.get()), pn() {
        __glibcxx_function_requires(_ConvertibleConcept<Y*, T*>)
        static_assert( sizeof(Y) > 0, "incomplete type" );
        Y* tmp = r.release();
        pn = shared_count(std::move(tmp));
        // __enable_shared_from_this_helper(pn, tmp, tmp);
    }
#endif

    template<typename Y>
    __shared_ptr(const __shared_ptr<Y>& r, __static_cast_tag)
      : px(static_cast<element_type*>(r.px)), pn(r.pn) {}
    
    template<typename Y>
    __shared_ptr(const __shared_ptr<Y>& r, __const_cast_tag)
      : px(const_cast<element_type*>(r.px)), pn(r.pn) {}
    
    template<typename Y>
    __shared_ptr(const __shared_ptr<Y>& r, __dynamic_cast_tag)
      : px(dynamic_cast<element_type*>(r.px)), pn(r.pn) {
        if(NULL == px)
            pn = shared_count();
    }
    
    void reset() {
        this_type().swap(*this);
    }
    
    template<typename Y>
    void reset(Y* p) {
        // GLIBCXX_DEBUG_ASSERT(NULL == p || px != p);
        assert(NULL == p || px != p);
        __shared_ptr(p).swap(*this);
    }

    template<typename Y>
    void reset(__shared_ptr<Y> const& r) {
        r.swap(*this);
    }

    template<typename Y>
    void reset(__shared_ptr<Y> const& r, pointer_type p) {
        this_type(r, p).swap(*this);
    }
    
    template<typename Y, typename D>
    void reset(Y* p, D d) {
        __shared_ptr(p, d).swap(*this);
    }
    
    pointer_type get() const {
        return px;
    }
    
    bool unique() const {
        return pn.unique();
    }

    long use_count() const {
        return pn.use_count();
    }

    void swap(__shared_ptr& r) {
        std::swap(px, r.px);
        pn.swap(r.pn);
    }
    #if 0
    template<typename Y>
    void swap(__shared_ptr<Y>& r) {
        std::swap(px, static_cast<T*>(r.px));
        pn.swap(r.pn);
    }
    #endif
private:
    // http://www.cplusplus.com/reference/typeinfo/type_info/
    void* m_get_deleter(const std::type_info& ti) const {
        return pn.get_deleter(ti);
    }
public:
    
    template<typename D, typename Y>
    friend D* get_deleter(const __shared_ptr<Y>& p);
    
    template<typename Y>
    bool owner_before(__shared_ptr<Y> const& rhs) const {
        return pn < rhs.pn;
    }
    
    template<typename Y>
    bool owner_before(__weak_ptr<Y> const& rhs) const {
        return pn < rhs.pn;
    }
    
    bool _internal_equiv(__shared_ptr const& rhs) const {
        return px == rhs.px && pn == rhs.pn;
    }
    
    element_type operator*() const {
        assert(NULL != px);
        return *px;
    }
    
    pointer_type operator->() const {
        assert(NULL != px);
        return px;
    }
    
    bool operator!() const {
        return NULL == px;
    }
    
    __shared_ptr& operator=(__shared_ptr const& r) {
        this_type(r).swap(*this);
        return *this;
    }
    
// http://zh.cppreference.com/w/cpp/algorithm/move
//#if _GLIBCXX_USE_DEPRECATED
#if !defined(__GXX_EXPERIMENTAL_CXX0X__) || _GLIBCXX_USE_DEPRECATED
    template<typename Y>
    __shared_ptr& operator=(std::auto_ptr<Y>& r) {
        __shared_ptr(std::move(r)).swap(*this);
        return *this;
    }
#endif

#if 0
    template<typename Y>
    __shared_ptr& operator=(std::auto_ptr<Y>& r) {
        this_type(r).swap(*this);
        return *this;
    }
#endif
    
    template<typename Y, typename D>
    __shared_ptr& operator=(std::unique_ptr<Y, D>&& r){
        __shared_ptr(std::move(r)).swap(*this);
        return *this;
    }
    
    template<typename Y>
    friend inline bool operator<(__shared_ptr const& l, __shared_ptr<Y> const& r) {
        l.owner_before(r);
    }
    
    template<typename Y>
    friend inline bool operator==(__shared_ptr const& l, __shared_ptr<Y> const& r) {
        return l.get() == r.get();
    }
    
    template<typename Y>
    friend inline bool operator!=(__shared_ptr const& l, __shared_ptr<Y> const& r) {
        return l.get() != r.get();
    }

    
};

template<typename Y>
inline void swap(__shared_ptr<Y>& lhs, __shared_ptr<Y>& rhs) {
    lhs.swap(rhs);
}

template<typename T, typename U>
__shared_ptr<T> static_pointer_cast(__shared_ptr<U> const& rhs) {
    (void)static_cast<T*>(static_cast<U*>(0));
    typedef typename __shared_ptr<T>::pointer_type PE;
    PE p = static_cast<PE>(rhs.get());
    return __shared_ptr<T>(rhs, p);
}

template<typename T, typename U>
__shared_ptr<T> const_pointer_cast(__shared_ptr<U> const& rhs) {
    (void)const_cast<T*>(static_cast<U*>(0));
    typedef typename __shared_ptr<T>::pointer_type PE;
    PE p = const_cast<PE>(rhs.get());
    return __shared_ptr<T>(rhs, p);
}

template<typename T, typename U>
__shared_ptr<T> dynamic_pointer_cast(__shared_ptr<U> const& rhs) {
    (void)dynamic_cast<T*>(static_cast<U*>(0));
    typedef typename __shared_ptr<T>::pointer_type PE;
    PE p = dynamic_cast<PE>(rhs.get());
    return __shared_ptr<T>(rhs, p);
}

template<typename T, typename U>
__shared_ptr<T> reinterpret_pointer_cast(__shared_ptr<U> const& rhs) {
    (void)reinterpret_cast<T*>(static_cast<U*>(0));
    typedef typename __shared_ptr<T>::pointer_type PE;
    PE p = reinterpret_cast<PE>(rhs.get());
    return __shared_ptr<T>(rhs, p);
}

template<typename T>
inline typename __shared_ptr<T>::pointer_type get_pointer(__shared_ptr<T> const& rhs) {
    return rhs.get();
}

template<typename T>
std::ostream& operator<<(std::ostream& os, __shared_ptr<T> const& rhs) {
    os << rhs.get();
    return os;
}

template<typename D, typename Y>
inline D* get_deleter(const __shared_ptr<Y>& p) {
#ifdef __GXX_RTTI
    return static_cast<D*>(p.m_get_deleter(typeid(D)));
#else
    return NULL;
#endif
}


template<typename T>
class __weak_ptr {

};



}



#endif

