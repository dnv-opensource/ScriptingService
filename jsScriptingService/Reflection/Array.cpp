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
#include "Array.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\Containers\ReflectVector.h"
#include "Reflection\Attributes\ContainerAttribute.h"
#include "Reflection\Attributes\AttributeCollection.h"
#include "Reflection\Members\IMember.h"
#include "Reflection\Objects\Delegate.h"
#include "Reflection\AutoReflect.h"
#include "Formatting\FormattingService.h"
#include "Reflection\Objects\Object.h"
#include "Undefined.h"
#include "Reflection\TypeLibraries\ITypeLibrary.h"
#include "Services\ServiceProvider.h"
#include <algorithm>

namespace DNVS { namespace MoFa { namespace Scripting {
    using namespace Reflection;
    using namespace Reflection::Objects;

    Array::Array(const std::vector<Object>& objects)
        : Array(objects.size())
    {
        for (size_t i = 0; i < objects.size(); ++i)
            m_data[i]->Reset(objects[i]);
    }

    Array::Array(size_t size)
    {
        Resize(size);
    }

    Array::Array()
    {

    }

    Array::~Array()
    {

    }

    std::shared_ptr<LValue> Array::operator[](size_t index)
    {
        if (index >= m_data.size())
            Resize(index + 1);
        return m_data[index];
    }

    Object Array::operator[](size_t index) const
    {
        if (index >= m_data.size())
            return Object();
        else 
            return m_data.at(index)->GetObject();
    }

    int Array::GetLength() const
    {
        return (int)m_data.size();
    }

    int Array::Push(const Object& first, const std::vector<Object>& objects)
    {
        m_data.push_back(std::make_shared<LValue>(first));
        Array lvalues(objects);
        m_data.insert(m_data.end(), lvalues.begin(), lvalues.end());
        return GetLength();
    }

    Object Array::Pop()
    {
        if (m_data.empty())
            return Object(Services::ServiceProvider::Instance().TryGetService<Reflection::TypeLibraries::ITypeLibrary>(), Undefined());
        Object back = m_data.back()->GetObject();
        m_data.pop_back();
        return back;
    }

    Object Array::Shift()
    {
        Object front = m_data.front()->GetObject();
        m_data.erase(m_data.begin());
        return front;
    }

    void Array::Sort()
    {
        if (m_data.size() <= 1)
            return;
        std::sort(m_data.begin(), m_data.end(), [](const std::shared_ptr<LValue>& lhs, const std::shared_ptr<LValue>& rhs) -> bool {
            if (lhs == rhs)
                return false;
            if (!lhs)
                return true;
            if (!rhs)
                return false;
            std::string strA = "undefined";
            std::string strB = "undefined";
            try {
                strA = lhs->GetObject().As<std::string>();
            }
            catch(...) {}
            try {
                strB = rhs->GetObject().As<std::string>();
            }
            catch (...) {}
            return strA < strB;
        });
    }

    void Array::Sort(Reflection::Members::MemberPointer member)
    {
        if (!member)
            throw std::runtime_error("Invalid sort function)");
        if (m_data.size() <= 1)
            return;
        std::sort(m_data.begin(), m_data.end(), [member](const std::shared_ptr<LValue>& lhs, const std::shared_ptr<LValue>& rhs) -> bool {
            if (lhs == rhs)
                return false;
            if (!lhs)
                return true;
            if (!rhs)
                return false;
            Reflection::Variants::Variant result = member->Invoke({ lhs->GetObject().GetVariant(), rhs->GetObject().GetVariant()});
            return Reflection::Objects::Object(jsStack::stack()->GetTypeLibrary(), result).As<double>() < 0.;
        });

    }

    Array Array::Splice(int position, int count, const std::vector<Object>& objects)
    {
        int end = position + count;
        if (end < 0)
            end = std::numeric_limits<int>::max();
        Array newArray = Slice(position, end);
        if(newArray.GetLength() > 0)
            m_data.erase(m_data.begin() + position, m_data.begin() + position + newArray.GetLength());
        if (!objects.empty())
        {
            Array lvalues(objects);
            m_data.insert(m_data.begin() + position, lvalues.begin(), lvalues.end());
        }
        return newArray;
    }

    Array Array::Slice(int begin, int end) const
    {
        Array newArray;
        for (size_t i = begin; i < end; ++i)
        {
            if (i < m_data.size())
                newArray.Push(m_data[i]->GetObject());
            else
                break;
        }
        return newArray;
    }

