// ----------------------------------------------------------------------------
//  Description      : Numeric conversions
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#include <ixlib_exgen.hh>
#include <ixlib_numconv.hh>
#include <stdio.h>
#include <boost/spirit/home/karma.hpp>
#include "Services/SpiritKarmaFix.h"

template <typename T>
struct script_real_policies : boost::spirit::karma::real_policies<T>
{
    script_real_policies(unsigned precision) : m_precision(precision) {}
    unsigned precision(T n) const { return m_precision; }
private:
    unsigned m_precision;
};

namespace boost { namespace spirit { namespace karma 
{
	template <typename T, typename CharEncoding, typename Tag>
	struct real_inserter<T, script_real_policies<T>, CharEncoding, Tag> : real_inserter_workaround<T, script_real_policies<T>, CharEncoding, Tag> {};
}}}

std::string DoubleToString(double value, unsigned precision)
{
    namespace karma = boost::spirit::karma;
    using karma::double_;
    using karma::_1;
    std::string str;
    std::back_insert_iterator<std::string> sink(str);
    karma::generate(sink, karma::real_generator<double, script_real_policies<double>>(precision), value);
    return str;
}

using namespace std;
using namespace ixion;

// data objects --------------------------------------------------------------
static string numChars = IXLIB_NUMCHARS;

// exported subroutines -------------------------------------------------------
string ixion::float2dec(double value)
{
    char buf[255];
    sprintf_s(buf, "%f", value);

    return string(buf);
}

string ixion::float2dec(double value, unsigned int precision)
{
    return DoubleToString(value, precision);
}

string ixion::unsigned2base(TUnsigned64 value, char digits, char radix)
{
    string temp;

    do
    {
        temp = numChars[TSize(value % radix)] + temp;
        value /= radix;
        if (digits)
            digits--;
    } while (value || digits);

    return temp;
}

string ixion::signed2base(TSigned64 value, char digits, char radix)
{
    if (value < 0)
        return "-" + unsigned2base(-value, digits, radix);
    else
        return unsigned2base(value, digits, radix);
}

string ixion::bytes2dec(TSize bytes)
{
    if (bytes > (TSize)10 * 1024 * 1024)
        return unsigned2dec(bytes / ((TSize)1024 * 1024)) + " MB";
    if (bytes > (TSize)10 * 1024)
        return unsigned2dec(bytes / ((TSize)1024)) + " KB";

    return unsigned2dec(bytes) + " Byte";
}

unsigned long ixion::evalNumeral(string const &numeral, unsigned radix)
{
    string numstr = upper(numeral);

    if (numstr.size() == 0)
        return 0;
    unsigned long value = 0, mulvalue = 1;
    TIndex index = numstr.size() - 1;

    do
    {
        string::size_type digvalue = numChars.find(numstr[index]);
        if (digvalue == string::npos)
            EXGEN_THROWINFO(EC_CANNOTEVALUATE, numstr.c_str())
        value += (unsigned)(mulvalue * digvalue);
        mulvalue *= radix;
    } while (index--);

    return value;
}

double ixion::evalFloat(string const &numeral)
{
    double result;
    int count = sscanf_s(numeral.c_str(), "%le", &result);

    if (count == 0)
        EXGEN_THROWINFO(EC_CANNOTEVALUATE, numeral.c_str())
    else
        return result;
}

unsigned long ixion::evalUnsigned(string const &numeral, unsigned default_base)
{
    if (numeral.size() == 0)
        return 0;

    if (numeral.substr(0, 2) == "0X" || numeral.substr(0, 2) == "0x")
        return evalNumeral(numeral.substr(2), 0x10);
    if (numeral.substr(0, 1) == "$")
        return evalNumeral(numeral.substr(1), 0x10);

    char lastchar = numeral[numeral.size() - 1];
    if (lastchar == 'H' || lastchar == 'h')
        return evalNumeral(numeral.substr(0, numeral.size() - 1), 0x10);
    if (lastchar == 'B' || lastchar == 'b')
        return evalNumeral(numeral.substr(0, numeral.size() - 1), 2);
    if (lastchar == 'D' || lastchar == 'd')
        return evalNumeral(numeral.substr(0, numeral.size() - 1), 10);
    if (lastchar == 'O' || lastchar == 'o')
        return evalNumeral(numeral.substr(0, numeral.size() - 1), 8);

    return evalNumeral(numeral, default_base);
}

signed long ixion::evalSigned(string const &numeral, unsigned default_base)
{
    if (numeral.size() == 0)
        return 0;
    if (numeral[0] == '-')
        return -(signed long)evalUnsigned(numeral.substr(1), default_base);
    else
    {
        if (numeral[0] == '+')
            return evalUnsigned(numeral.substr(1), default_base);
        else
            return evalUnsigned(numeral, default_base);
    }
}
