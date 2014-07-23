#ifndef EAGLE_ATOMIC_H
#define EAGLE_ATOMIC_H
#include <stdint.h>
#include "eagle_atomic_asm.h"

namespace eagle
{
class Atomic
{
public:

    typedef int atomic_type;

    Atomic(atomic_type at = 0)
    {
        set(at);
    }

    Atomic& operator++()
    {
        inc();
        return *this;
    }

    Atomic& operator--()
    {
        dec();
        return *this;
    }

    operator atomic_type() const
    {
        return get();
    }

    Atomic& operator+=(atomic_type n)
    {
        add(n);
        return *this;
    }

    Atomic& operator-=(atomic_type n)
    {
        sub(n);
        return *this;
    }

    Atomic& operator=(atomic_type n)
    {
        set(n);
        return *this;
    }

    atomic_type get() const         { return atomic_read(&value_); }

    atomic_type add(atomic_type i)  { return atomic_add_return(i, &value_); }

    atomic_type sub(atomic_type i)  { return atomic_sub_return(i, &value_); }

    atomic_type inc()               { return add(1); }

    atomic_type dec()               { return sub(1); }

    void inc_fast()               { return atomic_inc(&value_); }

    bool dec_and_test()               { return atomic_dec_and_test(&value_); }

    atomic_type set(atomic_type i)  { atomic_set(&value_, i); return i; }

protected:

    atomic_t    value_;
};
}

#endif
