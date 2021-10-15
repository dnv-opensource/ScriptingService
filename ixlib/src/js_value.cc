// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#include <cstdio>
#include <ixlib_i18n.hh>
#include <ixlib_numconv.hh>
#include <ixlib_re.hh>
#include <ixlib_string.hh>
#include <ixlib_js_internals.hh>
#include <ixlib_token_javascript.hh>
#include <limits>
#include <ixlib_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <strstream>

using namespace std;

namespace ixion { namespace javascript {

// value ----------------------------------------------------------------------
string value::toString() const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_CONVERT, (valueType2string(getType()) + string(_("-> string"))).c_str())
}

int value::toInt() const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_CONVERT, (valueType2string(getType()) + _(" -> int")).c_str())
}

double value::toFloat(bool allow_throw) const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_CONVERT, (valueType2string(getType()) + _(" -> float")).c_str())
}

bool value::toBoolean() const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_CONVERT, (valueType2string(getType()) + _(" -> bool")).c_str())
}

string value::stringify() const
{
    try
    {
        return toString();
    }
    catch (...)
    {
        return string("#<") + valueType2string(getType()) + ">";
    }
}

ref_ptr<value> value::eliminateWrappers()
{
    return this;
}

ixlib_iterator value::begin() const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": end")).c_str())
}

ixlib_iterator value::end() const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": end")).c_str())
}

TSize value::size() const
{
    return 0;
}

ref_ptr<value> value::duplicate()
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": duplication")).c_str())
}

ref_ptr<value> value::lookup(string const &identifier)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION,
                               (valueType2string(getType()) + _(": lookup of ") + identifier).c_str())
}

ref_ptr<value> value::subscript(value const &index)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": subscript")).c_str())
}

ref_ptr<value> value::call(parameter_list const &parameters)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": call")).c_str())
}

ref_ptr<value> value::callAsMethod(ref_ptr<value> instance, parameter_list const &parameters)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": call as method")).c_str())
}

ref_ptr<value> value::construct(parameter_list const &parameters)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": construction")).c_str())
}

ref_ptr<value> value::assign(ref_ptr<value> op2)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION, (valueType2string(getType()) + _(": assignment")).c_str())
}

ref_ptr<value> value::operatorUnary(operator_id op) const
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION,
                               (valueType2string(getType()) + _(": operator ") + operator2string(op)).c_str())
}

ref_ptr<value> value::operatorBinary(operator_id op, ref_ptr<value> op2) const
{
    if (op == OP_EQUAL)
    {
        if (getType() == VT_NULL)
            return makeConstant(op2->getType() == VT_NULL);
        if (op2->getType() == VT_NULL)
            return makeConstant(getType() == VT_NULL);
    }
    if (op == OP_NOT_EQUAL)
    {
        if (getType() == VT_NULL)
            return makeConstant(op2->getType() != VT_NULL);
        if (op2->getType() == VT_NULL)
            return makeConstant(getType() != VT_NULL);
    }
    EXJS_THROWINFO_NO_LOCATION(
        ECJS_INVALID_OPERATION,
        (valueType2string(getType()) + " " + string(operator2string(op)) + " " + valueType2string(op2->getType()))
            .c_str())
}

ref_ptr<value> value::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
{
    if (op == OP_LOGICAL_OR)
        return makeConstant(toBoolean() || op2.evaluate(ctx)->eliminateWrappers()->toBoolean());
    if (op == OP_LOGICAL_AND)
        return makeConstant(toBoolean() && op2.evaluate(ctx)->eliminateWrappers()->toBoolean());
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION,
                               (string(operator2string(op)) + _(" on ") + valueType2string(getType())).c_str())
}

ref_ptr<value> value::operatorUnaryModifying(operator_id op)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_INVALID_OPERATION,
                               (string(operator2string(op)) + _(" on ") + valueType2string(getType())).c_str())
}

ref_ptr<value> value::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
{
    EXJS_THROWINFO_NO_LOCATION(
        ECJS_INVALID_OPERATION,
        (valueType2string(getType()) + " " + string(operator2string(op)) + " " + valueType2string(op2->getType()))
            .c_str())
}

ref_ptr<value> value::ExecuteBoolConditional(ref_ptr<value> trueResult, ref_ptr<value> falseResult) const 
{
    if (toBoolean())
        return trueResult;
    else
        return falseResult;
}

value::operator_id value::token2operator(scanner::token const &token, bool unary, bool prefix)
{
    switch (token.Type)
    {
    case TT_JS_INCREMENT:
        return prefix ? OP_PRE_INCREMENT : OP_POST_INCREMENT;
    case TT_JS_DECREMENT:
        return prefix ? OP_PRE_DECREMENT : OP_POST_DECREMENT;
    case '+':
        return unary ? OP_UNARY_PLUS : OP_PLUS;
    case '-':
        return unary ? OP_UNARY_MINUS : OP_MINUS;
    case '!':
        return OP_LOG_NOT;
    case '~':
        return OP_BIN_NOT;
    case TT_JS_PLUS_ASSIGN:
        return OP_PLUS_ASSIGN;
    case TT_JS_MINUS_ASSIGN:
        return OP_MINUS_ASSIGN;
    case TT_JS_MULTIPLY_ASSIGN:
        return OP_MUTLIPLY_ASSIGN;
    case TT_JS_DIVIDE_ASSIGN:
        return OP_DIVIDE_ASSIGN;
    case TT_JS_MODULO_ASSIGN:
        return OP_MODULO_ASSIGN;
    case TT_JS_BIT_AND_ASSIGN:
        return OP_BIT_AND_ASSIGN;
    case TT_JS_BIT_OR_ASSIGN:
        return OP_BIT_OR_ASSIGN;
    case TT_JS_BIT_XOR_ASSIGN:
        return OP_BIT_XOR_ASSIGN;
    case TT_JS_LEFT_SHIFT_ASSIGN:
        return OP_LEFT_SHIFT_ASSIGN;
    case TT_JS_RIGHT_SHIFT_ASSIGN:
        return OP_RIGHT_SHIFT_ASSIGN;
    case '*':
        return OP_MULTIPLY;
    case '/':
        return OP_DIVIDE;
    case '%':
        return OP_MODULO;
    case '&':
        return OP_BIT_AND;
    case '|':
        return OP_BIT_OR;
    case '^':
        return OP_BIT_XOR;
    case TT_JS_LEFT_SHIFT:
        return OP_LEFT_SHIFT;
    case TT_JS_RIGHT_SHIFT:
        return OP_RIGHT_SHIFT;
    case TT_JS_LOGICAL_OR:
        return OP_LOGICAL_OR;
    case TT_JS_LOGICAL_AND:
        return OP_LOGICAL_AND;
    case TT_JS_EQUAL:
        return OP_EQUAL;
    case TT_JS_NOT_EQUAL:
        return OP_NOT_EQUAL;
    case TT_JS_IDENTICAL:
        return OP_IDENTICAL;
    case TT_JS_NOT_IDENTICAL:
        return OP_NOT_IDENTICAL;
    case TT_JS_LESS_EQUAL:
        return OP_LESS_EQUAL;
    case TT_JS_GREATER_EQUAL:
        return OP_GREATER_EQUAL;
    case '<':
        return OP_LESS;
    case '>':
        return OP_GREATER;
    default:
        EXJS_THROWINFO(ECJS_UNKNOWN_OPERATOR, token.Text.c_str())
    }
}

