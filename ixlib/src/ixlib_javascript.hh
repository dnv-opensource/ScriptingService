// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#ifndef IXLIB_JAVASCRIPT
#define IXLIB_JAVASCRIPT

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <vector>
#ifdef _MSC_VER
#include <map>
#include <algorithm>
#elif __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>
#endif
#include <ixlib_string.hh>
#include <ixlib_exbase.hh>
#include <ixlib_garbage.hh>
#include <ixlib_scanner.hh>
#include "TypeUtilities/IntrusiveClass.h"
#include "../IValueCreatorService.h"

// Error codes ----------------------------------------------------------------
#define ECJS_UNTERMINATED_COMMENT 0
#define ECJS_CANNOT_CONVERT 1
#define ECJS_INVALID_OPERATION 2
#define ECJS_UNEXPECTED 3
#define ECJS_UNEXPECTED_EOF 4
#define ECJS_CANNOT_MODIFY_RVALUE 5
#define ECJS_UNKNOWN_IDENTIFIER 6
#define ECJS_UNKNOWN_OPERATOR 7
#define ECJS_INVALID_NON_LOCAL_EXIT 8
#define ECJS_INVALID_NUMBER_OF_ARGUMENTS 9
#define ECJS_INVALID_TOKEN 10
#define ECJS_CANNOT_REDECLARE 11
#define ECJS_DOUBLE_CONSTRUCTION 12
#define ECJS_NO_SUPERCLASS 13
#define ECJS_DIVISION_BY_ZERO 14

// helpful macros -------------------------------------------------------------
#define IXLIB_JS_ASSERT_PARAMETERS(NAME, ARGMIN, ARGMAX)          \
    if (parameters.size() < ARGMIN || parameters.size() > ARGMAX) \
    EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, NAME)

#define IXLIB_JS_IF_METHOD(NAME, ARGMIN, ARGMAX)                      \
    if (identifier == NAME)                                           \
        if (parameters.size() < ARGMIN || parameters.size() > ARGMAX) \
            EXJS_THROWINFO(ECJS_INVALID_NUMBER_OF_ARGUMENTS, NAME)    \
        else

#define IXLIB_JS_DECLARE_FUNCTION(NAME)                                                  \
    namespace                                                                            \
    {                                                                                    \
    class NAME : public value                                                            \
    {                                                                                    \
    public:                                                                              \
        value_type getType() const                                                       \
        {                                                                                \
            return VT_FUNCTION;                                                          \
        }                                                                                \
        ixion::ref_ptr<ixion::javascript::value> call(parameter_list const &parameters); \
    };                                                                                   \
    }                                                                                    \
    ixion::ref_ptr<ixion::javascript::value> NAME::call(parameter_list const &parameters)

#define IXLIB_JS_CONVERT_PARAMETERS_0

// Exception throw macros -----------------------------------------------------
#define EXJS_THROW(CODE) EX_THROW(javascript, CODE)
#define EXJS_THROWINFO(CODE, INFO) EX_THROWINFO(javascript, CODE, INFO)
#define EXJS_THROW_NO_LOCATION(CODE) EX_THROW(no_location_javascript, CODE)
#define EXJS_THROWINFO_NO_LOCATION(CODE, INFO) EX_THROWINFO(no_location_javascript, CODE, INFO)
#define EXJS_THROWINFOLOCATION(CODE, INFO, LOCATION) \
    throw ixion::javascript_exception(CODE, LOCATION, INFO, __FILE__, __LINE__);
#define EXJS_THROWINFOTOKEN(CODE, INFO, TOKEN) EXJS_THROWINFOLOCATION(CODE, INFO, code_location(TOKEN))
#define EXJS_THROWINFOEXPRESSION(CODE, INFO, EXPR) EXJS_THROWINFOLOCATION(CODE, INFO, (EXPR).getCodeLocation())

