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
#pragma warning(disable: 4018)
#include "Number.h"
#include "Reflection\Classes\Class.h"
#include "Reflection\Attributes\UndefinedAttribute.h"
#include <boost/spirit/home/karma.hpp>
#include <math.h>
#include "Services\SpiritKarmaFix.h"

namespace boost { namespace spirit { namespace karma {
	template <typename T>
	struct script_real_policies : boost::spirit::karma::real_policies<T>
	{
		unsigned precision(T n) const { return 10; }
	};

	template <typename T>
	struct fixed_real_policies : boost::spirit::karma::real_policies<T>
	{
		fixed_real_policies(unsigned precision) : m_precision(precision) {}
		unsigned precision(T n) const { return m_precision; }
		int floatfield(T n) const { return fmtflags::fixed; }
		template <typename OutputIterator>
		bool dot(OutputIterator& sink, T /*n*/, unsigned /*precision*/) const
		{
			if (m_precision == 0)
				return true;
			else
				return boost::spirit::karma::char_inserter<>::call(sink, '.');  // generate the dot by default 
		}

	private:
		unsigned m_precision;
	};
	template <typename T>
	struct exponential_real_policies : boost::spirit::karma::real_policies<T>
	{
		exponential_real_policies(unsigned precision) : m_precision(precision) {}
		unsigned precision(T n) const { return m_precision; }
		int floatfield(T n) const { return fmtflags::scientific; }
		template <typename OutputIterator>
		bool dot(OutputIterator& sink, T /*n*/, unsigned /*precision*/) const
		{
			if (m_precision == 0)
				return true;
			else
				return boost::spirit::karma::char_inserter<>::call(sink, '.');  // generate the dot by default 
		}
		template <typename CharEncoding, typename Tag, typename OutputIterator>
		bool exponent(OutputIterator& sink, long n) const
		{
			using namespace boost::spirit;
			long abs_n = traits::get_absolute_value(n);
			bool r = karma::char_inserter<CharEncoding, Tag>::call(sink, 'e') &&
				karma::sign_inserter::call(sink, traits::test_zero(n)
					, traits::test_negative(n), true, true);

			// the C99 Standard requires at least two digits in the exponent
			if (r && abs_n < 10)
				r = karma::char_inserter<CharEncoding, Tag>::call(sink, '0');
			return r && karma::int_inserter<10>::call(sink, abs_n);
		}
	private:
		unsigned m_precision;
	};
	template <typename T>
	struct precision_real_policies : boost::spirit::karma::real_policies<T>
	{
		precision_real_policies(unsigned precision) : m_precision(precision) {}
		unsigned precision(T n) const { return m_precision; }
	private:
		unsigned m_precision;
	};
	template <typename T, typename CharEncoding, typename Tag>
	struct real_inserter<T, script_real_policies<T>, CharEncoding, Tag> : real_inserter_workaround<T, script_real_policies<T>, CharEncoding, Tag> {};
	template <typename T, typename CharEncoding, typename Tag>
	struct real_inserter<T, fixed_real_policies<T>, CharEncoding, Tag> : real_inserter_workaround<T, fixed_real_policies<T>, CharEncoding, Tag> {};
	template <typename T, typename CharEncoding, typename Tag>
	struct real_inserter<T, exponential_real_policies<T>, CharEncoding, Tag> : real_inserter_workaround<T, exponential_real_policies<T>, CharEncoding, Tag> {};
	template <typename T, typename CharEncoding, typename Tag>
	struct real_inserter<T, precision_real_policies<T>, CharEncoding, Tag> : real_inserter_workaround<T, precision_real_policies<T>, CharEncoding, Tag> {};
}}}

namespace std {
	template <typename T>
	std::string NumberToString(T const& value)
	{
		namespace karma = boost::spirit::karma;
		std::string str;
		std::back_insert_iterator<std::string> sink(str);
		karma::generate(sink, value);
		return str;
	}