    int Array::Unshift(const std::vector<Object>& objects)
    {
        Array lvalues(objects);
        m_data.insert(m_data.begin(), lvalues.begin(), lvalues.end());
        return GetLength();
    }

    std::string Array::Join(const std::string& separator) const
    {
        std::string str;
        for (const auto& object : m_data)
        {
            if (!str.empty())
                str += separator;
            if (object->GetObject().IsConvertibleTo<std::string>())
                str += object->GetObject().As<std::string>();
            else
                str += ToString(object->GetObject().ConvertToDynamicType(), DNVS::MoFa::Formatting::FormattingService());
        }
        return str;
    }

    const Array::LValueVector& Array::GetData() const
    {
        return m_data;
    }

    Array::LValueVector::iterator Array::begin()
    {
        return m_data.begin();
    }

    Array::LValueVector::iterator Array::end()
    {
        return m_data.end();
    }

    Array::LValueVector::const_iterator Array::begin() const
    {
        return m_data.begin();
    }

    Array::LValueVector::const_iterator Array::end() const
    {
        return m_data.end();
    }

    int Array::size() const
    {
        return (int)m_data.size();
    }

    bool Array::empty() const
    {
        return m_data.empty();
    }

    void Array::Resize(size_t size)
    {
        m_data.resize(size);
        for (std::shared_ptr<LValue>& lvalue : m_data)
        {
            if (!lvalue)
                lvalue = std::make_shared<LValue>();
        }
    }

    class ArrayToContainerConverter : public TypeConversions::IConversion
    {
    public:
        ArrayToContainerConverter(const TypeLibraries::TypeLibraryPointer& typeLibrary, const Types::DecoratedTypeInfo& returnType)
            : m_typeLibrary(typeLibrary)
            , m_returnType(returnType)
        {}
        virtual Variants::Variant Convert(const Variants::Variant& variant) override
        {
            auto containerType = m_typeLibrary->LookupType(m_returnType.GetTypeInfo());
            auto constructor = containerType->Lookup(Members::ConstructorString);
            Objects::Object container(m_typeLibrary, constructor->Invoke({}));

            const Array& myArray = Variants::InternalVariantService::UnreflectUnchecked<const Array&>(variant);
            for(const auto& lvalue : myArray.GetData())
            {
                container.Lookup("Add")(lvalue->GetObject());
            }
            return container.GetVariant();
        }

        virtual void IntrusiveConvert(Variants::Variant& variant) override
        {
            variant = Convert(variant);
        }

    private:
        TypeLibraries::TypeLibraryPointer m_typeLibrary;
        Types::DecoratedTypeInfo m_returnType;
    };

