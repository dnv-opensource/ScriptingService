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
#include <string>

namespace std {
    string DoubleToString(double value);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, char**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned char**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, signed char**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, short**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned short**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, int**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned int**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, long int**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned long**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned __int64**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, __int64**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, float**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, double**);
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, bool**);
}