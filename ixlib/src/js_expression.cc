// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#include <ixlib_js_internals.hh>
#include <ixlib_token_javascript.hh>

#define EXJS_ADD_CODE_LOCATION                               \
    catch (no_location_javascript_exception & half)          \
    {                                                        \
        throw javascript_exception(half, getCodeLocation()); \
    }

using namespace ixion;
using namespace javascript;
using namespace std;

// expression -----------------------------------------------------------------
expression::expression(code_location const &loc) : Location(loc)
{
}

expression::~expression()
{
}

string expression::getIndent(int indent) const
{
    string result;

    for (int i = 0; i < indent; ++i)
        result += " ";

    return result;
}

// constant -------------------------------------------------------------------
constant::constant(ref_ptr<value> val, code_location const &loc) : expression(loc), Value(val)
{
}

ref_ptr<value> constant::evaluate(context const &ctx) const
{
    return Value;
}

string constant::toString(int indent) const
{
    return getIndent(indent) + Value->toString();
}

operator_base::operator_base(value::operator_id opt, code_location const &loc) : expression(loc), Operator(opt)
{
}

int operator_base::getPrecedence() const
{
    switch (Operator)
    {
    case value::OP_POST_INCREMENT:
    case value::OP_POST_DECREMENT:
        return 0;
    case value::OP_PRE_INCREMENT:
    case value::OP_PRE_DECREMENT:
    case value::OP_UNARY_PLUS:
    case value::OP_UNARY_MINUS:
    case value::OP_LOG_NOT:
    case value::OP_BIN_NOT:
        return 1;
    case value::OP_MULTIPLY:
    case value::OP_DIVIDE:
    case value::OP_MODULO:
    case value::OP_INVERSE_DIVIDE:
        return 2;
    case value::OP_PLUS:
    case value::OP_MINUS:
        return 3;
    case value::OP_LEFT_SHIFT:
    case value::OP_RIGHT_SHIFT:
        return 4;
    case value::OP_GREATER_EQUAL:
    case value::OP_LESS:
    case value::OP_GREATER:
    case value::OP_LESS_EQUAL:
        return 5;
    case value::OP_EQUAL:
    case value::OP_NOT_EQUAL:
    case value::OP_IDENTICAL:
    case value::OP_NOT_IDENTICAL:
        return 6;
    case value::OP_BIT_AND:
        return 7;
    case value::OP_BIT_XOR:
        return 8;
    case value::OP_BIT_OR:
        return 9;
    case value::OP_LOGICAL_AND:
        return 10;
    case value::OP_LOGICAL_OR:
        return 11;
    // ternary operator missing
    case value::OP_PLUS_ASSIGN:
    case value::OP_MINUS_ASSIGN:
    case value::OP_MUTLIPLY_ASSIGN:
    case value::OP_DIVIDE_ASSIGN:
    case value::OP_MODULO_ASSIGN:
    case value::OP_BIT_AND_ASSIGN:
    case value::OP_BIT_OR_ASSIGN:
    case value::OP_BIT_XOR_ASSIGN:
    case value::OP_LEFT_SHIFT_ASSIGN:
    case value::OP_RIGHT_SHIFT_ASSIGN:
    case value::OP_ASSIGN:
        return 13;
    default:
        return 14;
    }
}

// unary_operator -------------------------------------------------
unary_operator::unary_operator(value::operator_id opt, ref_ptr<expression> opn, code_location const &loc)
    : operator_base(opt, loc), Operand(opn)
{
}

ref_ptr<value> unary_operator::evaluate(context const &ctx) const
{
    try
    {
        return Operand->evaluate(ctx)->operatorUnary(Operator);
    }
    EXJS_ADD_CODE_LOCATION
}

string unary_operator::toString(int indent) const
{
    switch (Operator)
    {
    case value::OP_UNARY_PLUS:
        return getIndent(indent) + "+ " + Operand->toString();
    case value::OP_UNARY_MINUS:
        return getIndent(indent) + "- " + Operand->toString();
    default:
        return getIndent(indent) + value::operator2string(Operator) + " " + Operand->toString();
    }
}

