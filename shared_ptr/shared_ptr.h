/*=============================================================================
#     FileName: shared_ptr.h
#         Desc: 
#       Author: Jeffrey Pu
#        Email: pujunying@gmail.com
#     HomePage: https://github.com/jfpu
#      Version: 0.0.1
#   LastChange: 2013-12-18 23:53:50
#      History:
=============================================================================*/

#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

#include "shared_ptr_base.h"

namespace jfpu {

template<typename T> class weak_ptr;

// The actual shared_ptr, with forwarding constructors and assignment operators.
template<typename T>
class shared_ptr : public __shared_ptr<T> {
    typedef T element_type;
    typedef T* pointer_type;
public:
    shared_ptr() : __shared_ptr<T>() {}

    template<typename Y>
    explicit shared_ptr(Y* p) : __shared_ptr<T>(p) {}

    template<typename Y, typename D>
    shared_ptr(Y* p, D d) : __shared_ptr<T>(p, d) {}

    template<typename Y>
    shared_ptr(const shared_ptr<Y>& sp) : __shared_ptr<T>(sp) {}
    
    template<typename Y>
    shared_ptr(shared_ptr<Y> const& sp, pointer_type p) : __shared_ptr<T>(sp, p) {}
    
    template<typename Y>
    explicit shared_ptr(const weak_ptr<Y>& wp) : __shared_ptr<T>(wp) {}

#if !defined(__GXX_EXPERIMENTAL_CXX0X__) || _GLIBCXX_USE_DEPRECATED
    template<typename Y>
    explicit shared_ptr(std::auto_ptr<Y>& ap) : __shared_ptr<T>(ap) {}
#endif
    
    template<typename Y>
    shared_ptr(const shared_ptr<Y>& sp, __static_cast_tag)
      : __shared_ptr<T>(sp, __static_cast_tag()) {}
    
    template<typename Y>
    shared_ptr(const shared_ptr<Y>& sp, __const_cast_tag)
      : __shared_ptr<T>(sp, __const_cast_tag()) {}
    
    template<typename Y>
    shared_ptr(const shared_ptr<Y>& sp, __dynamic_cast_tag)
      : __shared_ptr<T>(sp, __dynamic_cast_tag()) {}

    template<typename Y>
    shared_ptr& operator=(const shared_ptr<Y>& sp) {
        this->__shared_ptr<T>::operator=(sp);
        return *this;
    }
    
#if !defined(__GXX_EXPERIMENTAL_CXX0X__) || _GLIBCXX_USE_DEPRECATED
    template<typename Y>
    shared_ptr& operator=(std::auto_ptr<Y>& ap) {
        this->__shared_ptr<T>::operator=(ap);
    }
#endif
    
};

    // shared_ptr specialized algorithms.
    template<typename Y>
    inline void swap(shared_ptr<Y>& spa, shared_ptr<Y>& spb) {
        spa.swap(spb);
    }

    template<typename T, typename Y>
    inline shared_ptr<T> static_pointer_cast(const shared_ptr<Y>& r) {
        return shared_ptr<T>(r, __static_cast_tag());
    }

    template<typename T, typename Y>
    inline shared_ptr<T> const_pointer_cast(const shared_ptr<Y>& r) {
        return shared_ptr<T>(r, __const_cast_tag());
    }

    template<typename T, typename Y>
    inline shared_ptr<T> dynamic_pointer_cast(const shared_ptr<Y>& r) {
        return shared_ptr<T>(r, __dynamic_cast_tag());
    }

}


#endif

