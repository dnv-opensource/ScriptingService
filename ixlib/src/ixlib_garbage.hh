// ----------------------------------------------------------------------------
//  Description      : Garbage collection
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#ifndef IXLIB_GARBAGE
#define IXLIB_GARBAGE

#include <memory>
#include <ixlib_exgen.hh>
#include <ixlib_base.hh>
#include "boost\smart_ptr\intrusive_ptr.hpp"

namespace ixion {

/**
ref_ptr is an encapsulation of boost::intrusive_ptr. Requires T to inherit from Utility::IntrusiveClass<T>

Example:
<code>
  int main() {
    ref_ptr<int> my_int = new int(5);
    *my_int = 17;
    ref_ptr<int> another_int = my_int;
    *another_int = 12;

    *my_int == 12; // true
    }
  </code>
*/

template <class T, class T_Managed = T>
class ref_ptr : public boost::intrusive_ptr<T>
{
public:
    ref_ptr(ref_ptr const& src) : boost::intrusive_ptr<T>(src)
    {
    }

    template <typename T2>
    ref_ptr(ref_ptr<T2, T_Managed> const& src) : boost::intrusive_ptr<T>(src)
    {
    }

    ref_ptr(T* src = 0) : boost::intrusive_ptr<T>(src)
    {
    }

    using boost::intrusive_ptr<T>::operator=;
};

}
#endif
