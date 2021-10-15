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

using namespace std;
using namespace ixion;
using namespace javascript;

// variable_declaration -------------------------------------------------------
variable_declaration::variable_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, string const &id, ref_ptr<expression> def_value, code_location const &loc)
    : expression(loc)
    , Identifier(id)
    , DefaultValue(def_value)
    , m_valueCreator(valueCreator)
{
}

ref_ptr<value> variable_declaration::evaluate(context const &ctx) const
{
    try
    {
        return ctx.DeclarationScope->defineMember(ctx, Identifier, DefaultValue, m_valueCreator);
    }
    EXJS_ADD_CODE_LOCATION
}

string variable_declaration::toString(int indent) const
{
    if (DefaultValue.get())
        return getIndent(indent) + "var " + Identifier + " = " + DefaultValue->toString();
    else
        return getIndent(indent) + "var " + Identifier;
}

// constant_declaration -------------------------------------------------------
constant_declaration::constant_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, string const &id, ref_ptr<expression> def_value, code_location const &loc)
    : expression(loc)
    , Identifier(id)
    , DefaultValue(def_value)
    , m_valueCreator(valueCreator)
{
}

ref_ptr<value> constant_declaration::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> def;
        if (DefaultValue.get() != NULL)
            def = DefaultValue->evaluate(ctx)->eliminateWrappers()->duplicate();
        else
            def = m_valueCreator->MakeNull();

        ref_ptr<value> cns = m_valueCreator->WrapConstant(def);
        ctx.DeclarationScope->addMember(Identifier, cns);

        return cns;
    }
    EXJS_ADD_CODE_LOCATION
}

string constant_declaration::toString(int indent) const
{
    if (DefaultValue.get())
        return getIndent(indent) + Identifier + " = " + DefaultValue->toString();
    else
        return getIndent(indent) + Identifier;
}

// function_declaration -------------------------------------------------------
function_declaration::function_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, string const &id, parameter_name_list const &pnames,
    ref_ptr<expression> body, parameter_name_list const &ptypes, const std::string& rtype, code_location const &loc)
    : expression(loc)
    , Identifier(id)
    , ParameterNameList(pnames)
    , Body(body)
    , ParameterTypeList(ptypes)
    , ReturnType(rtype)
    , ValueCreator(valueCreator)
{
}

ref_ptr<value> function_declaration::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> fun = ValueCreator->MakeFunction(Identifier, ParameterNameList, ParameterTypeList, ReturnType, Body, ctx);
        if (Identifier.size() != 0)
        {
            ctx.DeclarationScope->addMember(Identifier, fun);
            return ref_ptr<value>(NULL);
        }
        else
            return fun;
    }
    EXJS_ADD_CODE_LOCATION
}

string function_declaration::toString(int indent) const
{
    string result = getIndent(indent) + Identifier + " = function(";

    for (size_t i = 0; i < ParameterNameList.size(); ++i)
    {
        if (i != 0)
            result += ", ";

        result += ParameterNameList[i];
    }

    result += ")\r\n";
    result += getIndent(indent) + "{\r\n";
    result += Body->toString(indent + 3);
    result += getIndent(indent) + "}\r\n";

    return result;
}

// method_declaration ---------------------------------------------------------
method_declaration::method_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, string const &id, parameter_name_list const &pnames, ref_ptr<expression> body,
                                       code_location const &loc)
    : expression(loc), Identifier(id), ParameterNameList(pnames), Body(body), ValueCreator(valueCreator)
{
}

ref_ptr<value> method_declaration::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> fun = new method(ValueCreator, ParameterNameList, Body, ctx.LookupScope, ctx);
        ctx.DeclarationScope->addMember(Identifier, fun);

        return ref_ptr<value>(NULL);
    }
    EXJS_ADD_CODE_LOCATION
}

string method_declaration::toString(int indent) const
{
    string result = getIndent(indent) + Identifier + " = function(";

    for (size_t i = 0; i < ParameterNameList.size(); ++i)
    {
        if (i != 0)
            result += ", ";

        result += ParameterNameList[i];
    }

    result += ")\r\n";
    result += getIndent(indent) + "{\r\n";
    result += Body->toString(indent + 3);
    result += getIndent(indent) + "}\r\n";
    return result;
}

// constructor_declaration ---------------------------------------------------------
constructor_declaration::constructor_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body,
                                                 code_location const &loc)
    : expression(loc), ParameterNameList(pnames), Body(body)
    , ValueCreator(valueCreator)
{
}

ref_ptr<value> constructor_declaration::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<value> fun = new constructor(ValueCreator, ParameterNameList, Body, ctx.LookupScope, ctx);
        return fun;
    }
    EXJS_ADD_CODE_LOCATION
}

string constructor_declaration::toString(int indent) const
{
    string result = getIndent(indent) + "function(";

    for (size_t i = 0; i < ParameterNameList.size(); ++i)
    {
        if (i != 0)
            result += ", ";

        result += ParameterNameList[i];
    }

    result += ")\r\n";
    result += getIndent(indent) + "{\r\n";
    result += Body->toString(indent + 3);
    result += getIndent(indent) + "}\r\n";

    return result;
}

// js_class_declaration -------------------------------------------------------
js_class_declaration::js_class_declaration(string const &id, ref_ptr<expression> superclass, code_location const &loc)
    : expression(loc), Identifier(id), SuperClass(superclass)
{
}

ref_ptr<value> js_class_declaration::evaluate(context const &ctx) const
{
    try
    {
        ref_ptr<list_scope, value> sml(ctx.DeclarationScope->construct());
        ref_ptr<list_scope, value> ml(ctx.DeclarationScope->construct());
        ref_ptr<list_scope, value> svl(ctx.DeclarationScope->construct());

        ref_ptr<value> sc;
        if (SuperClass.get())
            sc = SuperClass->evaluate(ctx);

        ref_ptr<value> constructor;
        if (ConstructorDeclaration.get())
            constructor = ConstructorDeclaration->evaluate(ctx);

        ref_ptr<value> cls(
            new js_class(ctx.LookupScope, sc, constructor, sml.get(), ml.get(), svl.get(), VariableList, ctx));

        ref_ptr<list_scope, value> static_scope(ctx.DeclarationScope->construct());
        static_scope->unite(ctx.LookupScope);
        static_scope->unite(cls);

        {
            FOREACH_CONST (first, StaticMethodList, declaration_list)
                (*first)->evaluate(context(sml, static_scope.get()));
        }
        {
            FOREACH_CONST (first, MethodList, declaration_list)
                (*first)->evaluate(context(ml, ctx.LookupScope));
        }
        {
            FOREACH_CONST (first, StaticVariableList, declaration_list)
                (*first)->evaluate(context(svl, static_scope.get()));
        }

        ctx.DeclarationScope->addMember(Identifier, cls);
        return cls;
    }
    EXJS_ADD_CODE_LOCATION
}

void js_class_declaration::setConstructor(ref_ptr<expression> decl)
{
    ConstructorDeclaration = decl;
}

void js_class_declaration::addStaticMethod(ref_ptr<expression> decl)
{
    StaticMethodList.push_back(decl);
}

void js_class_declaration::addMethod(ref_ptr<expression> decl)
{
    MethodList.push_back(decl);
}

void js_class_declaration::addStaticVariable(ref_ptr<expression> decl)
{
    StaticVariableList.push_back(decl);
}

void js_class_declaration::addVariable(ref_ptr<expression> decl)
{
    VariableList.push_back(decl);
}

string js_class_declaration::toString(int indent) const
{
    return "";
}
