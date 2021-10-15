// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#ifndef IXLIB_JS_INTERNALS
#define IXLIB_JS_INTERNALS

#include <ixlib_javascript.hh>
#include "TypeUtilities\IntrusiveClass.h"
#include <functional>

namespace ixion { namespace javascript {

struct code_location
{
    TIndex Line;

    code_location(const scanner::token &tok);
    explicit code_location(TIndex line);
    std::string stringify() const;
};

struct return_exception
{
    ref_ptr<value> ReturnValue;
    code_location Location;

    return_exception(ref_ptr<value> retval, code_location const &loc) : ReturnValue(retval), Location(loc)
    {
    }
};

struct break_exception
{
    bool HasLabel;
    std::string Label;
    code_location Location;

    break_exception(bool has_label, std::string const &label, code_location const &loc)
        : HasLabel(has_label), Label(label), Location(loc)
    {
    }
};

struct continue_exception
{
    bool HasLabel;
    std::string Label;
    code_location Location;

    continue_exception(bool has_label, std::string const &label, code_location const &loc)
        : HasLabel(has_label), Label(label), Location(loc)
    {
    }
};

// values -----------------------------------------------------------------
class null : public value
{
private:
    typedef value super;

public:
    value_type getType() const;
    bool toBoolean() const;
    double toFloat(bool allow_throw = false) const;
    int toInt() const;
    std::string toString() const;

    ref_ptr<value> duplicate();
};

class const_floating_point : public value_with_methods
{
private:
    typedef value_with_methods super;

protected:
    double Value;

public:
    const_floating_point(double value);
    bool operator==(const const_floating_point& other) const { return toFloat() == other.toFloat(); }
    bool operator!=(const const_floating_point& other) const { return toFloat() != other.toFloat(); }
    bool operator<(const const_floating_point& other) const { return toFloat() < other.toFloat(); }
    value_type getType() const;
    int toInt() const;
    double toFloat(bool allow_throw = false) const;
    bool toBoolean() const;
    std::string toString() const;

    ref_ptr<value> duplicate();

    ref_ptr<value> callMethod(std::string const &identifier, parameter_list const &parameters);

    ref_ptr<value> operatorUnary(operator_id op) const;
    ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
};

class floating_point : public const_floating_point
{
private:
    typedef const_floating_point super;

public:
    floating_point(double value);

    double &GetDoubleRef()
    {
        return Value;
    }

    ref_ptr<value> operatorUnaryModifying(operator_id op);
    ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
};

class const_integer : public value_with_methods
{
private:
    typedef value_with_methods super;

protected:
    long Value;

public:
    const_integer(long value);
    bool operator==(const const_integer& other) const { return toInt() == other.toInt(); }
    bool operator!=(const const_integer& other) const { return toInt() != other.toInt(); }
    bool operator<(const const_integer& other) const { return toInt() < other.toInt(); }

    value_type getType() const;
    int toInt() const;
    double toFloat(bool allow_throw = false) const;
    bool toBoolean() const;
    std::string toString() const;

    ref_ptr<value> duplicate();

    ref_ptr<value> callMethod(std::string const &identifier, parameter_list const &parameters);

    ref_ptr<value> operatorUnary(operator_id op) const;
    ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
};

class integer : public const_integer
{
private:
    typedef const_integer super;

public:
    integer(long value);

    ref_ptr<value> operatorUnaryModifying(operator_id op);
    ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
};

class js_string : public value_with_methods
{
private:
    typedef value_with_methods super;

protected:
    std::string Value;

public:
    js_string(std::string const &value);
    bool operator==(const js_string& other) const { return toString() == other.toString(); }
    bool operator!=(const js_string& other) const { return toString() != other.toString(); }
    bool operator<(const js_string& other) const { return toString() < other.toString(); }
    value_type getType() const;
    std::string toString() const;
    bool toBoolean() const;
    double toFloat(bool allow_throw = false) const;
    int toInt() const;
    std::string stringify() const;

