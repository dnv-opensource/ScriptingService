// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter library
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <ixlib_js_internals.hh>
#include <ixlib_numconv.hh>
#include <limits>

using namespace ixion;
using namespace javascript;

namespace {

class eval : public value
{
protected:
    interpreter &Interpreter;

public:
    value_type getType() const
    {
        return VT_FUNCTION;
    }

    eval(interpreter &interpreter) : Interpreter(interpreter)
    {
    }

    ref_ptr<value> call(parameter_list const &parameters);
};

}

// eval -----------------------------------------------------------------------
ref_ptr<value> eval::call(parameter_list const &parameters)
{
    if (parameters.size() != 1)
    {
        EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, "eval")
    }

    if (parameters[0]->getType() != VT_STRING)
        return parameters[0];

    return Interpreter.execute(parameters[0]->toString());
}

// parseInt -------------------------------------------------------------------
IXLIB_JS_DECLARE_FUNCTION(parseInt)
{
    if (parameters.size() != 1 && parameters.size() != 2)
    {
        EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, "parseInt")
    }

    unsigned radix = 10;
    if (parameters.size() == 2)
        radix = parameters[1]->toInt();

    return makeConstant(evalSigned(parameters[0]->toString(), radix));
}

// parseFloat -----------------------------------------------------------------
IXLIB_JS_DECLARE_FUNCTION(parseFloat)
{
    if (parameters.size() != 1)
    {
        EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, "parseFloat")
    }

    return makeConstant(evalFloat(parameters[0]->toString()));
}

// isNaN ----------------------------------------------------------------------
#ifdef ADVANCED_MATH_AVAILABLE
IXLIB_JS_DECLARE_FUNCTION(isNaN)
{
    if (parameters.size() != 1)
    {
        EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, "isNaN")
    }

    int classification = fpclassify(parameters[0]->toFloat());

    return makeConstant(classification == FP_NAN);
}
#endif

// isFinite -------------------------------------------------------------------
#ifdef ADVANCED_MATH_AVAILABLE
IXLIB_JS_DECLARE_FUNCTION(isFinite)
{
    if (parameters.size() != 1)
    {
        EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, "isFinite")
    }

    int classification = fpclassify(parameters[0]->toFloat());

    return makeConstant(classification != FP_NAN && classification != FP_INFINITE);
}
#endif

// external interface functions -----------------------------------------------
#define ADD_GLOBAL_OBJECT(NAME, TYPE)     \
    {                                     \
        ref_ptr<value> x = new TYPE();    \
        ip.RootScope->addMember(NAME, x); \
    }

void javascript::addGlobal(interpreter &ip)
{
    ref_ptr<value> ev = new eval(ip);
    ip.RootScope->addMember("eval", ev);

    ADD_GLOBAL_OBJECT("parseInt", parseInt)
    ADD_GLOBAL_OBJECT("parseFloat", parseFloat)
#ifdef ADVANCED_MATH_AVAILABLE
    ADD_GLOBAL_OBJECT("isNaN", isNaN)
    ADD_GLOBAL_OBJECT("isFinite", isFinite)
#endif

    // *** FIXME hope this is portable
    ip.RootScope->addMember("NaN", makeConstant(std::numeric_limits<double>::quiet_NaN()));
    ip.RootScope->addMember("Infinity", makeConstant(std::numeric_limits<double>::infinity()));
    ip.RootScope->addMember("undefined", makeUndefined());
}
