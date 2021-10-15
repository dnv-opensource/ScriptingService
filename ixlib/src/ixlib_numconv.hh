// ----------------------------------------------------------------------------
//  Description      : Numeric conversions
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#ifndef IXLIB_NUMCONV
#define IXLIB_NUMCONV

#include <ixlib_base.hh>
#include <ixlib_string.hh>

// Macros ---------------------------------------------------------------------
#define IXLIB_NUMCHARS "0123456789ABCDEF"

// Functions ------------------------------------------------------------------
namespace ixion {

std::string float2dec(double value);
std::string float2dec(double value, unsigned int precision);
std::string unsigned2base(TUnsigned64 value, char digits = 0, char radix = 10);

inline std::string unsigned2dec(TUnsigned64 value, char digits = 0)
{
    return unsigned2base(value, digits, 10);
}

inline std::string unsigned2hex(TUnsigned64 value, char digits = 0)
{
    return unsigned2base(value, digits, 16);
}

inline std::string unsigned2bin(TUnsigned64 value, char digits = 0)
{
    return unsigned2base(value, digits, 2);
}

inline std::string unsigned2oct(TUnsigned64 value, char digits = 0)
{
    return unsigned2base(value, digits, 8);
}

std::string signed2base(TSigned64 value, char digits = 0, char radix = 10);

inline std::string signed2dec(TSigned64 value, char digits = 0)
{
    return signed2base(value, digits, 10);
}

inline std::string signed2hex(TSigned64 value, char digits = 0)
{
    return signed2base(value, digits, 16);
}

inline std::string signed2bin(TSigned64 value, char digits = 0)
{
    return signed2base(value, digits, 2);
}

inline std::string signed2oct(TSigned64 value, char digits = 0)
{
    return signed2base(value, digits, 8);
}

std::string bytes2dec(TSize bytes);

unsigned long evalNumeral(std::string const &numeral, unsigned radix = 10);
double evalFloat(std::string const &numeral);
unsigned long evalUnsigned(std::string const &numeral, unsigned default_base = 10);
signed long evalSigned(std::string const &numeral, unsigned default_base = 10);

}

#endif