// modifying_unary_operator ---------------------------------------------------
modifying_unary_operator::modifying_unary_operator(value::operator_id opt, ref_ptr<expression> opn,
                                                   code_location const &loc)
    : operator_base(opt, loc), Operand(opn)
{
}

ref_ptr<value> modifying_unary_operator::evaluate(context const &ctx) const
{
    try
    {
        return Operand->evaluate(ctx)->operatorUnaryModifying(Operator);
    }
    EXJS_ADD_CODE_LOCATION
}

string modifying_unary_operator::toString(int indent) const
{
    switch (Operator)
    {
    case value::OP_PRE_INCREMENT:
        return getIndent(indent) + "++" + Operand->toString();
    case value::OP_PRE_DECREMENT:
        return getIndent(indent) + "--" + Operand->toString();
    case value::OP_POST_INCREMENT:
        return getIndent(indent) + Operand->toString() + "++";
    case value::OP_POST_DECREMENT:
        return getIndent(indent) + Operand->toString() + "--";
    default:
        return "";
    }
}

// binary_operator ------------------------------------------------------------
binary_operator::binary_operator(value::operator_id opt, ref_ptr<expression> opn1, ref_ptr<expression> opn2,
                                 code_location const &loc)
    : operator_base(opt, loc), Operand1(opn1), Operand2(opn2)
{
}

ref_ptr<value> binary_operator::evaluate(context const &ctx) const
{
    try
    {
        return Operand1->evaluate(ctx)->operatorBinary(Operator, Operand2->evaluate(ctx));
    }
    EXJS_ADD_CODE_LOCATION
}

string binary_operator::toString(int indent) const
{
    string result = getIndent(indent);
    operator_base *base = dynamic_cast<operator_base *>(Operand1.get());
    if (base && base->getPrecedence() > getPrecedence())
    {
        result += "( " + Operand1->toString() + " )";
    }
    else
    {
        result += Operand1->toString();
    }
    result += " " + value::operator2string(Operator) + " ";
    base = dynamic_cast<operator_base *>(Operand2.get());
    if (base &&
        (base->getPrecedence() > getPrecedence() ||
         ((Operator == value::OP_DIVIDE || Operator == value::OP_MODULO) && base->Operator == value::OP_MULTIPLY)))
    {
        result += "( " + Operand2->toString() + " )";
    }
    else
    {
        result += Operand2->toString();
    }
    return result;
}

// binary_shortcut_operator ---------------------------------------------------
binary_shortcut_operator::binary_shortcut_operator(value::operator_id opt, ref_ptr<expression> opn1,
                                                   ref_ptr<expression> opn2, code_location const &loc)
    : operator_base(opt, loc), Operand1(opn1), Operand2(opn2)
{
}

ref_ptr<value> binary_shortcut_operator::evaluate(context const &ctx) const
{
    try
    {
        return Operand1->evaluate(ctx)->operatorBinaryShortcut(Operator, *Operand2, ctx);
    }
    EXJS_ADD_CODE_LOCATION
}

string binary_shortcut_operator::toString(int indent) const
{
    string result = getIndent(indent);
    operator_base *base = dynamic_cast<operator_base *>(Operand1.get());
    if (base && base->getPrecedence() > getPrecedence())
    {
        result += "( " + Operand1->toString() + " )";
    }
    else
    {
        result += Operand1->toString();
    }
    result += " " + value::operator2string(Operator) + " ";
    base = dynamic_cast<operator_base *>(Operand2.get());
    if (base && base->getPrecedence() > getPrecedence())
    {
        result += "( " + Operand2->toString() + " )";
    }
    else
    {
        result += Operand2->toString();
    }
    return result;
}

// modifying_binary_operator --------------------------------------
modifying_binary_operator::modifying_binary_operator(value::operator_id opt, ref_ptr<expression> opn1,
                                                     ref_ptr<expression> opn2, code_location const &loc)
    : operator_base(opt, loc), Operand1(opn1), Operand2(opn2)
{
}

