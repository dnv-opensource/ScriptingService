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
#include "Reflection\TypeLibraries\TypeLibraryPointer.h"
#include "LValue.h"
#include <vector>
#include "jsScript\jsFunction.h"

namespace DNVS { namespace MoFa { namespace Scripting {
    using Reflection::Objects::Object;
    class Array {
    public:
        Array(const Array&) = default;
        Array(Array&&) = default;
        Array& operator=(const Array&) = default;
        Array& operator=(Array&&) = default;
        Array();
        Array(const std::vector<Object>& objects);
        Array(size_t size);
        ~Array();
        std::shared_ptr<LValue> operator[](size_t index);
        Object operator[](size_t index) const;
        
        int GetLength() const;
        int Push(const Object& first, const std::vector<Object>& objects = {});
        Object Pop();
        Object Shift();
        void Sort();
        void Sort(Reflection::Members::MemberPointer member);
        Array Splice(int position, int count, const std::vector<Object>& objects = {});
        Array Slice(int begin, int end) const;
        int Unshift(const std::vector<Object>& objects);
        std::string Join(const std::string& separator) const;
        typedef std::vector<std::shared_ptr<LValue>> LValueVector;
        const LValueVector& GetData() const;

        LValueVector::iterator begin();
        LValueVector::iterator end();
        LValueVector::const_iterator begin() const;
        LValueVector::const_iterator end() const;
        int size() const;
        bool empty() const;
    private:
        void Resize(size_t size);
        LValueVector m_data;
    };    
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, Array**);
}}}
