//one line to give the library's name and an idea of what it does.
// Copyright(C) 2021 DNV AS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; 
// version 2 of the License.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
// Library General Public License for more details.
// 
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
// Boston, MA  02110 - 1301, USA.
#include "jsValue_iterator.h"
#include "jsScript\jsValue.h"
#include "ConversionHelper.h"

namespace ixion {namespace javascript {

    jsValue_iterator::jsValue_iterator(jsValueIterator it) 
        : m_iterator(it)
    {

    }

    void jsValue_iterator::inc()
    {
        ++m_iterator;
    }

    void jsValue_iterator::dec()
    {
        --m_iterator;
    }

    ixion::ref_ptr<value> jsValue_iterator::get() const
    {
        mofa::ref<jsValue> value = const_cast<jsValue*>(*m_iterator);
        return ConversionHelper::ToIxion(value);
    }

    bool jsValue_iterator::equals(ixlib_iterator_base const* rhs) const
    {
        jsValue_iterator const* other = dynamic_cast<jsValue_iterator const*>(rhs);

        if (!other)
            return false;

        return m_iterator == other->m_iterator;
    }

    ixlib_iterator_base* jsValue_iterator::construct()
    {
        return new jsValue_iterator(m_iterator);
    }

}}