    ref_ptr<value> duplicate();

    ref_ptr<value> lookup(std::string const &identifier);
    ref_ptr<value> callMethod(std::string const &identifier, parameter_list const &parameters);

    ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
    ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
};

class lvalue : public value
{
protected:
    ref_ptr<value> Reference;

public:
    lvalue(ref_ptr<value> ref_ptr);

    value_type getType() const;
    std::string toString() const;
    int toInt() const;
    double toFloat(bool allow_throw = false) const;
    bool toBoolean() const;
    std::string stringify() const;

    ref_ptr<value> eliminateWrappers();
    ref_ptr<value> duplicate();

    ref_ptr<value> lookup(std::string const &identifier);
    ref_ptr<value> subscript(value const &index);
    ref_ptr<value> call(parameter_list const &parameters);
    ref_ptr<value> callAsMethod(ref_ptr<value> instance, parameter_list const &parameters);
    ref_ptr<value> construct(parameter_list const &parameters);
    ref_ptr<value> assign(ref_ptr<value> op2);
    virtual ixlib_iterator begin() const;
    virtual ixlib_iterator end() const;
    virtual TSize size() const;  // changed from size_t to Tsize for 64 bits compilling error.
    // for super class uses Tsize, but sub-class used size_t.
    // they are not the same type on 64 platform.
    // reason for using Tsize instead of size_t:  a array will not be longer then 4G.
    // jizhi@dnvs.

    ref_ptr<value> operatorUnary(operator_id op) const;
    ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
    ref_ptr<value> operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const;
    ref_ptr<value> operatorUnaryModifying(operator_id op);
    ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
};

class constant_wrapper : public value
{
protected:
    ref_ptr<value> Constant;

public:
    constant_wrapper(ref_ptr<value> val);

    value_type getType() const;
    std::string toString() const;
    int toInt() const;
    double toFloat(bool allow_throw = false) const;
    bool toBoolean() const;
    std::string stringify() const;

    ref_ptr<value> eliminateWrappers();
    ref_ptr<value> duplicate();

    ref_ptr<value> lookup(std::string const &identifier);
    ref_ptr<value> subscript(value const &index);
    ref_ptr<value> call(parameter_list const &parameters) const;
    ref_ptr<value> callAsMethod(ref_ptr<value> instance, parameter_list const &parameters);
    ref_ptr<value> construct(parameter_list const &parameters);
    ref_ptr<value> assign(ref_ptr<value> value);

    ref_ptr<value> operatorUnary(operator_id op) const;
    ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
    ref_ptr<value> operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const;
    ref_ptr<value> operatorUnaryModifying(operator_id op);
    ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
};

class callable_with_parameters : public value
{
public:
    typedef std::vector<std::string> parameter_name_list;

protected:
    parameter_name_list ParameterNameList;
    std::shared_ptr<IValueCreatorService> ValueCreator;
public:
    callable_with_parameters(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames);

    size_t param_count() const
    {
        return ParameterNameList.size();
    }
	const parameter_name_list& GetParameterNames() const { return ParameterNameList; }

    void addParametersTo(list_scope &scope, parameter_list const &parameters) const;
    static ref_ptr<value> evaluateBody(expression &body, context const &ctx);
};

class function : public callable_with_parameters
{
    typedef callable_with_parameters super;
    ref_ptr<expression> Body;
    ref_ptr<value> LexicalScope;
    context Context;

public:
    function(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, context const &ctx);

    value_type getType() const
    {
        return VT_FUNCTION;
    }

    ref_ptr<value> duplicate();
    std::string toString() const;
    std::string stringify() const;

    ref_ptr<value> call(parameter_list const &parameters);
};

class method : public callable_with_parameters
{
    typedef callable_with_parameters super;
    ref_ptr<expression> Body;
    ref_ptr<value> LexicalScope;
    context Context;

public:
    method(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, ref_ptr<value> lex_scope, context const &ctx);