string value::operator2string(operator_id op)
{
    switch (op)
    {
    case OP_PRE_INCREMENT:
        return _("prefix ++");
    case OP_POST_INCREMENT:
        return _("postfix ++");
    case OP_PRE_DECREMENT:
        return _("prefix --");
    case OP_POST_DECREMENT:
        return _("postfix ++");

    case OP_UNARY_PLUS:
        return _("unary +");
    case OP_UNARY_MINUS:
        return _("unary -");
    case OP_LOG_NOT:
        return "!";
    case OP_BIN_NOT:
        return "~";

    case OP_PLUS_ASSIGN:
        return "+=";
    case OP_MINUS_ASSIGN:
        return "-=";
    case OP_MUTLIPLY_ASSIGN:
        return "*=";
    case OP_DIVIDE_ASSIGN:
        return "/=";
    case OP_MODULO_ASSIGN:
        return "%=";

    case OP_BIT_AND_ASSIGN:
        return "&=";
    case OP_BIT_OR_ASSIGN:
        return "|=";
    case OP_BIT_XOR_ASSIGN:
        return "^=";
    case OP_LEFT_SHIFT_ASSIGN:
        return "<<=";
    case OP_RIGHT_SHIFT_ASSIGN:
        return ">>=";

    case OP_PLUS:
        return "+";
    case OP_MINUS:
        return "-";
    case OP_MULTIPLY:
        return "*";
    case OP_DIVIDE:
        return "/";
    case OP_INVERSE_DIVIDE:
        return "/";
    case OP_MODULO:
        return "%";

    case OP_BIT_AND:
        return "&";
    case OP_BIT_OR:
        return "|";
    case OP_BIT_XOR:
        return "^";
    case OP_LEFT_SHIFT:
        return "<<";
    case OP_RIGHT_SHIFT:
        return ">>";

    case OP_LOGICAL_OR:
        return "||";
    case OP_LOGICAL_AND:
        return "&&";
    case OP_EQUAL:
        return "==";
    case OP_NOT_EQUAL:
        return "!=";
    case OP_IDENTICAL:
        return "===";
    case OP_NOT_IDENTICAL:
        return "!==";
    case OP_LESS_EQUAL:
        return "<=";
    case OP_GREATER_EQUAL:
        return ">=";
    case OP_LESS:
        return "<";
    case OP_GREATER:
        return ">";

    case OP_ASSIGN:
        return "=";
    default:
        EXJS_THROW(ECJS_UNKNOWN_OPERATOR)
    }
}

string value::valueType2string(value_type vt)
{
    switch (vt)
    {
    case VT_UNDEFINED:
        return _("undefined");
    case VT_NULL:
        return _("null");
    case VT_INTEGER:
        return _("integer");
    case VT_FLOATING_POINT:
        return _("double");
    case VT_STRING:
        return _("string");
    case VT_FUNCTION:
        return _("function");
    case VT_OBJECT:
        return _("object");
    case VT_BUILTIN:
        return _("built-in object");
    case VT_HOST:
        return _("host object");
    case VT_SCOPE:
        return _("scope");
    case VT_BOUND_METHOD:
        return _("bound method");
    case VT_TYPE:
        return _("type");
    default:
        return _("unknown value type");
    }
}

// value_with_methods ---------------------------------------------------------
value_with_methods::bound_method::bound_method(string const &identifier, ref_ptr<value_with_methods, value> parent)
    : Identifier(identifier), Parent(parent)
{
}

ref_ptr<value> value_with_methods::bound_method::duplicate()
{
    // bound_methods are immutable
    return this;
}

ref_ptr<value> value_with_methods::bound_method::call(parameter_list const &parameters)
{
    return Parent->callMethod(Identifier, parameters);
}

ref_ptr<value> value_with_methods::lookup(string const &identifier)
{
    ref_ptr<value> result = new bound_method(identifier, this);
    return result;
}

// null -----------------------------------------------------------------------
value::value_type null::getType() const
{
    return VT_NULL;
}

bool null::toBoolean() const
{
    return false;
}

double null::toFloat(bool allow_throw) const
{
    return 0.0;
}

int null::toInt() const
{
    return 0;
}

string null::toString() const
{
    return "null";
}

ref_ptr<value> null::duplicate()
{
    return makeNull();
}

// const_floating_point -------------------------------------------------------
const_floating_point::const_floating_point(double value) : Value(value)
{
}

value::value_type const_floating_point::getType() const
{
    return VT_FLOATING_POINT;
}

int const_floating_point::toInt() const
{
    return (int)Value;
}

double const_floating_point::toFloat(bool allow_throw) const
{
    return (double)Value;
}

bool const_floating_point::toBoolean() const
{
    return Value != 0.0;
}

string const_floating_point::toString() const
{
    return float2dec(Value,10);
}

ref_ptr<value> const_floating_point::duplicate()
{
    return makeValue(Value);
}

ref_ptr<value> const_floating_point::callMethod(string const &identifier, parameter_list const &parameters)
{
    IXLIB_JS_IF_METHOD("toInt", 0, 0)
    return makeConstant((signed long)Value);
    IXLIB_JS_IF_METHOD("toFloat", 0, 0)
    return makeConstant(Value);
    IXLIB_JS_IF_METHOD("toString", 0, 1)
    {
        unsigned radix = 10;
        if (parameters.size() == 1)
            radix = parameters[0]->toInt();
        if (radix == 10)
            return makeConstant(float2dec(Value,10));
        else
            return makeConstant(signed2base((int)Value, 0, radix));
    }
    IXLIB_JS_IF_METHOD("toFixed", 0, 1)
    {
        unsigned digits = 0;
        if (parameters.size() == 1)
            digits = parameters[0]->toInt();

        char buffer[1024];
        sprintf_s(buffer, ("%." + unsigned2dec(digits) + "f").c_str(), Value);
        return makeConstant(buffer);
    }
    IXLIB_JS_IF_METHOD("toExponential", 0, 1)
    {
        char buffer[1024];
        if (parameters.size() == 1)
            sprintf_s(buffer, ("%." + unsigned2dec(parameters[0]->toInt()) + "e").c_str(), Value);
        else
            sprintf_s(buffer, "%e", Value);
        return makeConstant(buffer);
    }
    IXLIB_JS_IF_METHOD("toPrecision", 0, 1)
    {
        if (parameters.size() == 1)
            return makeConstant(float2dec(Value, parameters[0]->toInt()));
        else
            return makeConstant(float2dec(Value,10));
    }
    EXJS_THROWINFO_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER, ("float." + identifier).c_str())
}