	std::string DoubleToString(double value)
	{
		namespace karma = boost::spirit::karma;
		using karma::double_;
		using karma::_1;
		std::string str;
		std::back_insert_iterator<std::string> sink(str);
		karma::generate(sink, karma::real_generator<double, karma::script_real_policies<double>>(), value);
		return str;
	}

	template<typename T, typename... Args>
	void DoReflectConversion(DNVS::MoFa::Reflection::Classes::Class<T, Args...>& cls)
	{
		using namespace DNVS::MoFa::Reflection::Classes;
		cls.ImplicitConversion(&NumberToString<T>);
		cls.Function("toString", &NumberToString<T>);
		cls.Function("toInt", [](T x) {return (int)x; });
		cls.Function("toFloat", [](T x) {return (double)x; });
	}



	std::string ToFixed(double value, int precision)
	{
		namespace karma = boost::spirit::karma;
		using karma::double_;
		using karma::_1;
		std::string str;
		std::back_insert_iterator<std::string> sink(str);
		karma::generate(sink, karma::real_generator<double, karma::fixed_real_policies<double>>(karma::fixed_real_policies<double>(precision)), value);
		return str;
	}

    std::string ToExponential(double value, int precision)
    {
        namespace karma = boost::spirit::karma;
        using karma::double_;
        using karma::_1;
        std::string str;
        std::back_insert_iterator<std::string> sink(str);
        karma::generate(sink, karma::real_generator<double, karma::exponential_real_policies<double>>(karma::exponential_real_policies<double>(precision)), value);
        return str;
    }

    std::string ToPrecision(double value, int precision)
    {
        namespace karma = boost::spirit::karma;
        using karma::double_;
        using karma::_1;
        std::string str;
        std::back_insert_iterator<std::string> sink(str);
        karma::generate(sink, karma::real_generator<double, karma::precision_real_policies<double>>(karma::precision_real_policies<double>(precision)), value);
        return str;
    }

    template<typename... Args>
    void DoReflectConversion(DNVS::MoFa::Reflection::Classes::Class<double, Args...>& cls)
    {
        using namespace DNVS::MoFa::Reflection::Classes;
        cls.ImplicitConversion(&DoubleToString);
        cls.Function("ToString", &DoubleToString);
        cls.Function("toInt", [](double x) {return (int)x; });
        cls.Function("toFloat", [](double x) {return (double)x; });
        cls.Function("toFixed", ToFixed).AddSignature(Arg("precision") = 0);
        cls.Function("toExponential", ToExponential).AddSignature(Arg("precision") = 6);
        cls.Function("toPrecision", ToPrecision).AddSignature(Arg("precision") = 10);
    }

    template<typename... Args>
    void DoReflectConversion(DNVS::MoFa::Reflection::Classes::Class<bool, Args...>& cls)
    {
        cls.ImplicitConversion([](bool val) ->std::string {return val ? "true" : "false"; });
        cls.Function("toString", [](bool val) ->std::string {return val ? "true" : "false"; });
        cls.Function("toInt", [](bool x) {return (int)x; });
        cls.Function("toFloat", [](bool x) {return (double)x; });
    }

    template<typename... Args>
    void DoReflectConversion(DNVS::MoFa::Reflection::Classes::Class<char, Args...>& cls)
    {
        cls.ImplicitConversion([](char c) {return std::string(1, c); });
        cls.Function("toString", [](char c) {return std::string(1, c); });
        cls.Function("toInt", [](char x) {return (int)x; });
        cls.Function("toFloat", [](char x) {return (double)x; });
    }

    using namespace DNVS::MoFa::Reflection;

    template<typename Callback>
    void ForAllNumericTypes(Callback c)
    {
        c(unsigned char());
        c(signed char());
        c(unsigned short());
        c(signed short());
        c(unsigned int());
        c(signed int());
        c(unsigned long());
        c(signed long());
        c(unsigned __int64());
        c(signed __int64());
        c(float());
        c(double());
    }

