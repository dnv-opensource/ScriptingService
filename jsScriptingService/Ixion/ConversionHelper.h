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

#include "ixlib_javascript.hh"
#include "Reflection/Objects/Object.h"
#include "jsScript/jsValue.h"

namespace ixion { namespace javascript {
    using namespace DNVS::MoFa::Reflection;
    class ConversionHelper {
    public:
        static ref_ptr<value> ToIxion(const mofa::ref<jsValue>& value);
        static ref_ptr<value> ToIxion(const Objects::Object& value);
        static ref_ptr<value> Lookup(const Objects::Object& value, const std::string& identifier);
        static ref_ptr<value> Lookup(const mofa::ref<jsValue>& value, const std::string& identifier);
        static ref_ptr<value> Subscript(Objects::Object object, const value& index);
        static ref_ptr<value> Subscript(const mofa::ref<jsValue>& object, const value& index);
        static size_t Size(const Objects::Object& value);
        static size_t Size(const mofa::ref<jsValue>& value);
        static ixlib_iterator Begin(const Objects::Object& value);
        static ixlib_iterator Begin(const mofa::ref<jsValue>& value);
        static ixlib_iterator End(const Objects::Object& value);
        static ixlib_iterator End(const mofa::ref<jsValue>& value);

        static ref_ptr<value> InvokeBinaryOperator(const Objects::Object& lhs, const Objects::Object& rhs, value::operator_id op);
    };
}}