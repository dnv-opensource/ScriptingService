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
#include "String.h"
#include <boost/spirit/home/qi.hpp>
#include "Reflection/Classes/Class.h"
#include <Reflection/Containers/ReflectVector.h>
#include "boost/algorithm/string/case_conv.hpp"
#include <numeric>
#include "ixlib_re.hh"
#include "Array.h"
#include "Number.h"

namespace std {
    using namespace DNVS::MoFa::Scripting;
    template<typename T>
    T ParseStringToFloat(const std::string& arg)
    {
        if (arg.empty())
            return T();
        namespace qi = boost::spirit::qi;
        T value = std::numeric_limits<T>::quiet_NaN();
        auto first = arg.begin();
        qi::parse(first, arg.end(), qi::auto_, value);
        return value;
    }

    template<typename T>
    T StringToFloat(const std::string& arg)
    {
        if (arg.empty())
            return T();
        namespace qi = boost::spirit::qi;
        T value = std::numeric_limits<T>::quiet_NaN();
        auto first = arg.begin();
        if (qi::parse(first, arg.end(), qi::auto_, value))
        {
            while (first != arg.end())
            {
                if (boost::spirit::char_encoding::ascii::isblank(*first))
                    ++first;
                else
                    return std::numeric_limits<T>::quiet_NaN();
            }
            return value;
        }
        else
            return std::numeric_limits<T>::quiet_NaN();
    }
    using namespace DNVS::MoFa::Reflection::Variants;

    
    template<typename T>
    T StringToSignedInteger(const std::string& arg)
    {
        namespace qi = boost::spirit::qi;
        T intValue = 0;
        if (qi::parse(arg.begin(), arg.end(), qi::int_parser<T>(), intValue))
            return intValue;
        return 0;
    }
    template<typename T>
    T StringToUnsignedInteger(const std::string& arg)
    {
        namespace qi = boost::spirit::qi;
        T intValue = 0;
        if (qi::parse(arg.begin(), arg.end(), qi::uint_parser<T>(), intValue))
            return intValue;
        return 0;
    }

    Variant StringToNumber(const std::string& arg)
    {
        namespace qi = boost::spirit::qi;
        int intValue=0;
        if (qi::parse(arg.begin(), arg.end(), qi::int_, intValue))
            return VariantService::ReflectType<int>(intValue);
        double doubleValue = std::numeric_limits<double>::quiet_NaN();
        qi::parse(arg.begin(), arg.end(), qi::int_parser<double, 10, 1, -1>(), doubleValue);
        return VariantService::ReflectType<double>(doubleValue);
    }

    int indexOf(const string& self, const string& substring, string::size_type startPos)
    {
        auto result = self.find(substring, startPos);
        if (result == string::npos)
            return -1;
        else
            return (int)result;
    }
    int lastIndexOf(const string& self, const string& substring, string::size_type startPos)
    {
        auto result = self.rfind(substring, startPos);
        if (result == string::npos)
            return -1;
        else
            return (int)result;
    }

    string slice(const string& self, int start, int end)
    {
        if (end < 0)
            end = (int)self.size() + end;
        if (start < 0)
            start = (int)self.size() + start;
        return string(self, start, end - start);
    }

    string slice1(const string& self, int start)
    {
        return slice(self, start, (int)self.length());
    }

    Array split(const std::string& self, const std::string& separator, int maxLength)
    {
        Array result;
        auto typeLibrary = DNVS::MoFa::Services::ServiceProvider::Instance().GetService<DNVS::MoFa::Reflection::TypeLibraries::ITypeLibrary>();
        if(maxLength < 0)
            maxLength = (int)self.length();
        if (maxLength == 0)
            return result;

        if (separator.empty())
        {
            for (int i = 0; i < maxLength; ++i)
                result.Push(Object(typeLibrary,std::string(1,self[i])));
        }
        else
        {
            string::size_type start = 0, last = 0;
            while (true)
            {
                start = self.find(separator, last);
                if (start != std::string::npos)
                {
                    result.Push(Object(typeLibrary, self.substr(last, start - last)));
                    last = start + 1;
                }
                else
                {
                    result.Push(Object(typeLibrary,self.substr(last)));
                    break;
                }
                if(result.size() >= maxLength)
                    break;
            }
        }
        return result;
    }
    bool operator<(const string& lhs, double rhs) { return StringToFloat<double>(lhs) < rhs; }
    bool operator<(double lhs, const string& rhs) { return lhs < StringToFloat<double>(rhs); }
    bool operator<=(const string& lhs, double rhs) { return StringToFloat<double>(lhs) <= rhs; }
    bool operator<=(double lhs, const string& rhs) { return lhs <= StringToFloat<double>(rhs); }
    bool operator>(const string& lhs, double rhs) { return StringToFloat<double>(lhs) > rhs; }
    bool operator>(double lhs, const string& rhs) { return lhs > StringToFloat<double>(rhs); }
    bool operator>=(const string& lhs, double rhs) { return StringToFloat<double>(lhs) >= rhs; }
    bool operator>=(double lhs, const string& rhs) { return lhs >= StringToFloat<double>(rhs); }
    bool operator==(const string& lhs, double rhs) { return StringToFloat<double>(lhs) == rhs; }
    bool operator==(double lhs, const string& rhs) { return lhs == StringToFloat<double>(rhs); }
    bool operator!=(const string& lhs, double rhs) { return StringToFloat<double>(lhs) != rhs; }
    bool operator!=(double lhs, const string& rhs) { return lhs != StringToFloat<double>(rhs); }
    std::string operator+(const std::string& lhs, long rhs) { return lhs + std::to_string(rhs); }
    std::string operator+(long lhs, const std::string& rhs) { return std::to_string(lhs) + rhs; }
    std::string operator+(const std::string& lhs, int rhs) { return lhs + std::to_string(rhs); }
    std::string operator+(int lhs, const std::string& rhs) { return std::to_string(lhs) + rhs; }
    std::string operator+(const std::string& lhs, double rhs) { return lhs + DoubleToString(rhs); }
    std::string operator+(double lhs, const std::string& rhs) { return DoubleToString(lhs) + rhs; }
    double operator*(const std::string& lhs, double rhs) { return StringToFloat<double>(lhs) * rhs; }
    double operator*(double lhs , const std::string& rhs) { return lhs * StringToFloat<double>(rhs); }
    double operator/(const std::string& lhs, double rhs) { return StringToFloat<double>(lhs) / rhs; }
    double operator/(double lhs, const std::string& rhs) { return lhs / StringToFloat<double>(rhs); }
    void DoReflect(const DNVS::MoFa::Reflection::TypeLibraries::TypeLibraryPointer& typeLibrary, string**)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        Class<string, string> cls(typeLibrary, "string");
        cls.Constructor<const char*>();
        cls.Operator(This.Const < This.Const);
        cls.Operator(This.Const <= This.Const);
        cls.Operator(This.Const >= This.Const);
        cls.Operator(This.Const > This.Const);
        cls.Operator(This.Const == This.Const);
        cls.Operator(This.Const != This.Const);