namespace ixion { namespace javascript {

struct code_location;

}

// exceptions ---------------------------------------------------------------
struct no_location_javascript_exception : public base_exception
{
    no_location_javascript_exception(TErrorCode error, char const *info = NULL, char *module = NULL, TIndex line = 0)
        : base_exception(error, info, module, line, "JS")
    {
    }

    virtual const char *getText() const;
};

struct javascript_exception : public base_exception
{
    javascript_exception(TErrorCode error, char const *info = NULL, char *module = NULL, TIndex line = 0)
        : base_exception(error, info, module, line, "JS")
    {
    }

    javascript_exception(TErrorCode error, javascript::code_location const &loc, char const *info = 0,
                         char *module = NULL, TIndex line = 0);
    javascript_exception(no_location_javascript_exception const &half_ex, javascript::code_location const &loc);
    virtual const char *getText() const;
};

// javascript ---------------------------------------------------------------
/**
This code tries to be an implementation of ECMAScript 4, as available at
http://www.mozilla.org/js/language/
Note that ES4 is still in the process of standardization.

It is meant to behave like an ES4 interpreter in strict mode, none
of the backward-compatible braindead-isms like newline semicolon
insertion and other stuff will ever be implemented.

This is the list of its shortcomings:
<ul>
  <li> exceptions
  <li> namespaces,packages
  <li> constness
  <li> Number/String constructor and class methods
  <li> real regexp's
  <li> the methods listed in FIXME's (js_library.cc js_value.cc)
  <li> cannot cross-assign predefined methods [won't be]
  <li> Grammatical semicolon insertion [won't be]
  <li> type declaration [won't be]
  </ul>

Be advised that a javascript value that is passed to you through the
interpreter, e.g. as a call parameter, may not be of the type that
you expect. For example, in "var x = 4; f(x);", what comes in as
the parameter x into f is a wrapper value that adds assign()ability
to a value that is wrapped inside. The advised solution to get the
object that you expect is to call eliminateWrappers() on the potentially
wrapped value.
*/
namespace javascript {

class value;
class list_scope;

struct context
{
    ref_ptr<list_scope, value> DeclarationScope;
    ref_ptr<value> LookupScope;

    context(ref_ptr<list_scope, value> scope);
    context(ref_ptr<value> scope);
    context(ref_ptr<list_scope, value> decl_scope, ref_ptr<value> lookup_scope);
};

class expression;

struct ixlib_iterator;

class value : public DNVS::MoFa::TypeUtilities::IntrusiveClass<value>
{
public:
    enum operator_id
    {
        // unary, modifying
        OP_PRE_INCREMENT,
        OP_POST_INCREMENT,
        OP_PRE_DECREMENT,
        OP_POST_DECREMENT,
        // unary, non-modifying
        OP_UNARY_PLUS,
        OP_UNARY_MINUS,
        OP_LOG_NOT,
        OP_BIN_NOT,
        // binary, modifying
        OP_PLUS_ASSIGN,
        OP_MINUS_ASSIGN,
        OP_MUTLIPLY_ASSIGN,
        OP_DIVIDE_ASSIGN,
        OP_MODULO_ASSIGN,
        OP_BIT_AND_ASSIGN,
        OP_BIT_OR_ASSIGN,
        OP_BIT_XOR_ASSIGN,
        OP_LEFT_SHIFT_ASSIGN,
        OP_RIGHT_SHIFT_ASSIGN,
        // binary, non-modifying
        OP_PLUS,
        OP_MINUS,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_MODULO,
        OP_INVERSE_DIVIDE,  // inverses the operand for the divide operation. op2/op1
        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_XOR,
        OP_LEFT_SHIFT,
        OP_RIGHT_SHIFT,
        OP_LOGICAL_OR,
        OP_LOGICAL_AND,
        OP_EQUAL,
        OP_NOT_EQUAL,
        OP_IDENTICAL,
        OP_NOT_IDENTICAL,
        OP_LESS_EQUAL,
        OP_GREATER_EQUAL,
        OP_LESS,
        OP_GREATER,
        // special
        OP_ASSIGN,
    };