ref_ptr<value> modifying_binary_operator::evaluate(context const &ctx) const
{
    try
    {
        return ctx.DeclarationScope->operatorBinaryModifying(ctx, Operand1, Operator, Operand2);
    }
    EXJS_ADD_CODE_LOCATION
}

string modifying_binary_operator::toString(int indent) const
{
    string result = getIndent(indent);
    operator_base *base = dynamic_cast<operator_base *>(Operand1.get());
    if (base && base->getPrecedence() > getPrecedence())
    {
        result += "( " + Operand1->toString() + " )";
    }
    else
    {
        result += Operand1->toString();
    }
    result += " " + value::operator2string(Operator) + " ";
    base = dynamic_cast<operator_base *>(Operand2.get());
    if (base && base->getPrecedence() > getPrecedence())
    {
        result += "( " + Operand2->toString() + " )";
    }
    else
    {
        result += Operand2->toString();
    }
    return result;
}

// ternary_operator -----------------------------------------------------------
ternary_operator::ternary_operator(ref_ptr<expression> opn1, ref_ptr<expression> opn2, ref_ptr<expression> opn3,
                                   code_location const &loc)
    : expression(loc), Operand1(opn1), Operand2(opn2), Operand3(opn3)
{
}

ref_ptr<value> ternary_operator::evaluate(context const &ctx) const
{
    try
    {
        if (Operand1->evaluate(ctx)->toBoolean())
            return Operand2->evaluate(ctx);
        else
            return Operand3->evaluate(ctx);
    }
    EXJS_ADD_CODE_LOCATION
}

string ternary_operator::toString(int indent) const
{
    return getIndent(indent) + Operand1->toString() + "? " + Operand2->toString() + " : " + Operand3->toString();
}

// subscript_operation --------------------------------------------------------
subscript_operation::subscript_operation(ref_ptr<expression> opn1, ref_ptr<expression> opn2, code_location const &loc)
    : expression(loc), Operand1(opn1), Operand2(opn2)
{
}

ref_ptr<value> subscript_operation::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> op2 = Operand2->evaluate(ctx);
        return Operand1->evaluate(ctx)->subscript(*op2);
    }
    EXJS_ADD_CODE_LOCATION
}

string subscript_operation::toString(int indent) const
{
    return getIndent(indent) + Operand1->toString() + "[ " + Operand2->toString() + " ]";
}

// lookup_operation -----------------------------------------------------------
lookup_operation::lookup_operation(string const &id, code_location const &loc) : expression(loc), Identifier(id)
{
}

lookup_operation::lookup_operation(ref_ptr<expression> opn, string const &id, code_location const &loc)
    : expression(loc), Operand(opn), Identifier(id)
{
}

bool lookup_operation::IsLookupExpression() const
{
    return true;
}

ref_ptr<value> lookup_operation::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> scope(ctx.LookupScope);
        if (Operand.get() != NULL)
            scope = Operand->evaluate(ctx);
        return scope->lookup(Identifier);
    }
    EXJS_ADD_CODE_LOCATION
}

ref_ptr<expression> lookup_operation::operand() const
{
    return Operand;
}

const string &lookup_operation::identifier() const
{
    return Identifier;
}

string lookup_operation::toString(int indent) const
{
    if (Operand.get() != NULL)
    {
        operator_base *base = dynamic_cast<operator_base *>(Operand.get());
        if (base)
            return getIndent(indent) + "(" + Operand->toString() + ")." + Identifier;
        else
            return getIndent(indent) + Operand->toString() + "." + Identifier;
    }
    else
        return getIndent(indent) + Identifier;
}

// assignment -----------------------------------------------------------------
assignment::assignment(ref_ptr<expression> opn1, ref_ptr<expression> opn2, code_location const &loc)
    : expression(loc), Operand1(opn1), Operand2(opn2)
{
}

ref_ptr<value> assignment::evaluate(context const &ctx) const
{
    try
    {
        return ctx.DeclarationScope->assignment(ctx, Operand1, Operand2);
    }
    EXJS_ADD_CODE_LOCATION
}