    class ArrayToContainerAlternativeConverter : public TypeConversions::IAlternativeConverter
    {
    public:
        ArrayToContainerAlternativeConverter(const TypeLibraries::TypeLibraryPointer& typeLibrary)
            : m_typeLibrary(typeLibrary)
        {}
        virtual bool CanConvert(const Variants::Variant& variant, const Types::DecoratedTypeInfo& returnType) const
        {
            if (variant.GetDecoratedTypeInfo().GetTypeInfo() != typeid(Array))
                return false;
            auto typeLibrary = m_typeLibrary.lock();
            if (!typeLibrary)
                return false;
            auto containerType = typeLibrary->LookupType(returnType.GetTypeInfo());
            if (!containerType)
                return false;
            if (!containerType->GetAttributeCollection().HasAttribute<Attributes::ContainerAttribute>())
                return false;

            return AreAllItemsConvertibleTo(variant, containerType->GetAttributeCollection().GetAttribute<Attributes::ContainerAttribute>().GetValueType());
        }
        TypeConversions::ConversionPointer CreateConverter(const Types::DecoratedTypeInfo& returnType) const
        {
            return std::make_shared<ArrayToContainerConverter>(m_typeLibrary.lock(), returnType);
        }
        virtual TypeConversions::ConversionType::Type GetConversionType() const override
        {
            return TypeConversions::ConversionType::UserConversion;
        }
    private:
        virtual bool AreAllItemsConvertibleTo(const Variants::Variant& variant, const Types::DecoratedTypeInfo& valueType) const
        {
            auto typeLibrary = m_typeLibrary.lock();
            const Array& myArray = Variants::InternalVariantService::UnreflectUnchecked<const Array&>(variant);
            for(const auto& lvalue : myArray.GetData())
            {
                if (!lvalue->GetObject().IsConvertibleTo(valueType))
                    return false;
            }
            return true;
        }
        std::weak_ptr<TypeLibraries::ITypeLibrary> m_typeLibrary;
    };

    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, Array**)
    {
        using namespace Classes;
        Class<Array, Array> cls(typeLibrary, "Array");
        cls.Constructor<std::vector<Object>>(Vararg);
        //Remove this constructor for compatibility
        //cls.Constructor<size_t>(Explicit);
        cls.Operator(This[size_t()]);        
        cls.Operator(This.Const[size_t()]);
        cls.Get("length", &Array::GetLength)
            .AddDocumentation("Returns the size of the array");
        cls.Function("pop", &Array::Pop)
            .AddDocumentation("Removes the last item from the array (returns this item)");            
        cls.Function("push", &Array::Push, Vararg)
            .AddDocumentation("Adds one or more items to the end of the array (returns the length of the array)")
            .AddSignature("object");
        typeLibrary->GetConversionGraph()->AddAlternativeConverter(
            Types::TypeId<Array>(), std::make_shared<ArrayToContainerAlternativeConverter>(typeLibrary));

        cls.AddAttribute<ContainerAttribute>(DNVS::MoFa::Reflection::Types::TypeId<Array>(), false);
        cls.Constructor();
        cls.Function<Array::LValueVector::iterator()>("begin", &Array::begin);
        cls.Function<Array::LValueVector::const_iterator(), Const>("begin", &Array::begin);
        cls.Function<Array::LValueVector::iterator()>("end", &Array::end);
        cls.Function<Array::LValueVector::const_iterator(), Const>("end", &Array::end);
        cls.Function("size", &Array::size)
            .AddDocumentation("Returns the size of the array");
        cls.Function("empty", &Array::empty)
            .AddDocumentation("Returns true if the array is empty");
        cls.Function("Add", &Array::Push, Vararg)
            .AddDocumentation("Adds one or more items to the array, Same as Push")
            .AddSignature("object");
        cls.Function("reverse", [](Array& array) {std::reverse(array.begin(), array.end()); })
            .AddDocumentation("Reverses the items in the array");
        cls.Function("shift", &Array::Shift)
            .AddDocumentation("Remove one item from the start of the array (returns this item)");
        cls.Function("unshift", &Array::Unshift, Vararg)
            .AddDocumentation("Add items to the front of the array (returns the length of the array");
        cls.Function("splice", &Array::Splice, Vararg)
            .AddDocumentation("Remove items from the array and return this as a new array")
            .AddSignature("position", Arg("count") = std::numeric_limits<int>::max());
        cls.Function("slice", &Array::Slice)
            .AddDocumentation("Returns a copy of (parts of the) array")
            .AddSignature(Arg("begin") = 0, Arg("end") = std::numeric_limits<int>::max());
        cls.Function("join", &Array::Join)
            .AddDocumentation("Converts arguments to strings and joins the result.")
            .AddSignature(Arg("separator") = std::string(","));
        cls.Function("ToString", [](const Array& a) {return a.Join(","); });
        cls.Function<void()>("sort", &Array::Sort)
            .AddDocumentation("Sorts an array by converting all items to strings and comparing the strings.");
        cls.Function<void(Members::MemberPointer)>("sort", &Array::Sort)
            .AddDocumentation("Sorts an array by converting all items to strings and comparing the strings.");


        Class<Array::LValueVector::const_iterator, DNVS::MoFa::Reflection::IgnoreAutoReflector> const_iterator(typeLibrary, "__ConstIterator__");
        const_iterator.Operator(++This);
        const_iterator.Operator(--This);
        const_iterator.Operator(This++);
        const_iterator.Operator(This--);
        const_iterator.Operator(This.Const == This.Const);
        const_iterator.Operator(This.Const != This.Const);
        const_iterator.Operator(*This.Const);

        Class<Array::LValueVector::iterator, Public<Array::LValueVector::const_iterator>, DNVS::MoFa::Reflection::IgnoreAutoReflector> iterator(typeLibrary, "__Iterator__");
        iterator.Operator(++This);
        iterator.Operator(--This);
        iterator.Operator(This++);
        iterator.Operator(This--);
        iterator.Operator(This.Const == This.Const);
        iterator.Operator(This.Const != This.Const);
        iterator.Operator(*This.Const);
    }
}}}