    enum value_type
    {
        VT_UNDEFINED,
        VT_NULL,
        VT_INTEGER,
        VT_FLOATING_POINT,
        VT_STRING,
        VT_FUNCTION,
        VT_OBJECT,
        VT_BUILTIN,
        VT_HOST,
        VT_SCOPE,
        VT_BOUND_METHOD,
        VT_TYPE
    };
    typedef std::vector<ref_ptr<value> > parameter_list;

    virtual ~value()
    {
    }

    double GetDouble() const
    {
        return toFloat(false);
    }
    virtual value_type getType() const = 0;
    virtual std::string toString() const;
    virtual int toInt() const;
    virtual double toFloat(bool allow_throw = false) const;
    virtual bool toBoolean() const;
    // toString is meant as a type conversion, whereas stringify
    // is for debuggers and the like
    virtual std::string stringify() const;

    virtual ref_ptr<value> eliminateWrappers();
    virtual ref_ptr<value> duplicate();
    virtual ref_ptr<value> duplicateAndResolveDelegate() { return duplicate(); }

    virtual ixlib_iterator begin() const;
    virtual ixlib_iterator end() const;
    virtual TSize size() const;

    virtual ref_ptr<value> lookup(std::string const &identifier);
    virtual ref_ptr<value> subscript(value const &index);
    virtual ref_ptr<value> call(parameter_list const &parameters);
    virtual ref_ptr<value> callAsMethod(ref_ptr<value> instance, parameter_list const &parameters);
    virtual ref_ptr<value> construct(parameter_list const &parameters);
    virtual ref_ptr<value> assign(ref_ptr<value> op2);

    virtual ref_ptr<value> operatorUnary(operator_id op) const;
    virtual ref_ptr<value> operatorBinary(operator_id op, ref_ptr<value> op2) const;
    virtual ref_ptr<value> operatorBinaryShortcut(operator_id op, expression const &op2, context const &ctx) const;
    virtual ref_ptr<value> operatorUnaryModifying(operator_id op);
    virtual ref_ptr<value> operatorBinaryModifying(operator_id op, ref_ptr<value> op2);
    virtual void setName(const std::string& name) {}

    static operator_id token2operator(scanner::token const &token, bool unary = false, bool prefix = false);
    static std::string operator2string(operator_id op);
    static std::string valueType2string(value_type vt);

    virtual ref_ptr<value> ExecuteBoolConditional(ref_ptr<value> trueResult, ref_ptr<value> falseResult) const;
};

// obviously, any value can have methods, but with this neat little
// interface implementing methods has just become easier.
class value_with_methods : public value
{
    class bound_method : public value
    {
        std::string Identifier;
        ref_ptr<value_with_methods, value> Parent;

    public:
        bound_method(std::string const &identifier, ref_ptr<value_with_methods, value> parent);

        value_type getType() const
        {
            return VT_BOUND_METHOD;
        }

        ref_ptr<value> duplicate();
        ref_ptr<value> call(parameter_list const &parameters);
    };

public:
    ref_ptr<value> lookup(std::string const &identifier);
    virtual ref_ptr<value> callMethod(std::string const &identifier, parameter_list const &parameters) = 0;
};

// obviously, any value can already represent a scope ("lookup" member!).
// the list_scope class is an explicit scope that can "swallow"
// (=unite with) other scopes and keeps a list of registered members
class list_scope : public value
{
protected:
#ifdef _MSC_VER
    typedef std::map<std::string, ref_ptr<value> > member_map;
#else
    typedef std::hash_map<std::string, ref_ptr<value>, string_hash> member_map;
#endif
    typedef std::vector<ref_ptr<value> > swallowed_list;

    member_map MemberMap;
    swallowed_list SwallowedList;

public:
    value_type getType() const
    {
        return VT_SCOPE;
    }

    virtual ~list_scope()
    {
    }

    ref_ptr<value> lookup(std::string const &identifier);

    void unite(ref_ptr<value> scope);
    void separate(ref_ptr<value> scope);
    void clearScopes();