string assignment::toString(int indent) const
{
    return getIndent(indent) + Operand1->toString() + " = " + Operand2->toString();
}

// basic_call -----------------------------------------------------------------
basic_call::basic_call(parameter_expression_list const &pexps, code_location const &loc)
    : expression(loc), ParameterExpressionList(pexps)
{
}

void basic_call::makeParameterValueList(context const &ctx, parameter_value_list &pvalues) const
{
    FOREACH_CONST (first, ParameterExpressionList, parameter_expression_list)
    {
        pvalues.push_back((*first)->evaluate(ctx));
    }
}

string basic_call::toString(int indent) const
{
    string result = "(";
    for (size_t i = 0; i < ParameterExpressionList.size(); ++i)
    {
        if (i != 0)
            result += ", ";
        result += ParameterExpressionList[i]->toString();
    }
    result += ")";
    return result;
}

// function_call --------------------------------------------------------------
function_call::function_call(const std::shared_ptr<IValueCreatorService>& valueCreator, ref_ptr<expression> fun, parameter_expression_list const &pexps, code_location const &loc)
    : super(pexps, loc)
    , Function(fun)
    , m_valueCreator(valueCreator)
{
}

void function_call::Throw(no_location_javascript_exception & half) const
{
    lookup_operation *op = dynamic_cast<lookup_operation *>(Function.get());
    std::string error = "In function: ";
    if (op)
    {
        if (op->operand())
            error += op->operand()->toString() + ".";
        error += op->identifier();
    }
    else 
        error += Function->toString();

    if (op)
    {
        std::string arguments = "(";
        bool isFirst = true;
        FOREACH_CONST(first, ParameterExpressionList, parameter_expression_list)
        {
            if (!isFirst)
                arguments += ", ";
            else
                isFirst = false;
            arguments += (*first)->toString();
        }
        arguments += ")";
        error += arguments;
    }
    error += " : ";
    throw javascript_exception(no_location_javascript_exception(half.Error, (error + half.Info).c_str()), getCodeLocation());
}

ref_ptr<value> function_call::evaluate(context const &ctx) const
{
    try
    {
        lookup_operation *op = dynamic_cast<lookup_operation *>(Function.get());
        ctx.DeclarationScope->setLineNumber((int)Function->getCodeLocation().Line);
        ref_ptr<value> func_value = Function->evaluate(ctx);

        value::parameter_list pvalues;
        makeParameterValueList(ctx, pvalues);
        ref_ptr<value> result = func_value->call(pvalues);

        if (result.get() == NULL)
            return m_valueCreator->MakeNull();
        else
            return result;
    }
    catch (no_location_javascript_exception & half)
    {                 
        Throw(half);
    }
    catch (std::exception& e)
    {
        Throw(no_location_javascript_exception(2, e.what()));
    }
    catch (...)
    {
        Throw(no_location_javascript_exception(2, "Unknown exception"));
    }
}

bool function_call::IsLookupExpression() const
{
    lookup_operation *op = dynamic_cast<lookup_operation *>(Function.get());
    if (op && op->operand())
        return op->operand()->IsLookupExpression();
    return false;

}

bool function_call::TrySplitFunctionArguments(std::string& functionName, std::vector<std::string>& args) const
{
    functionName = Function->toString();
    for (const auto& arg : ParameterExpressionList)
    {
        args.push_back(arg->toString());
    }
    return true;
}

string function_call::toString(int indent) const
{
    return getIndent(indent) + Function->toString() + super::toString();
}

// construction ---------------------------------------------------------------
construction::construction(ref_ptr<expression> cls, parameter_expression_list const &pexps, code_location const &loc)
    : super(pexps, loc), Class(cls)
{
}

ref_ptr<value> construction::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> class_value = Class->evaluate(ctx);

        value::parameter_list pvalues;
        makeParameterValueList(ctx, pvalues);

        return class_value->construct(pvalues);
    }
    EXJS_ADD_CODE_LOCATION
}

string construction::toString(int indent) const
{
    return getIndent(indent) + Class->toString() + super::toString();
}
