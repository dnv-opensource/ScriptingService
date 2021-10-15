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
#include "reflected_iterator.h"
#include "reflected_value.h"
#include "Reflection\Types\DynamicTypeTraits.h"
#include "ref_ptr.Reflection.h"
#include "ConversionHelper.h"
namespace ixion {namespace javascript {

    reflected_iterator::reflected_iterator(const Object& object)
        : m_object(object)
    {
        RemoveConst();
    }

    void reflected_iterator::inc()
    {
        m_object++;
    }

    void reflected_iterator::dec()
    {
        m_object--;
    }

    ref_ptr<value> reflected_iterator::get() const
    {
        return ConversionHelper::ToIxion(*m_object);
    }

    bool reflected_iterator::equals(ixlib_iterator_base const* rhs) const
    {
        auto other = dynamic_cast<reflected_iterator const *>(rhs);
        if (!other)
            return false;
        return m_object == other->m_object;
    }

    ixlib_iterator_base* reflected_iterator::construct()
    {
        return new reflected_iterator(m_object);
    }

    void reflected_iterator::RemoveConst()
    {
        auto variant = m_object.GetVariant();
        variant.SetDecoratedTypeInfo(DNVS::MoFa::Reflection::Types::RemoveConst(variant.GetDecoratedTypeInfo()));
        m_object = Object(m_object.GetTypeLibrary(), variant);
    }

}}