    // New methods required for overloading list_scope (to control assignment)
    virtual ref_ptr<list_scope, value> construct();
    virtual ref_ptr<value> construct_unit(ref_ptr<value> opn1, ref_ptr<value> opn2);
    virtual ref_ptr<value> assignment(context const &ctx, ref_ptr<expression> opn1, ref_ptr<expression> opn2);
    virtual ref_ptr<value> operatorBinaryModifying(context const &ctx, ref_ptr<expression> opn1, operator_id op,
                                                   ref_ptr<expression> opn2);
    virtual std::string getName(ref_ptr<value> object);

    virtual bool hasMember(std::string const &name) const;
    virtual void addMember(std::string const &name, ref_ptr<value> member);
    virtual ref_ptr<value> defineMember(context const& ctx, const std::string& name, ref_ptr<expression> expression, const std::shared_ptr<IValueCreatorService>& valueCreatorService);
    virtual void removeMember(std::string const &name);
    virtual void clearMembers();
    virtual void setLineNumber(int lineNumber);
    virtual void setLastLineNumber(int lineNumber);

    virtual void clear();
};

class js_array : public value_with_methods
{
private:
    typedef value_with_methods super;

protected:
    typedef std::vector<ref_ptr<value> > value_array;
    value_array Array;

public:
    js_array()
    {
    }

    js_array(TSize size);

    js_array(value_array::const_iterator first, value_array::const_iterator last) : Array(first, last)
    {
    }

    js_array(js_array const &src) : Array(src.Array)
    {
    }

    value_type getType() const
    {
        return VT_BUILTIN;
    }

    std::string stringify() const;

    ref_ptr<value> duplicate();

    ref_ptr<value> lookup(std::string const &identifier);
    ref_ptr<value> subscript(value const &index);
    ref_ptr<value> callMethod(std::string const &identifier, parameter_list const &parameters);

    TSize size() const
    {
        return Array.size();
    }

    void resize(TSize size);
    ref_ptr<value> &operator[](TIndex idx);
    void push_back(ref_ptr<value> val);
};

class expression;

ref_ptr<value> makeUndefined();
ref_ptr<value> makeNull();
ref_ptr<value> makeValue(signed long val);
ref_ptr<value> makeConstant(signed long val);
ref_ptr<value> makeValue(signed int val);
ref_ptr<value> makeConstant(signed int val);
ref_ptr<value> makeValue(unsigned long val);
ref_ptr<value> makeConstant(unsigned long val);
ref_ptr<value> makeValue(unsigned int val);
ref_ptr<value> makeConstant(_w64 unsigned int val);
ref_ptr<value> makeConstant(unsigned __int64);
ref_ptr<value> makeValue(double val);
ref_ptr<value> makeConstant(double val);
ref_ptr<value> makeConstantFromLong(double val);
ref_ptr<value> makeValue(std::string const &val);
ref_ptr<value> makeConstant(std::string const &val);
ref_ptr<value> makeArray(TSize size = 0);
ref_ptr<value> makeLValue(ref_ptr<value> target);
ref_ptr<value> wrapConstant(ref_ptr<value> val);

class interpreter
{
public:
    ref_ptr<list_scope, value> RootScope;
    std::shared_ptr<IValueCreatorService> m_valueCreatorService;

public:
    interpreter();
    interpreter(ref_ptr<list_scope, value> scope);
    interpreter(ref_ptr<list_scope, value> scope, std::shared_ptr<IValueCreatorService> valueCreatorService);
    ~interpreter();

    ref_ptr<expression> parse(std::string const &str);
    ref_ptr<expression> parse(std::istream &istr);
    ref_ptr<value> execute(std::string const &str);
    ref_ptr<value> execute(std::istream &istr);
    ref_ptr<value> execute(ref_ptr<expression> expr);

private:
    ref_ptr<value> evaluateCatchExits(ref_ptr<expression> expr);
};

void addGlobal(interpreter &ip);
void addMath(interpreter &ip);
void addStandardLibrary(interpreter &ip);

}}

#endif
