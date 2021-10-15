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
#include "EliminateWrappersConversion.h"
#include "ixlib_garbage.hh"
#include "ixlib_javascript.hh"
#include "Reflection\Variants\VariantService.h"
#include "ref_ptr.Reflection.h"

namespace ixion { namespace javascript {

    Variants::Variant EliminateWrappersConversion::Convert(const Variants::Variant& other)
    {
        ref_ptr<value> wrapper = Variants::InternalVariantService::UnreflectUnchecked<ref_ptr<value>>(other);
        if (wrapper.get()) {
            auto result = wrapper->eliminateWrappers();
            if (result != wrapper)
                return Variants::VariantService().Reflect(result);
        }
        return other;
    }

    void EliminateWrappersConversion::IntrusiveConvert(Variants::Variant& variable)
    {
        variable = Convert(variable);
    }

}}

