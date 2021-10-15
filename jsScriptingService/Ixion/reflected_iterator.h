#pragma once
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
#include "ixlib_iterator.hpp"
#include "Reflection\Objects\Object.h"

namespace ixion {namespace javascript {
    using DNVS::MoFa::Reflection::Objects::Object;

    class reflected_iterator : public ixlib_iterator_base
    {
    public:
        reflected_iterator(const Object& object);
        virtual void inc() override;
        virtual void dec() override;
        virtual ref_ptr<value> get() const override;
        virtual bool equals(ixlib_iterator_base const* rhs) const override;
        virtual ixlib_iterator_base* construct() override;
    private:
        void RemoveConst();
        Object m_object;
    };
}}