        cls.Operator(This.Const < double());
        cls.Operator(double() < This.Const);
        cls.Operator(This.Const <= double());
        cls.Operator(double() <= This.Const);
        cls.Operator(This.Const > double());
        cls.Operator(double() > This.Const);
        cls.Operator(This.Const >= double());
        cls.Operator(double() >= This.Const);
        cls.Operator(This.Const == double());
        cls.Operator(double() == This.Const);
        cls.Operator(This.Const != double());
        cls.Operator(double() != This.Const);

        cls.Operator(This.Const + This.Const);
        cls.Operator(This.Const + int());
        cls.Operator(int() + This.Const);
        cls.Operator(This.Const + long());
        cls.Operator(long() + This.Const);
        cls.Operator(This.Const + double());
        cls.Operator(This.Const + nullptr, [](const std::string& str, std::nullptr_t) {return str + "null"; });
        cls.Operator(nullptr + This.Const, [](std::nullptr_t, const std::string& str) {return "null" + str; });
        cls.Operator(double() + This.Const);
        cls.Operator(This.Const * double());
        cls.Operator(double() * This.Const);
        cls.Operator(This.Const / double());
        cls.Operator(double() / This.Const);

        cls.Get("length", &string::length);
        cls.Function("toString", [](const std::string& arg) {return arg; });
        cls.Function("charAt", [](const string& self, size_t index) {return string(1, self.at(index)); });
        cls.Function<const char&(size_t), Const>("charCodeAt", &string::at).AddSignature("index");
        cls.Function("concat", [](string self, const vector<string>& args)
        {
            return accumulate(args.begin(), args.end(), self);
        }, Vararg);
        cls.Function("indexOf", indexOf).AddSignature("subString", Arg("startPos") = 0);
        cls.Function("lastIndexOf", lastIndexOf).AddSignature("subString", Arg("startPos") = string::npos);
        cls.Function("slice", slice1).AddSignature("start");
        cls.Function("slice", slice).AddSignature("start", "end");
        cls.Function("substring", [](const std::string& arg, int start, int end) 
        {
            if (end < start) 
                std::swap(start, end); 
            return arg.substr(start, end - start); 
        }).AddSignature("start", "end");
        cls.Function("substr", &std::string::substr).AddSignature(Arg("start") = 0, Arg("length") = std::string::npos);
        cls.Function("toLowerCase", [](const string& arg) {return boost::algorithm::to_lower_copy(arg); });
        cls.Function("toUpperCase", [](const string& arg) {return boost::algorithm::to_upper_copy(arg); });
        cls.Function("replace", [](const std::string& arg, const std::string& textToReplace, const std::string& newText) 
        {
            ixion::regex_string re(textToReplace);
            return re.replaceAll(arg, newText);

        }).AddSignature("textToReplace", "newText");
        cls.Function("split", split).AddSignature(Arg("separator") = std::string(""), Arg("maxLength") = -1);
        cls.Function("search", [](const std::string& arg, const std::string& searchString) {
            ixion::regex_string re(searchString);
            if (re.match(arg))
                return (int)re.MatchIndex;
            else
                return -1;
        }).AddSignature("searchString");
        cls.ImplicitConversion(StringToFloat<double>);
        cls.ImplicitConversion(StringToSignedInteger<short>);
        cls.ImplicitConversion(StringToSignedInteger<int>);
        cls.ImplicitConversion(StringToSignedInteger<long>);
        cls.ImplicitConversion(StringToSignedInteger<long long>);
        cls.ImplicitConversion(StringToUnsignedInteger<unsigned short>);
        cls.ImplicitConversion(StringToUnsignedInteger<unsigned int>);
        cls.ImplicitConversion(StringToUnsignedInteger<unsigned long>);
        cls.ImplicitConversion(StringToUnsignedInteger<unsigned long long>);
        cls.ImplicitConversion([](const std::string& arg) { return !arg.empty(); });
        Class<DNVS::MoFa::Reflection::Members::GlobalType> global(typeLibrary,"");
        global.StaticFunction("parseInt", StringToNumber);
        global.StaticFunction("parseFloat", ParseStringToFloat<double>);
    }
}
