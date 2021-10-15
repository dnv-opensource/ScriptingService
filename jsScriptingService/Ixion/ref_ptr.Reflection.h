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

#include "Reflection\Variants\SmartPointers\CopyableSmartPointerStorage.h"
#include "ixlib_garbage.hh"

namespace DNVS {namespace MoFa {namespace Reflection {namespace Variants {
    //Special handling of the ixion::ref_ptr<T,U>. We need to handle casting up and down between ref_ptr in a special way since
    //this is a very atypical smart pointer. 
    template<typename U>
    class IxionRefPtrStorage : public StorageBase
    {
    public:
        IxionRefPtrStorage(ixion::ref_ptr<U> smartPointer) 
            :   m_ptr(smartPointer)
        {
            m_data=static_cast<void*>(smartPointer.get());
        }
        ~IxionRefPtrStorage()
        {
        }
        static bool IsConvertible(Variants::StoragePointer storage, bool strict = false) {
            return dynamic_cast<IxionRefPtrStorage*>(storage.get())!=0;
        }
        template<typename T>
        static ixion::ref_ptr<T,U> RecreateSmartPointer(Variants::StoragePointer storage,T* pointer)
        {
            return pointer;
        }
    private:
        ixion::ref_ptr<U> m_ptr;
    };

    template<typename T,typename U>
    struct CopyableSmartPointerSelector<ixion::ref_ptr<T,U> > : public IxionRefPtrStorage<U>
    {
    public:
        CopyableSmartPointerSelector(const ixion::ref_ptr<T,U>& smartPointer)
            : IxionRefPtrStorage<U>(smartPointer) 
        {}
        ~CopyableSmartPointerSelector()
        {}
    };
}}}}