    template<typename T1, typename T2, typename R>
    R ComputeRemainder(T1 lhs, T2 rhs)
    {
        if (_isnan((R)lhs) || _isnan((R)rhs))
            return std::numeric_limits<R>::quiet_NaN();
        R dividend = fabs(lhs);
        R divisor = fabs(rhs);
        if (dividend == std::numeric_limits<R>::infinity() || divisor == R(0))
            return std::numeric_limits<R>::quiet_NaN();
        if (divisor == std::numeric_limits<R>::infinity())
            return (R)lhs;
        if (dividend == R(0))
            return (R)lhs;
        size_t q = (size_t)(dividend / divisor);
        auto r = dividend - divisor * q;
        if (lhs < R(0))
            return -r;
        else
            return r;
    }

    template<typename ClassT>
    void DoReflectCompareOperators(ClassT& cls)
    {
        using namespace Classes;
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const < other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const <= other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const >= other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const > other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const == other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const != other); });
        cls.Operator(!This.Const);
    }

    template<typename ClassT>
    void DoReflectCompareOperatorsBool(ClassT& cls)
    {
        using namespace Classes;
        cls.Operator(This.Const < This.Const);
        cls.Operator(This.Const <= This.Const);
        cls.Operator(This.Const >= This.Const);
        cls.Operator(This.Const > This.Const);
        cls.Operator(This.Const == This.Const);
        cls.Operator(This.Const != This.Const);
        cls.Operator(!This.Const);
    }

    template<typename ClassT>
    void DoReflectBitwiseOperators(ClassT& cls)
    {
        using namespace Classes;
        cls.Operator(This.Const | This.Const);
        cls.Operator(This.Const & This.Const);
        cls.Operator(This.Const ^ This.Const);
        cls.Operator(This.Const << This.Const);
        cls.Operator(This.Const >> This.Const);
        cls.Operator(~This.Const);
    }
    template<typename UnsignedT>
    class UnsignedInteger {
    public:
        UnsignedInteger(UnsignedT val) : m_unsigned(val) {}
        typedef std::make_signed_t<UnsignedT> SignedType;
        SignedType operator-() const {return -SignedType(m_unsigned);}
    private:
        UnsignedT m_unsigned;
    };
    template<typename T>
    auto TryToSigned(T value) { return std::make_signed_t<T>(value); }

    auto TryToSigned(float value) { return value; }
    auto TryToSigned(double value) { return value; }

    template<typename ClassT>
    void DoReflectArithmeticOperatorsUnsignedIntegral(ClassT& cls, const TypeLibraries::TypeLibraryPointer& typeLibrary)
    {
        typedef typename ClassT::ReflectedType UnsignedType;
        using namespace Classes;
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const + other); });
        ForAllNumericTypes([&](auto other) {
            using Other = decltype(other);
            cls.Operator(This.Const - other, [](UnsignedType lhs, Other rhs) { return std::make_signed_t<UnsignedType>(lhs) - TryToSigned(rhs); });
        });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const * other); });
        cls.Operator(This.Const / double());
        cls.Operator(This.Const % double(), ComputeRemainder<UnsignedType, double, double>);
        cls.Operator(+This.Const);
        cls.Operator(-Other<UnsignedInteger<UnsignedType>>());
        Class<UnsignedInteger<UnsignedType>> unsignedWrapper(typeLibrary,"UnsignedInterger " + cls.GetType()->GetName());
        unsignedWrapper.Constructor<UnsignedType>(Alias);
    }
    template<typename ClassT>
    void DoReflectArithmeticOperatorsSignedIntegral(ClassT& cls)
    {
        using SignedType = typename ClassT::ReflectedType;
        using namespace Classes;
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const + other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const + other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const - other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const * other); });
        cls.Operator(This.Const / double());
        cls.Operator(This.Const % double(), ComputeRemainder<SignedType, double, double>);
        cls.Operator(-This.Const);
        cls.Operator(+This.Const);
    }
    template<typename ClassT>
    void DoReflectArithmeticOperatorsFloat(ClassT& cls)
    {
        using namespace Classes;
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const + other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const - other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const * other); });
        ForAllNumericTypes([&](auto other) {cls.Operator(This.Const / other); });
        cls.Operator(-This.Const);
        cls.Operator(+This.Const);
    }

    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, int**)
    {
        using namespace Classes;
        Class<int, int> cls(typeLibrary, "int");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsSignedIntegral(cls);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned int**)
    {
        using namespace Classes;
        Class<unsigned int, unsigned int> cls(typeLibrary, "unsigned int");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsUnsignedIntegral(cls, typeLibrary);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, long**)
    {
        using namespace Classes;
        Class<long, long> cls(typeLibrary, "long");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsSignedIntegral(cls);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned long**)
    {
        using namespace Classes;
        Class<unsigned long, unsigned long> cls(typeLibrary, "unsigned long");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsUnsignedIntegral(cls, typeLibrary);
        DoReflectConversion(cls);
    }

    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, __int64**)
    {
        using namespace Classes;
        Class<__int64, __int64> cls(typeLibrary, "int64");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsSignedIntegral(cls);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned __int64**)
    {
        using namespace Classes;
        Class<unsigned __int64, unsigned __int64> cls(typeLibrary, "unsigned __int64");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsUnsignedIntegral(cls, typeLibrary);
        DoReflectConversion(cls);
    }

    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, short**)
    {
        using namespace Classes;
        Class<short, short> cls(typeLibrary, "short");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsSignedIntegral(cls);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned short**)
    {
        using namespace Classes;
        Class<unsigned short, unsigned short> cls(typeLibrary, "unsigned short");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsUnsignedIntegral(cls, typeLibrary);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, char**)
    {
        using namespace Classes;
        Class<char, char> cls(typeLibrary, "char");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsSignedIntegral(cls);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, unsigned char**)
    {
        using namespace Classes;
        Class<unsigned char, unsigned char> cls(typeLibrary, "unsigned char");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsUnsignedIntegral(cls, typeLibrary);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, signed char**)
    {
        using namespace Classes;
        Class<char, char> cls(typeLibrary, "char");
        DoReflectCompareOperators(cls);
        DoReflectBitwiseOperators(cls);
        DoReflectArithmeticOperatorsSignedIntegral(cls);
        DoReflectConversion(cls);
    }

    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, bool**)
    {
        using namespace Classes;
        Class<bool, bool> cls(typeLibrary, "bool");
        DoReflectCompareOperatorsBool(cls);
        cls.Operator(This.Const | This.Const);
        cls.Operator(This.Const & This.Const);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, float**)
    {
        using namespace Classes;
        Class<float, float> cls(typeLibrary, "float");
        cls.AddAttribute<UndefinedAttribute>([](float val) {return !!_isnanf(val); });
        cls.Operator(This.Const % This.Const, ComputeRemainder<float, float, float>);
        DoReflectCompareOperators(cls);
        DoReflectArithmeticOperatorsFloat(cls);
        DoReflectConversion(cls);
    }
    void DoReflect(const TypeLibraries::TypeLibraryPointer& typeLibrary, double**)
    {
        using namespace Classes;
        Class<double, double> cls(typeLibrary, "double");
        cls.AddAttribute<UndefinedAttribute>([](double val) {return !!_isnan(val); });
        cls.Operator(This.Const % This.Const, ComputeRemainder<double, double, double>);
        DoReflectCompareOperators(cls);
        DoReflectArithmeticOperatorsFloat(cls);
        DoReflectConversion(cls);
        using namespace Classes;
        Class<Members::GlobalType> global(typeLibrary, "");
        global.StaticFunction("isNaN", isnan<double>);
        global.StaticFunction("isFinite", isfinite<double>);
        global.StaticGet("Infinity", std::numeric_limits<double>::infinity());
        global.StaticGet("NaN", std::numeric_limits<double>::quiet_NaN());
    }
}