    value_type getType() const
    {
        return VT_FUNCTION;
    }

    ref_ptr<value> duplicate();

    ref_ptr<value> callAsMethod(ref_ptr<value> instance, parameter_list const &parameters);
};

class constructor : public callable_with_parameters
{
    typedef callable_with_parameters super;
    ref_ptr<expression> Body;
    ref_ptr<value> LexicalScope;
    context Context;

public:
    constructor(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, ref_ptr<value> lex_scope,
                context const &ctx);

    value_type getType() const
    {
        return VT_FUNCTION;
    }

    ref_ptr<value> duplicate();
    ref_ptr<value> callAsMethod(ref_ptr<value> instance, parameter_list const &parameters);
};

class js_class : public value
{
    class super_instance_during_construction : public value
    {
        // this object constructs the superclass
        // a) if it is called, by calling the super constructor
        //    with the aprropriate parameters
        // b) implicitly with no super constructor arguments,
        //    if the super object is referenced explicitly

        ref_ptr<value> SuperClass;
        ref_ptr<value> SuperClassInstance;

    public:
        super_instance_during_construction(ref_ptr<value> super_class);

        value_type getType() const
        {
            return VT_OBJECT;
        }

        ref_ptr<value> call(parameter_list const &parameters);
        ref_ptr<value> lookup(std::string const &identifier);

        ref_ptr<value> getSuperClassInstance();
    };

    typedef std::vector<ref_ptr<expression> > declaration_list;

    ref_ptr<value> LexicalScope;
    ref_ptr<value> SuperClass;
    ref_ptr<value> Constructor;
    ref_ptr<value> StaticMethodScope;
    ref_ptr<value> MethodScope;
    ref_ptr<value> StaticVariableScope;
    declaration_list VariableList;
    context Context;

public:
    js_class(ref_ptr<value> lex_scope, ref_ptr<value> super_class, ref_ptr<value> constructor,
             ref_ptr<value> static_method_scope, ref_ptr<value> method_scope, ref_ptr<value> static_variable_scope,
             declaration_list const &variable_list, context const &ctx);

    value_type getType() const
    {
        return VT_TYPE;
    }

    ref_ptr<value> duplicate();
    ref_ptr<value> lookup(std::string const &identifier);
    ref_ptr<value> lookupLocal(std::string const &identifier);
    ref_ptr<value> construct(parameter_list const &parameters);
};

class js_class_instance : public value
{
    class bound_method : public value
    {
        ref_ptr<value> Instance;
        ref_ptr<value> Method;

    public:
        bound_method(ref_ptr<value> instance, ref_ptr<value> method);

        value_type getType() const
        {
            return VT_BOUND_METHOD;
        }

        ref_ptr<value> call(parameter_list const &parameters);
    };

    ref_ptr<value> SuperClassInstance;
    ref_ptr<js_class, value> Class;
    ref_ptr<value> MethodScope;
    ref_ptr<value> VariableScope;

public:
    js_class_instance(ref_ptr<js_class, value> cls, ref_ptr<value> method_scope, ref_ptr<value> variable_scope);

    void setSuperClassInstance(ref_ptr<value> super_class_instance);

    value_type getType() const
    {
        return VT_OBJECT;
    }

    ref_ptr<value> duplicate();
    ref_ptr<value> lookup(std::string const &identifier);
};

class js_array_constructor : public value
{
public:
    value_type getType() const
    {
        return VT_TYPE;
    }