ref_ptr<value> const_floating_point::operatorUnary(operator_id op) const
{
    switch (op)
    {
    case OP_UNARY_PLUS:
        return makeConstant(+Value);
    case OP_UNARY_MINUS:
        return makeConstant(-Value);
    case OP_LOG_NOT:
        return makeConstant(!Value);
    case OP_BIN_NOT:
        return makeConstant(~(long)Value);
    default:
        return super::operatorUnary(op);
    }
}

ref_ptr<value> const_floating_point::operatorBinary(operator_id op, ref_ptr<value> op2) const
{
    switch (op)
    {
    case OP_PLUS:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(toString() + op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value + op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MINUS:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value - op2->toFloat());
        case VT_TYPE:
            return op2->operatorUnary(OP_UNARY_MINUS)
                ->operatorBinary(OP_PLUS, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MULTIPLY:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value * op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_DIVIDE:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
        {
            double op2value = op2->toFloat();
            if (op2value == 0)
                EXJS_THROW(ECJS_DIVISION_BY_ZERO);
            return makeConstant(Value / op2value);
        }
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MODULO:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
        {
            double op2value = op2->toFloat();
            if (op2value == 0)
                EXJS_THROW(ECJS_DIVISION_BY_ZERO);
            return makeConstant(Value - floor(Value / op2value) * op2value);
        }
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_AND:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(int(Value) & op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_OR:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(int(Value) | op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_XOR:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(int(Value) ^ op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LEFT_SHIFT:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(int(Value) << op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_RIGHT_SHIFT:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(int(Value) >> op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        {
            return makeConstant(Value == op2->toFloat());
        }
        case VT_NULL:
        {
            return makeConstant(false);
        }
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_NOT_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        {
            return makeConstant(Value != op2->toFloat());
        }
        case VT_NULL:
        {
            return makeConstant(true);
        }
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LESS_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
        {
            return makeConstant(Value <= op2->toFloat());
        }
        case VT_TYPE:
            return op2->operatorBinary(OP_GREATER_EQUAL, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_GREATER_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
        {
            return makeConstant(Value >= op2->toFloat());
        }
        case VT_TYPE:
            return op2->operatorBinary(OP_LESS_EQUAL, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LESS:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
        {
            return makeConstant(Value < op2->toFloat());
        }
        case VT_TYPE:
            return op2->operatorBinary(OP_GREATER, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_GREATER:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
        {
            return makeConstant(Value > op2->toFloat());
        }
        case VT_TYPE:
            return op2->operatorBinary(OP_LESS, ref_ptr<value>(const_cast<const_floating_point *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    default:
        return super::operatorBinary(op, op2);
    }
}

// floating_point -------------------------------------------------------------
floating_point::floating_point(double val) : const_floating_point(val)
{
}

ref_ptr<value> floating_point::operatorUnaryModifying(operator_id op)
{
    switch (op)
    {
    case OP_PRE_INCREMENT:
        Value++;
        return ref_ptr<value>(this);
    case OP_POST_INCREMENT:
        // *** FIXME this should be an lvalue
        return makeConstant(Value++);
    case OP_PRE_DECREMENT:
        Value--;
        return ref_ptr<value>(this);
    case OP_POST_DECREMENT:
        // *** FIXME this should be an lvalue
        return makeConstant(Value--);
    default:
        return super::operatorUnaryModifying(op);
    }
}

ref_ptr<value> floating_point::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
{
    int val;
    switch (op)
    {
    case OP_PLUS_ASSIGN:
        Value += op2->toFloat();
        return ref_ptr<value>(this);
    case OP_MINUS_ASSIGN:
        Value -= op2->toFloat();
        return ref_ptr<value>(this);
    case OP_MUTLIPLY_ASSIGN:
        Value *= op2->toFloat();
        return ref_ptr<value>(this);
    case OP_DIVIDE_ASSIGN:
    {
        double op2value = op2->toFloat();
        if (op2value == 0)
            EXJS_THROW(ECJS_DIVISION_BY_ZERO)
        Value /= op2value;
        return ref_ptr<value>(this);
    }
    case OP_MODULO_ASSIGN:
        val = (int)Value;
        val %= (int)op2->toFloat();
        Value = val;
        return ref_ptr<value>(this);
    case OP_BIT_AND_ASSIGN:
        val = (int)Value;
        val &= (int)op2->toFloat();
        Value = val;
        return ref_ptr<value>(this);
    case OP_BIT_OR_ASSIGN:
        val = (int)Value;
        val |= (int)op2->toFloat();
        Value = val;
        return ref_ptr<value>(this);
    case OP_BIT_XOR_ASSIGN:
        val = (int)Value;
        val ^= (int)op2->toFloat();
        Value = val;
        return ref_ptr<value>(this);
    case OP_LEFT_SHIFT_ASSIGN:
        val = (int)Value;
        val <<= (int)op2->toFloat();
        Value = val;
        return ref_ptr<value>(this);
    case OP_RIGHT_SHIFT_ASSIGN:
        val = (int)Value;
        val >>= (int)op2->toFloat();
        Value = val;
        return ref_ptr<value>(this);
    default:
        return super::operatorBinaryModifying(op, op2);
    }
}

// const_integer --------------------------------------------------------------
const_integer::const_integer(long value) : Value(value)
{
}

value::value_type const_integer::getType() const
{
    return VT_INTEGER;
}

int const_integer::toInt() const
{
    return (int)Value;
}

double const_integer::toFloat(bool allow_throw) const
{
    return (double)Value;
}

bool const_integer::toBoolean() const
{
    return Value != 0;
}

string const_integer::toString() const
{
    return signed2dec(Value);
}

ref_ptr<value> const_integer::duplicate()
{
    return makeValue(Value);
}

ref_ptr<value> const_integer::callMethod(string const &identifier, parameter_list const &parameters)
{
    IXLIB_JS_IF_METHOD("toInt", 0, 0)
    return makeConstant(Value);
    IXLIB_JS_IF_METHOD("toFloat", 0, 0)
    return makeConstant((double)Value);
    IXLIB_JS_IF_METHOD("toString", 0, 1)
    {
        unsigned radix = 10;
        if (parameters.size() == 1)
            radix = parameters[0]->toInt();
        return makeConstant(signed2base(Value, 0, radix));
    }
    EXJS_THROWINFO_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER, ("integer." + identifier).c_str())
}

ref_ptr<value> const_integer::operatorUnary(operator_id op) const
{
    switch (op)
    {
    case OP_UNARY_PLUS:
        return makeConstant(+Value);
    case OP_UNARY_MINUS:
        return makeConstant(-Value);
    case OP_LOG_NOT:
        return makeConstant(!Value);
    case OP_BIN_NOT:
        return makeConstant(~(long)Value);
    default:
        return super::operatorUnary(op);
    }
}

ref_ptr<value> const_integer::operatorBinary(operator_id op, ref_ptr<value> op2) const
{
    switch (op)
    {
    case OP_PLUS:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(toString() + op2->toString());
        case VT_FLOATING_POINT:
            return makeConstant(Value + op2->toFloat());
        case VT_INTEGER:
        case VT_NULL:
            return makeConstantFromLong(Value + op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MINUS:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
            return makeConstant(Value - op2->toFloat());
        case VT_INTEGER:
        case VT_NULL:
            return makeConstantFromLong(Value - op2->toFloat());
        case VT_TYPE:
            return op2->operatorUnary(OP_UNARY_MINUS)
                ->operatorBinary(OP_PLUS, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MULTIPLY:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
            return makeConstant(Value * op2->toFloat());
        case VT_INTEGER:
        case VT_NULL:
            return makeConstantFromLong(Value * op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_DIVIDE:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
        {
            double op2value = op2->toFloat();
            if (op2value == 0)
                EXJS_THROW(ECJS_DIVISION_BY_ZERO);
            return makeConstant(Value / op2value);
        }
        case VT_TYPE:
            return op2->operatorBinary(OP_INVERSE_DIVIDE, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MODULO:
        switch (op2->getType())
        {
        case VT_INTEGER:
            return makeConstant(Value % op2->toInt());
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_NULL:
        {
            double op2value = op2->toFloat();
            if (op2value == 0)
                EXJS_THROW(ECJS_DIVISION_BY_ZERO);
            return makeConstant(Value - floor(Value / op2value) * op2value);
        }
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_AND:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value & op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_OR:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value | op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_XOR:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value ^ op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LEFT_SHIFT:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value << op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_RIGHT_SHIFT:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value >> op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_EQUAL:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
            return makeConstant(Value == op2->toFloat());
        case VT_NULL:
            return makeConstant(false);
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_NOT_EQUAL:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
            return makeConstant(Value != op2->toFloat());
        case VT_NULL:
            return makeConstant(true);
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LESS_EQUAL:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value <= op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_GREATER_EQUAL, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_GREATER_EQUAL:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value >= op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_LESS_EQUAL, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LESS:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value < op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_GREATER, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_GREATER:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value > op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_LESS, ref_ptr<value>(const_cast<const_integer *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    default:
        return super::operatorBinary(op, op2);
    }
}

// integer --------------------------------------------------------------------
integer::integer(long val) : const_integer(val)
{
}

ref_ptr<value> integer::operatorUnaryModifying(operator_id op)
{
    switch (op)
    {
    case OP_PRE_INCREMENT:
        Value++;
        return ref_ptr<value>(this);
    case OP_POST_INCREMENT:
        // *** FIXME this should be an lvalue
        return makeConstant(Value++);
    case OP_PRE_DECREMENT:
        Value--;
        return ref_ptr<value>(this);
    case OP_POST_DECREMENT:
        // *** FIXME this should be an lvalue
        return makeConstant(Value--);
    default:
        return super::operatorUnaryModifying(op);
    }
}

ref_ptr<value> integer::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
{
    int val;
    int op2value = op2->toInt();
    switch (op)
    {
    case OP_PLUS_ASSIGN:
        Value += op2value;
        return ref_ptr<value>(this);
    case OP_MINUS_ASSIGN:
        Value -= op2value;
        return ref_ptr<value>(this);
    case OP_MUTLIPLY_ASSIGN:
        Value *= op2value;
        return ref_ptr<value>(this);
    case OP_DIVIDE_ASSIGN:
        if (op2value == 0)
            EXJS_THROW(ECJS_DIVISION_BY_ZERO)
        Value /= op2value;
        return ref_ptr<value>(this);
    case OP_MODULO_ASSIGN:
        val = Value;
        val %= op2value;
        Value = val;
        return ref_ptr<value>(this);
    case OP_BIT_AND_ASSIGN:
        val = Value;
        val &= op2value;
        Value = val;
        return ref_ptr<value>(this);
    case OP_BIT_OR_ASSIGN:
        val = Value;
        val |= op2value;
        Value = val;
        return ref_ptr<value>(this);
    case OP_BIT_XOR_ASSIGN:
        val = Value;
        val ^= op2value;
        Value = val;
        return ref_ptr<value>(this);
    case OP_LEFT_SHIFT_ASSIGN:
        val = Value;
        val <<= op2value;
        Value = val;
        return ref_ptr<value>(this);
    case OP_RIGHT_SHIFT_ASSIGN:
        val = Value;
        val >>= op2value;
        Value = val;
        return ref_ptr<value>(this);
    default:
        return super::operatorBinaryModifying(op, op2);
    }
}

// js_string ------------------------------------------------------------------
js_string::js_string(string const &value) : Value(value)
{
}

value::value_type js_string::getType() const
{
    return VT_STRING;
}

string js_string::toString() const
{
    return Value;
}

bool js_string::toBoolean() const
{
    return Value.size() != 0;
}

int gethexdigit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return c - 'A' + 10;
}

double js_string::toFloat(bool allow_throw) const
{
    string substr = Value;
    if (!allow_throw)
    {
        string::const_iterator istart = Value.begin();
        while (istart != Value.end() && isspace(*istart))
            ++istart;
        string::const_iterator iend = istart;
        while (iend != Value.end() && !isspace(*iend))
            ++iend;
        substr = string(istart, iend);
    }

    double result = 0;
    try
    {
        if (substr == "Infinity")
            result = std::numeric_limits<double>::infinity();
        else if (substr == "-Infinity")
            result = -std::numeric_limits<double>::infinity();
        else if (substr.substr(0, 2) == "0x" || substr.substr(0, 2) == "0X")
        {
            substr = substr.substr(2, -1);
            for (string::const_iterator it = substr.begin(); it != substr.end(); ++it)
            {
                if (isxdigit(*it))
                    result = result * 16 + gethexdigit(*it);
                else
                    throw exception();
            }
        }
        else
        {
            std::strstream interpreter;  // for out-of-the-box g++ 2.95.2
            if (!(interpreter << substr) || !(interpreter >> result) || !(interpreter >> std::ws).eof())
                throw exception();
        }
        return result;
    }
    catch (...)
    {
        if (allow_throw)
            throw exception();
        else
            return std::numeric_limits<double>::quiet_NaN();
    }
}

int js_string::toInt() const
{
    return int(toFloat());
}

string js_string::stringify() const
{
    return '"' + Value + '"';
}

ref_ptr<value> js_string::duplicate()
{
    return makeValue(Value);
}

ref_ptr<value> js_string::lookup(string const &identifier)
{
    if (identifier == "length")
        return makeConstant(Value.size());
    return super::lookup(identifier);
}

ref_ptr<value> js_string::callMethod(string const &identifier, parameter_list const &parameters)
{
    IXLIB_JS_IF_METHOD("toString", 0, 0)
    return makeConstant(Value);
    IXLIB_JS_IF_METHOD("charAt", 1, 1)
    return makeConstant(string(1, Value.at(parameters[0]->toInt())));
    IXLIB_JS_IF_METHOD("charCodeAt", 1, 1)
    return makeConstant(Value.at(parameters[0]->toInt()));
    if (identifier == "concat")
    {
        string result = Value;
        FOREACH_CONST (first, parameters, parameter_list)
            result += (*first)->toString();
        return makeConstant(result);
    }
    IXLIB_JS_IF_METHOD("indexOf", 1, 2)
    {
        string::size_type startpos = 0;
        if (parameters.size() == 2)
            startpos = parameters[1]->toInt();
        string::size_type result = Value.find(parameters[0]->toString(), startpos);
        if (result == string::npos)
            return makeConstant(-1);
        else
            return makeConstant(result);
    }
    IXLIB_JS_IF_METHOD("lastIndexOf", 1, 2)
    {
        string::size_type startpos = string::npos;
        if (parameters.size() == 2)
            startpos = parameters[1]->toInt();
        string::size_type result = Value.rfind(parameters[0]->toString(), startpos);
        if (result == string::npos)
            return makeConstant(-1);
        else
            return makeConstant(result);
    }
    // *** FIXME we need proper regexps
    IXLIB_JS_IF_METHOD("match", 1, 1)
    {
        regex_string re(parameters[0]->toString());
        return makeConstant(re.matchAt(Value, 0));
    }
    IXLIB_JS_IF_METHOD("replace", 2, 2)
    {
        regex_string re(parameters[0]->toString());
        return makeConstant(re.replaceAll(Value, parameters[1]->toString()));
    }
    IXLIB_JS_IF_METHOD("search", 1, 1)
    {
        regex_string re(parameters[0]->toString());
        if (re.match(Value))
            return makeConstant(re.MatchIndex);
        else
            return makeConstant(-1);
    }
    IXLIB_JS_IF_METHOD("slice", 1, 2)
    {
        int start = parameters[0]->toInt();
        int end = (int)Value.size();
        if (parameters.size() == 2)
            end = parameters[1]->toInt();
        if (end < 0)
            end = (int)Value.size() + end;
        if (start < 0)
            start = (int)Value.size() + start;
        return makeConstant(string(Value, start, end - start));
    }

    /*
      Allow 2 parameters here, the first one is the separator and the second
      one is the maximum allowed number of elements of the resulting array
      if the function is called without a parameter the string is split
      into a character array.
      The second parameter sets the maximum number of elements of the
      resulting array.
    */

    IXLIB_JS_IF_METHOD("split", 0, 2)
    {
        std::string separator;
        if (parameters.size() > 0)
            separator = parameters[0]->toString();
        int maxLength = -1;
        if (parameters.size() > 1)
            maxLength = parameters[1]->toInt();

        ref_ptr<js_array, value> result(new js_array(0));

        if (maxLength < 0)
            maxLength = (int)Value.length();
        if (maxLength == 0)
            return result;

        if (separator.empty())
        {
            for (int i = 0; i < maxLength; ++i)
                result->push_back(makeValue(string(1, Value[i])));
        }
        else
        {
            string::size_type start = 0, last = 0;
            while (true)
            {
                start = Value.find(separator, last);
                if (start != std::string::npos)
                {
                    result->push_back(makeValue(Value.substr(last, start - last)));
                    last = start + 1;
                }
                else
                {
                    result->push_back(makeValue(Value.substr(last)));
                    break;
                }
                if (result->size() >= maxLength)
                    break;
            }
        }
        return result;
    }

    IXLIB_JS_IF_METHOD("substring", 2, 2)
    {
        TIndex start = parameters[0]->toInt(), end = parameters[1]->toInt();
        if (start > end)
            swap(start, end);
        return makeConstant(string(Value, start, end - start));
    }
    IXLIB_JS_IF_METHOD("toLowerCase", 0, 0)
    {
        return makeConstant(lower(Value));
    }
    IXLIB_JS_IF_METHOD("toUpperCase", 0, 0)
    {
        return makeConstant(upper(Value));
    }
    EXJS_THROWINFO_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER, ("String." + identifier).c_str())
}

ref_ptr<value> js_string::operatorBinary(operator_id op, ref_ptr<value> op2) const
{
    switch (op)
    {
    case OP_PLUS:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(Value + op2->toString());
        case VT_TYPE:
            return makeConstant(Value + op2->toString());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MINUS:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toFloat() - op2->toFloat());
        case VT_TYPE:
            return op2->operatorUnary(OP_UNARY_MINUS)
                ->operatorBinary(OP_PLUS, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MULTIPLY:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toFloat() * op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_DIVIDE:
        switch (op2->getType())
        {
        case VT_FLOATING_POINT:
        case VT_STRING:
        case VT_INTEGER:
        case VT_NULL:
        {
            double op2value = op2->toFloat();
            if (op2value == 0)
                EXJS_THROW(ECJS_DIVISION_BY_ZERO);
            return makeConstant(toFloat() / op2value);
        }
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_MODULO:
        switch (op2->getType())
        {
        case VT_INTEGER:
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_NULL:
        {
            double op2value = op2->toFloat();
            double op1value = toFloat();
            if (op2value == 0)
                EXJS_THROW(ECJS_DIVISION_BY_ZERO);
            return makeConstant(op1value - floor(op1value / op2value) * op2value);
        }
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_AND:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toInt() & op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_OR:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toInt() | op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_BIT_XOR:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toInt() ^ op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LEFT_SHIFT:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toInt() << op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_RIGHT_SHIFT:
        switch (op2->getType())
        {
        case VT_STRING:
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toInt() >> op2->toInt());
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(Value == op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
            return makeConstant(toFloat() == op2->toFloat());
        case VT_NULL:
            return makeConstant(false);
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_NOT_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(Value != op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
            return makeConstant(toFloat() != op2->toFloat());
        case VT_NULL:
            return makeConstant(true);
        case VT_TYPE:
            return op2->operatorBinary(op, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LESS_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(Value < op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toFloat() <= op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_GREATER_EQUAL, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_GREATER_EQUAL:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(Value >= op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toFloat() >= op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_LESS_EQUAL, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_LESS:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(Value < op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toFloat() < op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_GREATER, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    case OP_GREATER:
        switch (op2->getType())
        {
        case VT_STRING:
            return makeConstant(Value > op2->toString());
        case VT_FLOATING_POINT:
        case VT_INTEGER:
        case VT_NULL:
            return makeConstant(toFloat() > op2->toFloat());
        case VT_TYPE:
            return op2->operatorBinary(OP_LESS, ref_ptr<value>(const_cast<js_string *>(this)));
        default:
            return super::operatorBinary(op, op2);
        }
    default:
        return super::operatorBinary(op, op2);
    }
}

ref_ptr<value> js_string::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
{
    switch (op)
    {
    case OP_PLUS_ASSIGN:
        Value += op2->toString();
        return ref_ptr<value>(this);
    default:
        return super::operatorBinaryModifying(op, op2);
    }
}

// lvalue ---------------------------------------------------------------------
lvalue::lvalue(ref_ptr<value> ref_ptr) : Reference(ref_ptr)
{
}

value::value_type lvalue::getType() const
{
    return Reference->getType();
}

string lvalue::toString() const
{
    return Reference->toString();
}

int lvalue::toInt() const
{
    return Reference->toInt();
}

double lvalue::toFloat(bool allow_throw) const
{
    return Reference->toFloat(allow_throw);
}

bool lvalue::toBoolean() const
{
    return Reference->toBoolean();
}

string lvalue::stringify() const
{
    return Reference->stringify();
}

ref_ptr<value> lvalue::eliminateWrappers()
{
    return Reference.get();
}

ref_ptr<value> lvalue::duplicate()
{
    return makeLValue(Reference->duplicate());
}

ref_ptr<value> lvalue::lookup(string const &identifier)
{
    return Reference->lookup(identifier);
}

ref_ptr<value> lvalue::subscript(value const &index)
{
    return Reference->subscript(index);
}

ixlib_iterator lvalue::begin() const
{
    return Reference->begin();
}

ixlib_iterator lvalue::end() const
{
    return Reference->end();
}

TSize lvalue::size() const
{
    return Reference->size();
}

ref_ptr<value> lvalue::call(parameter_list const &parameters)
{
    return Reference->call(parameters);
}

ref_ptr<value> lvalue::callAsMethod(ref_ptr<value> instance, parameter_list const &parameters)
{
    return Reference->callAsMethod(instance, parameters);
}

ref_ptr<value> lvalue::construct(parameter_list const &parameters)
{
    return Reference->construct(parameters);
}

ref_ptr<value> lvalue::assign(ref_ptr<value> op2)
{
    Reference = op2;
    return this;
}

ref_ptr<value> lvalue::operatorUnary(operator_id op) const
{
    return Reference->operatorUnary(op);
}

ref_ptr<value> lvalue::operatorBinary(operator_id op, ref_ptr<value> op2) const
{
    return Reference->operatorBinary(op, op2);
}

ref_ptr<value> lvalue::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
{
    return Reference->operatorBinaryShortcut(op, op2, ctx);
}

#define LVALUE_RETURN(VALUE)               \
    ref_ptr<value> __result = VALUE;       \
    if (__result.get() == Reference.get()) \
        return this;                       \
    else                                   \
        return __result;

ref_ptr<value> lvalue::operatorUnaryModifying(operator_id op)
{
    LVALUE_RETURN(Reference->operatorUnaryModifying(op))
}

ref_ptr<value> lvalue::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
{
    switch (op)
    {
    case OP_PLUS_ASSIGN:
        Reference = Reference->operatorBinary(OP_PLUS, op2);
        break;
    case OP_MINUS_ASSIGN:
        Reference = Reference->operatorBinary(OP_MINUS, op2);
        break;
    case OP_MUTLIPLY_ASSIGN:
        Reference = Reference->operatorBinary(OP_MULTIPLY, op2);
        break;
    case OP_DIVIDE_ASSIGN:
        Reference = Reference->operatorBinary(OP_DIVIDE, op2);
        break;
    case OP_MODULO_ASSIGN:
        Reference = Reference->operatorBinary(OP_MODULO, op2);
        break;
    case OP_BIT_AND_ASSIGN:
        Reference = Reference->operatorBinary(OP_BIT_AND, op2);
        break;
    case OP_BIT_OR_ASSIGN:
        Reference = Reference->operatorBinary(OP_BIT_OR, op2);
        break;
    case OP_BIT_XOR_ASSIGN:
        Reference = Reference->operatorBinary(OP_BIT_XOR, op2);
        break;
    case OP_LEFT_SHIFT_ASSIGN:
        Reference = Reference->operatorBinary(OP_LEFT_SHIFT, op2);
        break;
    case OP_RIGHT_SHIFT_ASSIGN:
        Reference = Reference->operatorBinary(OP_RIGHT_SHIFT, op2);
        break;
    default:
        LVALUE_RETURN(Reference->operatorBinaryModifying(op, op2));
    }
    LVALUE_RETURN(Reference);
}

// constant_wrapper -----------------------------------------------------------
constant_wrapper::constant_wrapper(ref_ptr<value> val) : Constant(val)
{
}

value::value_type constant_wrapper::getType() const
{
    return Constant->getType();
}

string constant_wrapper::toString() const
{
    return Constant->toString();
}

int constant_wrapper::toInt() const
{
    return Constant->toInt();
}

double constant_wrapper::toFloat(bool allow_throw) const
{
    return Constant->toFloat(allow_throw);
}

bool constant_wrapper::toBoolean() const
{
    return Constant->toBoolean();
}

string constant_wrapper::stringify() const
{
    return Constant->stringify();
}

ref_ptr<value> constant_wrapper::eliminateWrappers()
{
    return Constant.get();
}

ref_ptr<value> constant_wrapper::duplicate()
{
    return wrapConstant(Constant->duplicate());
}

ref_ptr<value> constant_wrapper::lookup(string const &identifier)
{
    return Constant->lookup(identifier);
}

ref_ptr<value> constant_wrapper::subscript(value const &index)
{
    return Constant->subscript(index);
}

ref_ptr<value> constant_wrapper::call(parameter_list const &parameters) const
{
    return Constant->call(parameters);
}

ref_ptr<value> constant_wrapper::callAsMethod(ref_ptr<value> instance, parameter_list const &parameters)
{
    return Constant->callAsMethod(instance, parameters);
}

ref_ptr<value> constant_wrapper::construct(parameter_list const &parameters)
{
    return Constant->construct(parameters);
}

ref_ptr<value> constant_wrapper::assign(ref_ptr<value> value)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_MODIFY_RVALUE, _("by assignment"))
}

ref_ptr<value> constant_wrapper::operatorUnary(operator_id op) const
{
    return Constant->operatorUnary(op);
}

ref_ptr<value> constant_wrapper::operatorBinary(operator_id op, ref_ptr<value> op2) const
{
    return Constant->operatorBinary(op, op2);
}

ref_ptr<value> constant_wrapper::operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const
{
    return Constant->operatorBinaryShortcut(op, op2, ctx);
}

ref_ptr<value> constant_wrapper::operatorUnaryModifying(operator_id op)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_MODIFY_RVALUE, operator2string(op).c_str())
}

ref_ptr<value> constant_wrapper::operatorBinaryModifying(operator_id op, ref_ptr<value> op2)
{
    EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_MODIFY_RVALUE, operator2string(op).c_str())
}

// list_scope -----------------------------------------------------------------
ref_ptr<value> list_scope::lookup(string const &identifier)
{
    member_map::iterator item = MemberMap.find(identifier);
    if (item != MemberMap.end())
        return item->second;

    FOREACH_CONST (first, SwallowedList, swallowed_list)
    {
        try
        {
            return (*first)->lookup(identifier);
        }
        catch (...)
        {
        }
    }
    EXJS_THROWINFO_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER, identifier.c_str())
}

void list_scope::unite(ref_ptr<value> scope)
{
    SwallowedList.push_back(scope);
}

void list_scope::setLineNumber(int lineNumber)
{
}

void list_scope::setLastLineNumber(int lineNumber)
{
}

void list_scope::separate(ref_ptr<value> scope)
{
    FOREACH (first, SwallowedList, swallowed_list)
    {
        if (*first == scope)
        {
            SwallowedList.erase(first);
            return;
        }
    }
    EXGEN_THROW(EC_ITEMNOTFOUND)
}

void list_scope::clearScopes()
{
    SwallowedList.clear();
}

bool list_scope::hasMember(string const &name) const
{
    member_map::const_iterator item = MemberMap.find(name);
    return item != MemberMap.end();
}

ref_ptr<value> list_scope::defineMember(context const& ctx, const std::string& name, ref_ptr<expression> expression, const std::shared_ptr<IValueCreatorService>& valueCreatorService)
{
    ref_ptr<value> def;
    if (expression.get() != NULL)
        def = expression->evaluate(ctx)->eliminateWrappers()->duplicate();
    else
        def = valueCreatorService->MakeUndefined();

    ref_ptr<value> lv = valueCreatorService->MakeLValue(def);
    ctx.DeclarationScope->addMember(name, lv);
    return lv;
}

void list_scope::addMember(string const &name, ref_ptr<value> member)
{
    if (hasMember(name))
        EXJS_THROWINFO_NO_LOCATION(ECJS_CANNOT_REDECLARE, name.c_str())

    MemberMap[name] = member;
}

void list_scope::removeMember(string const &name)
{
    MemberMap.erase(name);
}

void list_scope::clearMembers()
{
    member_map backup;
    std::swap(MemberMap, backup);
}

void list_scope::clear()
{
    clearScopes();
    clearMembers();
}

ref_ptr<list_scope, value> list_scope::construct()
{
    return new list_scope;
}

ref_ptr<value> list_scope::construct_unit(ref_ptr<value> opn1, ref_ptr<value> opn2)
{
    return NULL;
}

ref_ptr<value> list_scope::assignment(context const &ctx, ref_ptr<expression> opn1, ref_ptr<expression> opn2)
{
    return opn1->evaluate(ctx)->assign(opn2->evaluate(ctx)->eliminateWrappers()->duplicate());
}

ref_ptr<value> list_scope::operatorBinaryModifying(context const &ctx, ref_ptr<expression> opn1, operator_id op,
                                                   ref_ptr<expression> opn2)
{
    return opn1->evaluate(ctx)->operatorBinaryModifying(op, opn2->evaluate(ctx));
}

// This method is meant to be overloaded
std::string list_scope::getName(ref_ptr<value> object)
{
    member_map::const_iterator item;
    for (item = MemberMap.begin(); item != MemberMap.end(); item++)
    {
        if (item->second == object)
            return item->first;
    }
    EXJS_THROW_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER)
}

// callable_with_parameters ---------------------------------------------------
callable_with_parameters::callable_with_parameters(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames) 
    : ParameterNameList(pnames)
    , ValueCreator(valueCreator)
{
}

void callable_with_parameters::addParametersTo(list_scope &scope, parameter_list const &parameters) const
{
    parameter_list::const_iterator firstp = parameters.begin(), lastp = parameters.end();

    FOREACH_CONST (first, ParameterNameList, parameter_name_list)
    {
        if (firstp == lastp)
        {
            scope.addMember(*first, ValueCreator->MakeLValue(ValueCreator->MakeNull()));
            break;
        }
        else
        {
            scope.addMember(*first, ValueCreator->MakeLValue((*firstp)->eliminateWrappers()->duplicate()));
        }
        firstp++;
    }
}

ref_ptr<value> callable_with_parameters::evaluateBody(expression &body, context const &ctx)
{
    ref_ptr<value> result;

    try
    {
        result = body.evaluate(ctx);
    }
    catch (return_exception &fr)
    {
        result = fr.ReturnValue;
    }
    catch (break_exception &be)
    {
        if (be.HasLabel)
            EXJS_THROWINFOLOCATION(ECJS_INVALID_NON_LOCAL_EXIT, ("break " + be.Label).c_str(), be.Location)
        else
            EXJS_THROWINFOLOCATION(ECJS_INVALID_NON_LOCAL_EXIT, "break", be.Location)
    }
    catch (continue_exception &ce)
    {
        if (ce.HasLabel)
            EXJS_THROWINFOLOCATION(ECJS_INVALID_NON_LOCAL_EXIT, ("continue " + ce.Label).c_str(), ce.Location)
        else
            EXJS_THROWINFOLOCATION(ECJS_INVALID_NON_LOCAL_EXIT, "continue", ce.Location)
    }
    if (result.get())
        return result->eliminateWrappers()->duplicate();
    return ref_ptr<value>(NULL);
}

// function -------------------------------------------------------------------
function::function(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, context const &ctx)
    : super(valueCreator, pnames), Body(body), LexicalScope(ctx.LookupScope), Context(ctx)
{
}

ref_ptr<value> function::duplicate()
{
    // functions are not mutable
    return this;
}

ref_ptr<value> function::call(parameter_list const &parameters)
{
    ref_ptr<list_scope, value> scope = Context.DeclarationScope->construct();
    scope->unite(LexicalScope);

    addParametersTo(*scope, parameters);
    return evaluateBody(*Body, context(scope));

    // ATTENTION: this is a scope cancellation point.
}

string function::toString() const
{
    return Body->toString();
}

std::string function::stringify() const
{
    std::string result = "function(";
    for (size_t i = 0; i < ParameterNameList.size(); ++i)
    {
        if (i != 0)
            result += ", ";
        result += ParameterNameList[i];
    }
    result += ")\r\n";
    result += "{\r\n";
    result += toString();
    result += "}";
    return result;
}

// method ---------------------------------------------------------------------
method::method(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, ref_ptr<value> lex_scope,
               context const &ctx)
    : super(valueCreator, pnames), Body(body), LexicalScope(lex_scope), Context(ctx)
{
}

ref_ptr<value> method::duplicate()
{
    // methods are not mutable
    return this;
}

ref_ptr<value> method::callAsMethod(ref_ptr<value> instance, parameter_list const &parameters)
{
    ref_ptr<list_scope, value> scope = Context.DeclarationScope->construct();
    scope->unite(instance);
    scope->unite(LexicalScope);
    scope->addMember("this", instance);

    addParametersTo(*scope, parameters);
    return evaluateBody(*Body, context(scope));

    // ATTENTION: this is a scope cancellation point.
}

// constructor ----------------------------------------------------------------
constructor::constructor(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, ref_ptr<value> lex_scope,
                         context const &ctx)
    : super(valueCreator, pnames), Body(body), LexicalScope(lex_scope), Context(ctx)
{
}

ref_ptr<value> constructor::duplicate()
{
    // constructors are not mutable
    return this;
}

ref_ptr<value> constructor::callAsMethod(ref_ptr<value> instance, parameter_list const &parameters)
{
    ref_ptr<list_scope, value> scope = Context.DeclarationScope->construct();
    scope->unite(LexicalScope);
    scope->unite(instance);
    scope->addMember("this", instance);

    addParametersTo(*scope, parameters);
    return evaluateBody(*Body, context(scope));

    // ATTENTION: this is a scope cancellation point.
}

// js_class -------------------------------------------------------------------
js_class::super_instance_during_construction::super_instance_during_construction(ref_ptr<value> super_class)
    : SuperClass(super_class)
{
}

ref_ptr<value> js_class::super_instance_during_construction::call(parameter_list const &parameters)
{
    if (SuperClassInstance.get())
        EXJS_THROW_NO_LOCATION(ECJS_DOUBLE_CONSTRUCTION)

    SuperClassInstance = SuperClass->construct(parameters);
    return SuperClassInstance;
}

ref_ptr<value> js_class::super_instance_during_construction::lookup(string const &identifier)
{
    return getSuperClassInstance()->lookup(identifier);
}

ref_ptr<value> js_class::super_instance_during_construction::getSuperClassInstance()
{
    if (SuperClassInstance.get())
        return SuperClassInstance;

    SuperClassInstance = SuperClass->construct(parameter_list());
    return SuperClassInstance;
}

js_class::js_class(ref_ptr<value> lex_scope, ref_ptr<value> super_class, ref_ptr<value> constructor,
                   ref_ptr<value> static_method_scope, ref_ptr<value> method_scope,
                   ref_ptr<value> static_variable_scope, declaration_list const &variable_list, context const &ctx)
    : LexicalScope(lex_scope),
      SuperClass(super_class),
      Constructor(constructor),
      StaticMethodScope(static_method_scope),
      MethodScope(method_scope),
      StaticVariableScope(static_variable_scope),
      VariableList(variable_list),
      Context(ctx)
{
}

ref_ptr<value> js_class::duplicate()
{
    // classes are not mutable
    return this;
}

ref_ptr<value> js_class::lookup(string const &identifier)
{
    try
    {
        return lookupLocal(identifier);
    }
    catch (...)
    {
    }

    if (SuperClass.get())
        return SuperClass->lookup(identifier);

    EXJS_THROWINFO_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER, identifier.c_str())
}

ref_ptr<value> js_class::lookupLocal(string const &identifier)
{
    try
    {
        return StaticMethodScope->lookup(identifier);
    }
    catch (...)
    {
    }

    return StaticVariableScope->lookup(identifier);
}

ref_ptr<value> js_class::construct(parameter_list const &parameters)
{
    ref_ptr<list_scope, value> vl(Context.DeclarationScope->construct());

    ref_ptr<js_class_instance, value> instance(new js_class_instance(this, MethodScope, vl.get()));

    ref_ptr<list_scope, value> construction_scope(Context.DeclarationScope->construct());
    construction_scope->unite(LexicalScope);
    construction_scope->unite(instance.get());

    FOREACH_CONST (first, VariableList, declaration_list)
        (*first)->evaluate(context(vl, construction_scope.get()));

    ref_ptr<super_instance_during_construction, value> temp_super;
    if (SuperClass.get())
    {
        temp_super = new super_instance_during_construction(SuperClass);
        vl->addMember("super", temp_super.get());
    }

    if (Constructor.get())
        Constructor->callAsMethod(instance.get(), parameters);

    if (temp_super.get())
    {
        ref_ptr<value> super = temp_super->getSuperClassInstance();
        vl->removeMember("super");
        instance->setSuperClassInstance(super);
        vl->addMember("super", super);
    }
    return instance.get();
}

// js_class_instance ----------------------------------------------------------
js_class_instance::bound_method::bound_method(ref_ptr<value> instance, ref_ptr<value> method)
    : Instance(instance), Method(method)
{
}

ref_ptr<value> js_class_instance::bound_method::call(parameter_list const &parameters)
{
    return Method->callAsMethod(Instance, parameters);
}

js_class_instance::js_class_instance(ref_ptr<js_class, value> cls, ref_ptr<value> method_scope,
                                     ref_ptr<value> variable_scope)
    : Class(cls), MethodScope(method_scope), VariableScope(variable_scope)
{
}

void js_class_instance::setSuperClassInstance(ref_ptr<value> super_class_instance)
{
    SuperClassInstance = super_class_instance;
}

ref_ptr<value> js_class_instance::duplicate()
{
    return this;
}

ref_ptr<value> js_class_instance::lookup(string const &identifier)
{
    try
    {
        ref_ptr<value> method = MethodScope->lookup(identifier);
        ref_ptr<value> bound = new bound_method(this, method);
        return bound;
    }
    catch (...)
    {
    }

    try
    {
        return VariableScope->lookup(identifier);
    }
    catch (...)
    {
    }

    try
    {
        return Class->lookupLocal(identifier);
    }
    catch (...)
    {
    }

    if (SuperClassInstance.get())
        return SuperClassInstance->lookup(identifier);

    EXJS_THROWINFO_NO_LOCATION(ECJS_UNKNOWN_IDENTIFIER, identifier.c_str())
}

// value creation -------------------------------------------------------------
ref_ptr<value> javascript::makeUndefined()
{
    // *** FIXME: this is non-compliant
    ref_ptr<value> result(new null());
    return result;
}

ref_ptr<value> javascript::makeNull()
{
    ref_ptr<value> result(new null());
    return result;
}

ref_ptr<value> javascript::makeValue(signed int val)
{
    ref_ptr<value> result(new integer(val));
    return result;
}

ref_ptr<value> javascript::makeConstant(signed int val)
{
    ref_ptr<value> result(new const_integer(val));
    return result;
}

ref_ptr<value> javascript::makeValue(unsigned int val)
{
    ref_ptr<value> result(new integer(val));
    return result;
}

ref_ptr<value> javascript::makeConstant(unsigned int val)
{
    ref_ptr<value> result(new const_integer(val));
    return result;
}

// this function is added as to get it work on 64bits platform
// jizhi@2011,April
ref_ptr<value> javascript::makeConstant(unsigned __int64 val)
{
    ref_ptr<value> result(new const_integer((unsigned long)val));
    return result;
}

ref_ptr<value> javascript::makeValue(signed long val)
{
    ref_ptr<value> result(new integer(val));
    return result;
}

ref_ptr<value> javascript::makeConstant(signed long val)
{
    ref_ptr<value> result(new const_integer(val));
    return result;
}

ref_ptr<value> javascript::makeValue(unsigned long val)
{
    ref_ptr<value> result(new integer(val));
    return result;
}

ref_ptr<value> javascript::makeConstant(unsigned long val)
{
    ref_ptr<value> result(new const_integer(val));
    return result;
}

ref_ptr<value> javascript::makeValue(double val)
{
    ref_ptr<value> result(new floating_point(val));
    return result;
}

ref_ptr<value> javascript::makeConstant(double val)
{
    ref_ptr<value> result(new const_floating_point(val));
    return result;
}

ref_ptr<value> javascript::makeValue(string const &val)
{
    ref_ptr<value> result(new js_string(val));
    return result;
}

ref_ptr<value> javascript::makeConstant(string const &val)
{
    return wrapConstant(makeValue(val));
}

ref_ptr<value> javascript::makeConstantFromLong(double val)
{
    if (val > std::numeric_limits<long>::max())
        return makeConstant(val);
    else if (val < std::numeric_limits<long>::min())
        return makeConstant(val);
    else
        return makeConstant(long(val));
}

ref_ptr<value> javascript::makeArray(TSize size)
{
    auto_ptr<js_array> result(new js_array(size));
    return result.release();
}

ref_ptr<value> javascript::makeLValue(ref_ptr<value> target)
{
    ref_ptr<value> result = new lvalue(target);
    return result;
}

ref_ptr<value> javascript::wrapConstant(ref_ptr<value> val)
{
    ref_ptr<value> result(new constant_wrapper(val));
    return result;
}

}}
