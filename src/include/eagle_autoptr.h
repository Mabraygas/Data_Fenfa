#ifndef EAGLE_AUTOPTR_H
#define EAGLE_AUTOPTR_H
#include "eagle_ex.h"
#include "eagle_atomic.h"

namespace eagle
{

template<class  T>
class AutoPtrBaseT
{
public:

    typedef T atomic_type;

    AutoPtrBaseT& operator=(const AutoPtrBaseT&)
    {
        return *this;
    }

    void incRef() { atomic_.inc_fast(); }

    void decRef()
    {
        if(atomic_.dec_and_test() && !bNoDelete_)
        {
            bNoDelete_ = true;
            delete this;
        }
    }

    int getRef() const        { return atomic_.get(); }

    void setNoDelete(bool b)  { bNoDelete_ = b; }

protected:

    AutoPtrBaseT() : atomic_(0), bNoDelete_(false)
    {
    }

    AutoPtrBaseT(const AutoPtrBaseT&) : atomic_(0), bNoDelete_(false)
    {
    }

    virtual ~AutoPtrBaseT()
    {
    }

protected:

    atomic_type   atomic_;

    bool        bNoDelete_;
};

template<>
inline void AutoPtrBaseT<int>::incRef() 
{ 
    ++atomic_; 
}

template<> 
inline void AutoPtrBaseT<int>::decRef()
{
    if(--atomic_ == 0 && !bNoDelete_)
    {
        bNoDelete_ = true;
        delete this;
    }
}

template<> 
inline int AutoPtrBaseT<int>::getRef() const        
{ 
    return atomic_; 
} 

typedef AutoPtrBaseT<Atomic> AutoPtrBase;


template<typename T>
class AutoPtr
{
public:

    typedef T element_type;

	AutoPtr(T* p = 0)
    {
        ptr_ = p;

        if(ptr_) {
            ptr_->incRef();
        }
    }

    template<typename Y>
    AutoPtr(const AutoPtr<Y>& r)
    {
        ptr_ = r.ptr_;

        if(ptr_) {
            ptr_->incRef();
        }
    }

    AutoPtr(const AutoPtr& r)
    {
        ptr_ = r.ptr_;

        if(ptr_) {
            ptr_->incRef();
        }
    }

    ~AutoPtr()
    {
        if(ptr_) {
            ptr_->decRef();
        }
    }

    AutoPtr& operator=(T* p)
    {
        if(ptr_ != p) {
            if(p) {
                p->incRef();
            }

            T* ptr = ptr_;
            ptr_ = p;

            if(ptr) {
                ptr->decRef();
            }
        }
        return *this;
    }

    template<typename Y>
    AutoPtr& operator=(const AutoPtr<Y>& r)
    {
        if(ptr_ != r.ptr_)
        {
            if(r.ptr_) {
                r.ptr_->incRef();
            }

            T* ptr = ptr_;
            ptr_ = r.ptr_;

            if(ptr) {
                ptr->decRef();
            }
        }
        return *this;
    }

    AutoPtr& operator=(const AutoPtr& r)
    {
        if(ptr_ != r.ptr_)
        {
            if(r.ptr_) {
                r.ptr_->incRef();
            }

            T* ptr = ptr_;
            ptr_ = r.ptr_;

            if(ptr) {
                ptr->decRef();
            }
        }
        return *this;
    }

    template<class Y>
    static AutoPtr dynamicCast(const AutoPtr<Y>& r)
    {
        return AutoPtr(dynamic_cast<T*>(r.ptr_));
    }

    template<class Y>
    static AutoPtr dynamicCast(Y* p)
    {
        return AutoPtr(dynamic_cast<T*>(p));
    }

    T* get() const
    {
        return ptr_;
    }

    T* operator->() const
    {
        if(!ptr_)
        {
            throwNullHandleException();
        }

        return ptr_;
    }

    T& operator*() const
    {
        if(!ptr_)
        {
            throwNullHandleException();
        }

        return *ptr_;
    }

    operator bool() const
    {
        return ptr_ ? true : false;
    }

    void swap(AutoPtr& other)
    {
        std::swap(ptr_, other.ptr_);
    }

protected:

    void throwNullHandleException() const;

public:

	T* ptr_;
};

template<typename T> inline void
AutoPtr<T>::throwNullHandleException() const
{
    throw EagleException("autoptr null handle error");
}

template<typename T, typename U>
inline bool operator==(const AutoPtr<T>& lhs, const AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l == *r;
    }
    else
    {
        return !l && !r;
    }
}

template<typename T, typename U>
inline bool operator!=(const AutoPtr<T>& lhs, const AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l != *r;
    }
    else
    {
        return l || r;
    }
}

template<typename T, typename U>
inline bool operator<(const AutoPtr<T>& lhs, const AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l < *r;
    }
    else
    {
        return !l && r;
    }
}

}
#endif