    ref_ptr<value> duplicate();
    ref_ptr<value> construct(parameter_list const &parameters);
};

// expressions ----------------------------------------------------------
class expression : public DNVS::MoFa::TypeUtilities::IntrusiveClass<expression>
{
    code_location Location;

public:
    expression(code_location const &loc);
    virtual ~expression();
    virtual ref_ptr<value> evaluate(context const &ctx) const = 0;
    virtual std::string toString(int indent = 0) const = 0;
    std::string getIndent(int indent) const;
    virtual bool IsLookupExpression() const { return false; }
    virtual bool TrySplitFunctionArguments(std::string& functionName, std::vector<std::string>& args) const { return false; }

    code_location const &getCodeLocation() const
    {
        return Location;
    }
};

class constant : public expression
{
    ref_ptr<value> Value;

public:
    constant(ref_ptr<value> val, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class operator_base : public expression
{
public:
    operator_base(value::operator_id opt, code_location const &loc);
    int getPrecedence() const;
    value::operator_id Operator;
};

class unary_operator : public operator_base
{
    ref_ptr<expression> Operand;

public:
    unary_operator(value::operator_id opt, ref_ptr<expression> opn, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class modifying_unary_operator : public operator_base
{
    ref_ptr<expression> Operand;

public:
    modifying_unary_operator(value::operator_id opt, ref_ptr<expression> opn, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class binary_operator : public operator_base
{
protected:
    ref_ptr<expression> Operand1;
    ref_ptr<expression> Operand2;

public:
    binary_operator(value::operator_id opt, ref_ptr<expression> opn1, ref_ptr<expression> opn2,
                    code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class binary_shortcut_operator : public operator_base
{
    ref_ptr<expression> Operand1;
    ref_ptr<expression> Operand2;

public:
    binary_shortcut_operator(value::operator_id opt, ref_ptr<expression> opn1, ref_ptr<expression> opn2,
                             code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class modifying_binary_operator : public operator_base
{
    ref_ptr<expression> Operand1;
    ref_ptr<expression> Operand2;

public:
    modifying_binary_operator(value::operator_id opt, ref_ptr<expression> opn1, ref_ptr<expression> opn2,
                              code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class ternary_operator : public expression
{
    ref_ptr<expression> Operand1;
    ref_ptr<expression> Operand2;
    ref_ptr<expression> Operand3;

public:
    ternary_operator(ref_ptr<expression> opn1, ref_ptr<expression> opn2, ref_ptr<expression> opn3,
                     code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class subscript_operation : public expression
{
    ref_ptr<expression> Operand1;
    ref_ptr<expression> Operand2;

public:
    subscript_operation(ref_ptr<expression> opn1, ref_ptr<expression> opn2, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class lookup_operation : public expression
{
    ref_ptr<expression> Operand;
    std::string Identifier;

public:
    lookup_operation(std::string const &id, code_location const &loc);
    lookup_operation(ref_ptr<expression> opn, std::string const &id, code_location const &loc);
    virtual bool IsLookupExpression() const;
    ref_ptr<value> evaluate(context const &ctx) const;
    // Implemented in order to make assignment to variables not defined using a var declaration possible
    ref_ptr<expression> operand() const;
    const std::string &identifier() const;
    virtual std::string toString(int indent = 0) const;
};

class assignment : public expression
{
    ref_ptr<expression> Operand1;
    ref_ptr<expression> Operand2;

public:
    assignment(ref_ptr<expression> opn1, ref_ptr<expression> opn2, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class basic_call : public expression
{
public:
    typedef std::vector<ref_ptr<expression> > parameter_expression_list;
    typedef std::vector<ref_ptr<value> > parameter_value_list;

protected:
    parameter_expression_list ParameterExpressionList;

public:
    basic_call(parameter_expression_list const &pexps, code_location const &loc);
    void makeParameterValueList(context const &ctx, parameter_value_list &pvalues) const;
    virtual std::string toString(int indent = 0) const;
};

class function_call : public basic_call
{
    typedef basic_call super;
    ref_ptr<expression> Function;
    std::shared_ptr<IValueCreatorService> m_valueCreator;
public:
    virtual bool IsLookupExpression() const;
    virtual bool TrySplitFunctionArguments(std::string& functionName, std::vector<std::string>& args) const;
    function_call(const std::shared_ptr<IValueCreatorService>& valueCreator, ref_ptr<expression> fun, parameter_expression_list const &pexps, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
    void Throw(no_location_javascript_exception & half) const;
};

class construction : public basic_call
{
    typedef basic_call super;
    ref_ptr<expression> Class;

public:
    construction(ref_ptr<expression> cls, parameter_expression_list const &pexps, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

// declarations -----------------------------------------------------------
class variable_declaration : public expression
{
protected:
    std::string Identifier;
    ref_ptr<expression> DefaultValue;
    std::shared_ptr<IValueCreatorService> m_valueCreator;
public:
    variable_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, std::string const &id, ref_ptr<expression> def_value, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class constant_declaration : public expression
{
protected:
    std::string Identifier;
    ref_ptr<expression> DefaultValue;
    std::shared_ptr<IValueCreatorService> m_valueCreator;
public:
    constant_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, std::string const &id, ref_ptr<expression> def_value, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    virtual std::string toString(int indent = 0) const;
};

class function_declaration : public expression
{
public:
    typedef function::parameter_name_list parameter_name_list;

private:
    std::string Identifier;
    parameter_name_list ParameterNameList;
    parameter_name_list ParameterTypeList;
    std::string ReturnType;
    ref_ptr<expression> Body;
    std::shared_ptr<IValueCreatorService> ValueCreator;
public:
    function_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, std::string const &id, parameter_name_list const &pnames, ref_ptr<expression> body,
        parameter_name_list const &ptypes, const std::string& rtype, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class method_declaration : public expression
{
public:
    typedef method::parameter_name_list parameter_name_list;

private:
    std::string Identifier;
    parameter_name_list ParameterNameList;
    ref_ptr<expression> Body;
    std::shared_ptr<IValueCreatorService> ValueCreator;
public:
    method_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, std::string const &id, parameter_name_list const &pnames, ref_ptr<expression> body,
                       code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class constructor_declaration : public expression
{
public:
    typedef method::parameter_name_list parameter_name_list;

private:
    parameter_name_list ParameterNameList;
    ref_ptr<expression> Body;
    std::shared_ptr<IValueCreatorService> ValueCreator;
public:
    constructor_declaration(const std::shared_ptr<IValueCreatorService>& valueCreator, parameter_name_list const &pnames, ref_ptr<expression> body, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_class_declaration : public expression
{
    typedef std::vector<ref_ptr<expression> > declaration_list;

    std::string Identifier;
    ref_ptr<expression> SuperClass;
    ref_ptr<expression> ConstructorDeclaration;
    declaration_list StaticMethodList;
    declaration_list MethodList;
    declaration_list StaticVariableList;
    declaration_list VariableList;

public:
    js_class_declaration(std::string const &id, ref_ptr<expression> superclass, code_location const &loc);

    ref_ptr<value> evaluate(context const &ctx) const;

    void setConstructor(ref_ptr<expression> decl);
    void addStaticMethod(ref_ptr<expression> decl);
    void addMethod(ref_ptr<expression> decl);
    void addStaticVariable(ref_ptr<expression> decl);
    void addVariable(ref_ptr<expression> decl);
    std::string toString(int indent = 0) const;
};

// instructions ---------------------------------------------------------
class instruction_list : public expression
{
    typedef std::vector<ref_ptr<expression> > expression_list;
    expression_list ExpressionList;

public:
    instruction_list(code_location const &loc) : expression(loc)
    {
    }
    virtual bool IsLookupExpression() const;
    virtual bool TrySplitFunctionArguments(std::string& functionName, std::vector<std::string>& args) const;

    ref_ptr<value> evaluate(context const &ctx) const;
    void add(ref_ptr<expression> expr);
    std::string toString(int indent = 0) const;
};

class scoped_instruction_list : public instruction_list
{
public:
    scoped_instruction_list(code_location const &loc) : instruction_list(loc)
    {
    }

    ref_ptr<value> evaluate(context const &ctx) const;
};

class js_if : public expression
{
    ref_ptr<expression> Conditional;
    ref_ptr<expression> IfExpression;
    ref_ptr<expression> IfNotExpression;
    std::shared_ptr<IValueCreatorService> valueCreator;
public:
    js_if(const std::shared_ptr<IValueCreatorService>& valueCreator, ref_ptr<expression> cond, ref_ptr<expression> ifex, ref_ptr<expression> ifnotex, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
    std::string toString(int indent, bool elseif) const;
};

class js_while : public expression
{
    ref_ptr<expression> Conditional;
    ref_ptr<expression> LoopExpression;
    bool HasLabel;
    std::string Label;

public:
    js_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, code_location const &loc);
    js_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, std::string const &Label, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_do_while : public expression
{
    ref_ptr<expression> Conditional;
    ref_ptr<expression> LoopExpression;
    bool HasLabel;
    std::string Label;

public:
    js_do_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, code_location const &loc);
    js_do_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, std::string const &Label, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_for : public expression
{
    ref_ptr<expression> Initialization;
    ref_ptr<expression> Conditional;
    ref_ptr<expression> Update;
    ref_ptr<expression> LoopExpression;
    bool HasLabel;
    std::string Label;

public:
    js_for(ref_ptr<expression> init, ref_ptr<expression> cond, ref_ptr<expression> update, ref_ptr<expression> loop,
           code_location const &loc);
    js_for(ref_ptr<expression> init, ref_ptr<expression> cond, ref_ptr<expression> update, ref_ptr<expression> loop,
        std::string const &label, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_for_in : public expression
{
    ref_ptr<expression> Iterator;
    ref_ptr<expression> Array;
    ref_ptr<expression> LoopExpression;
    bool HasLabel;
    std::string Label;

public:
    js_for_in(ref_ptr<expression> iter, ref_ptr<expression> array, ref_ptr<expression> loop, code_location const &loc);
    js_for_in(ref_ptr<expression> iter, ref_ptr<expression> array, ref_ptr<expression> loop, std::string const &label,
              code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_return : public expression
{
    ref_ptr<expression> ReturnValue;

public:
    js_return(ref_ptr<expression> retval, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_break : public expression
{
    bool HasLabel;
    std::string Label;

public:
    js_break(code_location const &loc);
    js_break(std::string const &label, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_continue : public expression
{
    bool HasLabel;
    std::string Label;

public:
    js_continue(code_location const &loc);
    js_continue(std::string const &label, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class break_label : public expression
{
    std::string Label;
    ref_ptr<expression> Expression;

public:
    break_label(std::string const &label, ref_ptr<expression> expr, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    std::string toString(int indent = 0) const;
};

class js_switch : public expression
{
    bool HasLabel;
    std::string Label;
    ref_ptr<expression> Discriminant;

    struct case_label
    {
        ref_ptr<expression> DiscriminantValue;
        ref_ptr<expression> Expression;
    };

    typedef std::vector<case_label> case_list;
    case_list CaseList;

public:
    js_switch(ref_ptr<expression> discriminant, code_location const &loc);
    js_switch(ref_ptr<expression> discriminant, std::string const &label, code_location const &loc);
    ref_ptr<value> evaluate(context const &ctx) const;
    void addCase(ref_ptr<expression> dvalue, ref_ptr<expression> expr);
    std::string toString(int indent = 0) const;
};

// Defined in order to make parsing of units possible
typedef std::function<ref_ptr<expression>(ref_ptr<expression>, scanner::token_iterator &,
                                          scanner::token_iterator const &, int)> UnitParserFunction;
void setUnitParser(UnitParserFunction unitParser);

}}

#endif
