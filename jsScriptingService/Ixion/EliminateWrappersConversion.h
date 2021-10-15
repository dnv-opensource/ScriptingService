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
#include "Reflection\TypeConversions\IConversion.h"

namespace ixion { namespace javascript {
    using namespace DNVS::MoFa::Reflection;
    class EliminateWrappersConversion : public TypeConversions::IConversion
    {
    public:
        virtual Variants::Variant Convert(const Variants::Variant& other);

        virtual void IntrusiveConvert(Variants::Variant& variable);
    };
}}