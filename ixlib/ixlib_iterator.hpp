// Copyright Peder Holt 2003. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_CLIPP_IXLIB_ITERATOR_HPP_HOLT_04122003
#define BOOST_CLIPP_IXLIB_ITERATOR_HPP_HOLT_04122003

#include <ixlib_javascript.hh>
#include <iterator>

namespace ixion { namespace javascript {

struct ixlib_iterator_base
{
    ixlib_iterator_base(ixlib_iterator_base const& rhs)
    {
    }

    ixlib_iterator_base()
    {
    }

    virtual ~ixlib_iterator_base()
    {
    }

    virtual void inc() = 0;
    virtual void dec() = 0;
    virtual ref_ptr<value> get() const = 0;
    virtual bool equals(ixlib_iterator_base const* rhs) const = 0;
    virtual ixlib_iterator_base* construct() = 0;
};

struct ixlib_iterator : std::iterator<std::bidirectional_iterator_tag, ref_ptr<value>, ref_ptr<value> >
{
    ixlib_iterator(ixlib_iterator_base* base) : m_holder(base)
    {
    }

    ixlib_iterator(ixlib_iterator const& rhs) : m_holder(rhs.m_holder->construct())
    {
    }

    ~ixlib_iterator()
    {
    }

    const ref_ptr<value> operator*() const
    {
        return m_holder->get();
    }

    ref_ptr<value> operator*()
    {
        return m_holder->get();
    }

    ixlib_iterator& operator++()
    {
        m_holder->inc();
        return *this;
    }

    ixlib_iterator& operator--()
    {
        m_holder->dec();
        return *this;
    }

    ixlib_iterator operator++(int)
    {
        ixlib_iterator tmp(*this);
        m_holder->inc();
        return tmp;
    }

    ixlib_iterator operator--(int)
    {
        ixlib_iterator tmp(*this);
        m_holder->dec();
        return tmp;
    }

    friend bool operator==(ixlib_iterator const& lhs, ixlib_iterator const& rhs)
    {
        return lhs.m_holder->equals(rhs.m_holder.get());
    }

    friend bool operator!=(ixlib_iterator const& lhs, ixlib_iterator const& rhs)
    {
        return !lhs.m_holder->equals(rhs.m_holder.get());
    }

protected:
    std::auto_ptr<ixlib_iterator_base> m_holder;
};

}} // namespace ixion::javascript

#endif  // BOOST_CLIPP_VALUE_ITERATOR_HPP_HOLT_04122003