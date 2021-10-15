// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#include <strstream>
#include "ixlib_i18n.hh"
#include <ixlib_base.hh>
#include <ixlib_scanjs.hh>
#include <ixlib_numconv.hh>
#include <ixlib_js_internals.hh>
#include <ixlib_token_javascript.hh>
#include "NativeValueCreatorService.h"

// tools ----------------------------------------------------------------------
#define ADVANCE        \
    first++;           \
    if (first == last) \
    EXJS_THROW(ECJS_UNEXPECTED_EOF)
#define EXPECT(WHAT, STRINGIFIED)       \
    if (first == last)                  \
        EXJS_THROW(ECJS_UNEXPECTED_EOF) \
    if (first->Type != WHAT)            \
    EXJS_THROWINFOTOKEN(ECJS_UNEXPECTED, ("'" + first->Text + "' " + _("instead of ") + STRINGIFIED).c_str(), *first)

static char *dummy_i18n_referencer = N_("instead of ");

using namespace ixion;
using namespace javascript;

namespace {

ref_ptr<expression> makeConstantExpression(ref_ptr<value> val, code_location const &loc)
{
    ref_ptr<expression> result(new constant(val, loc));
    return result;
}

}

// exception texts ------------------------------------------------------------
static char *(PlainText[]) = {
    N_("Unterminated comment"),
    N_("Cannot convert"),
    N_("Invalid operation"),
    N_("Unexpected token encountered"),
    N_("Unexpected end of file"),
    N_("Cannot modify rvalue"),
    N_("Unknown identifier"),
    N_("Unknown operator"),
    N_("Invalid non-local exit"),
    N_("Invalid number of arguments"),
    N_("Invalid token encountered"),
    N_("Cannot redeclare identifier"),
    N_("Constructor called on constructed object"),
    N_("No superclass available"),
    N_("Division by zero"),
};

// no_location_javascript_exception -------------------------------------------
const char *no_location_javascript_exception::getText() const
{
    return _(PlainText[Error]);
}

// javascript_exception -------------------------------------------------------
javascript_exception::javascript_exception(TErrorCode error, code_location const &loc, char const *info, char *module,
                                           TIndex line)
    : base_exception(error, NULL, module, line, "JS")
{
    HasInfo = true;

    try
    {
        string temp = loc.stringify();

        if (info)
        {
            temp += ": \n\t";
            temp += info;
        }

        strcpy_s(Info, temp.c_str());
    }
    catch (...)
    {
    }
}

javascript_exception::javascript_exception(no_location_javascript_exception const &half_ex,
                                           javascript::code_location const &loc)
    : base_exception(half_ex.Error, NULL, half_ex.Module, half_ex.Line, half_ex.Category)
{
    HasInfo = true;

    try
    {
        string temp = loc.stringify() + ": \n\t" + half_ex.Info;
        strcpy_s(Info, temp.c_str());
    }
    catch (...)
    {
    }
}

const char *javascript_exception::getText() const
{
    return _(PlainText[Error]);
}

// code_location --------------------------------------------------------------
code_location::code_location(const scanner::token &tok) : Line(tok.Line)
{
}

code_location::code_location(TIndex line) : Line(line)
{
}

string code_location::stringify() const
{
    return "line: " + unsigned2dec(Line);
}

// context --------------------------------------------------------------------
context::context(ref_ptr<list_scope, value> scope) : DeclarationScope(scope.get()), LookupScope(scope.get())
{
}

context::context(ref_ptr<value> scope) : LookupScope(scope)
{
}

context::context(ref_ptr<list_scope, value> decl_scope, ref_ptr<value> lookup_scope)
    : DeclarationScope(decl_scope), LookupScope(lookup_scope)
{
}

UnitParserFunction g_parseUnit;

void ixion::javascript::setUnitParser(UnitParserFunction parseUnit)
{
    g_parseUnit = parseUnit;
}

// parser ---------------------------------------------------------------------
namespace {

ref_ptr<expression> parseInstructionList(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                         bool scoped);
ref_ptr<expression> parseSwitch(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                string const &label);
ref_ptr<expression> parseVariableDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last);
ref_ptr<expression> parseConstantDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last);
ref_ptr<expression> parseFunctionDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last);
ref_ptr<expression> parseMethodDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last);
ref_ptr<expression> parseConstructorDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                                string const &class_id);
ref_ptr<expression> parseClassDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last);
ref_ptr<expression> parseInstruction(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last);
ref_ptr<expression> parseExpression(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                    int precedence = 0);

ref_ptr<expression> parseInstructionList(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                         bool scoped)
{
    auto_ptr<instruction_list> ilist(scoped ? new scoped_instruction_list(*first) : new instruction_list(*first));

    while (first != last && first->Type != '}')
    {
        ref_ptr<expression> expr = parseInstruction(valueCreator, first, last);

        if (expr.get() != NULL)
            ilist->add(expr);
    }

    return ilist.release();
}

ref_ptr<expression> parseSwitch(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                string const &label)
{
    code_location loc(*first);

    ADVANCE
    EXPECT('(', _("'(' in switch statement"))
    ADVANCE

    ref_ptr<expression> discr = parseExpression(valueCreator, first, last);
    EXPECT(')', _("')' in switch statement"))
    ADVANCE

    EXPECT('{', _("'{' in switch statement"))
    ADVANCE

    auto_ptr<js_switch> sw;
    if (label.size())
    {
        auto_ptr<js_switch> tsw(new js_switch(discr, label, loc));
        sw = tsw;
    }
    else
    {
        auto_ptr<js_switch> tsw(new js_switch(discr, loc));
        sw = tsw;
    }

    ref_ptr<expression> dvalue;
    auto_ptr<instruction_list> ilist;

    while (first != last && first->Type != '}')
    {
        if (first->Type == TT_JS_CASE)
        {
            if (ilist.get())
                sw->addCase(dvalue, ilist.release());

            auto_ptr<instruction_list> tilist(new instruction_list(*first));
            ilist = tilist;

            ADVANCE

            dvalue = parseExpression(valueCreator, first, last);
            EXPECT(':', _("':' in case label"))
            ADVANCE
        }
        else if (first->Type == TT_JS_DEFAULT)
        {
            if (ilist.get())
                sw->addCase(dvalue, ilist.release());

            auto_ptr<instruction_list> tilist(new instruction_list(*first));
            ilist = tilist;

            ADVANCE
            dvalue = NULL;
            EXPECT(':', _("':' in default label"))
            ADVANCE
        }
        else
        {
            ref_ptr<expression> expr = parseInstruction(valueCreator, first, last);

            if (ilist.get() && expr.get() != NULL)
                ilist->add(expr);
        }
    }

    if (ilist.get())
        sw->addCase(dvalue, ilist.release());

    EXPECT('}', _("'}' in switch statement"))
    ADVANCE

    return sw.release();
}

ref_ptr<expression> parseVariableDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last)
{
    code_location loc(*first);

    EXPECT(TT_JS_IDENTIFIER, _("variable identifier"))
    string id = first->Text;
    ADVANCE

    ref_ptr<expression> def;

    if (first->Type == '=')
    {
        ADVANCE
        def = parseExpression(valueCreator, first, last);
    }

    ref_ptr<expression> result = new variable_declaration(valueCreator, id, def, loc);

    if (first->Type == ',')
    {
        auto_ptr<instruction_list> ilist(new instruction_list(*first));
        ilist->add(result);

        while (first->Type == ',')
        {
            ADVANCE

            code_location loc(*first);

            EXPECT(TT_JS_IDENTIFIER, _("variable identifier"))
            id = first->Text;
            ADVANCE

            if (first->Type == '=')
            {
                ADVANCE
                def = parseExpression(valueCreator, first, last);
            }

            ref_ptr<expression> decl = new variable_declaration(valueCreator, id, def, loc);
            ilist->add(decl);
        }
        result = ilist.release();
    }

    return result;
}

ref_ptr<expression> parseConstantDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last)
{
    code_location loc(*first);

    EXPECT(TT_JS_IDENTIFIER, _("constant identifier"))
    string id = first->Text;
    ADVANCE

    ref_ptr<expression> def;
    EXPECT('=', _("initializer for constant"))
    ADVANCE
    def = parseExpression(valueCreator, first, last);

    ref_ptr<expression> result = new constant_declaration(valueCreator, id, def, loc);

    if (first->Type == ',')
    {
        auto_ptr<instruction_list> ilist(new instruction_list(*first));
        ilist->add(result);

        while (first->Type == ',')
        {
            ADVANCE

            code_location loc(*first);

            EXPECT(TT_JS_IDENTIFIER, _("constant identifier"))
            id = first->Text;
            ADVANCE

            EXPECT('=', _("initializer for constant"))
            ADVANCE
            def = parseExpression(valueCreator, first, last);

            ref_ptr<expression> decl = new constant_declaration(valueCreator, id, def, loc);
            ilist->add(decl);
        }
        result = ilist.release();
    }

    return result;
}

ref_ptr<expression> parseFunctionDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last)
{
    code_location loc(*first);

    string id;
    if (first->Type == TT_JS_IDENTIFIER)
    {
        id = first->Text;
        ADVANCE
    }
    // Allow unnamed functions
    else if (first->Type != '(')
    {
        EXPECT(TT_JS_IDENTIFIER, _("function identifier"))
    }

    EXPECT('(', _("'(' in function declaration"))
    ADVANCE

    ixion::javascript::function::parameter_name_list pnames;
    ixion::javascript::function::parameter_name_list types;
    std::string returnType;

    while (first->Type != ')')
    {
        EXPECT(TT_JS_IDENTIFIER, _("parameter identifier"))
        pnames.push_back(first->Text);
        types.push_back("");
        ADVANCE

        if(first->Type == ':')
        {
            ADVANCE
            EXPECT(TT_JS_IDENTIFIER, _("parameter identifier"))
            types.back()  = first->Text;
            ADVANCE
        }
        if (first->Type == ',')
        {
            ADVANCE
        }
        else
        {
            EXPECT(')', _("')' in function declaration"))
        }
    }
    EXPECT(')', _("')' in function declaration"))
    ADVANCE

    if (first->Type == ':')
    {
        ADVANCE
        EXPECT(TT_JS_IDENTIFIER, _("parameter identifier"))
        returnType = first->Text;
        ADVANCE
    }
    EXPECT('{', _("'{' in function definition"))
    ADVANCE

    ref_ptr<expression> body = parseInstructionList(valueCreator, first, last, false);

    EXPECT('}', "'}' in method definition")
    first++;

    ref_ptr<expression> result = new function_declaration(valueCreator, id, pnames, body, types, returnType, loc);
    return result;
}

ref_ptr<expression> parseMethodDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last)
{
    code_location loc(*first);

    EXPECT(TT_JS_IDENTIFIER, _("method identifier"))
    string id = first->Text;
    ADVANCE

    EXPECT('(', _("'(' in method declaration"))
    ADVANCE

    method::parameter_name_list pnames;

    while (first->Type != ')')
    {
        EXPECT(TT_JS_IDENTIFIER, _("parameter identifier"))
        pnames.push_back(first->Text);
        ADVANCE

        if (first->Type == ',')
        {
            ADVANCE
        }
        else
        {
            EXPECT(')', _("')' in function declaration"))
        }
    }

    EXPECT(')', _("')' in method declaration"))
    ADVANCE

    EXPECT('{', _("'{' in method definition"))
    ADVANCE

    ref_ptr<expression> body = parseInstructionList(valueCreator, first, last, false);

    EXPECT('}', "'}' in method definition")
    first++;

    ref_ptr<expression> result = new method_declaration(valueCreator, id, pnames, body, loc);
    return result;
}

ref_ptr<expression> parseConstructorDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last,
                                                string const &class_id)
{
    code_location loc(*first);

    EXPECT(TT_JS_IDENTIFIER, _("constructor identifier"))

    if (first->Text != class_id)
    {
        EXJS_THROWINFOTOKEN(ECJS_UNEXPECTED,
                            ("'" + first->Text + "' " + _("instead of ") + _("class identifier")).c_str(), *first)
    }

    ADVANCE

    EXPECT('(', _("'(' in constructor declaration"))
    ADVANCE

    method::parameter_name_list pnames;

    while (first->Type != ')')
    {
        EXPECT(TT_JS_IDENTIFIER, _("parameter identifier"))
        pnames.push_back(first->Text);
        ADVANCE

        if (first->Type == ',')
        {
            ADVANCE
        }
        else
        {
            EXPECT(')', _("')' in constructor declaration"))
        }
    }

    EXPECT(')', _("')' in constructor declaration"))
    ADVANCE

    EXPECT('{', _("'{' in constructor definition"))
    ADVANCE

    ref_ptr<expression> body = parseInstructionList(valueCreator, first, last, false);

    EXPECT('}', "'}' in constructor definition")
    first++;

    ref_ptr<expression> result = new constructor_declaration(valueCreator, pnames, body, loc);
    return result;
}

ref_ptr<expression> parseClassDeclaration(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last)
{
    code_location loc(*first);

    EXPECT(TT_JS_IDENTIFIER, _("class identifier"))
    string id = first->Text;
    ADVANCE

    ref_ptr<expression> superclass;
    if (first->Type == TT_JS_EXTENDS)
    {
        ADVANCE
        superclass = parseExpression(valueCreator, first, last);
    }

    EXPECT('{', _("'{' in class declaration"))
    ADVANCE

    auto_ptr<js_class_declaration> decl(new js_class_declaration(id, superclass, loc));

    while (first != last && first->Type != '}')
    {
        if (first->Type == TT_JS_STATIC)
        {
            ADVANCE

            if (first->Type == TT_JS_FUNCTION)
            {
                ADVANCE
                decl->addStaticMethod(parseFunctionDeclaration(valueCreator, first, last));
            }
            else
            {
                if (first->Type == TT_JS_CONST)
                {
                    ADVANCE
                    decl->addStaticVariable(parseConstantDeclaration(valueCreator, first, last));
                }
                else
                {
                    ADVANCE
                    decl->addStaticVariable(parseVariableDeclaration(valueCreator, first, last));
                }
                EXPECT(';', "';'")
                first++;
            }
        }
        else
        {
            if (first->Type == TT_JS_FUNCTION)
            {
                ADVANCE
                decl->addMethod(parseMethodDeclaration(valueCreator, first, last));
            }
            else if (first->Type == TT_JS_CONSTRUCTOR)
            {
                ADVANCE

                EXPECT(TT_JS_FUNCTION, _("'function' keyword"))
                ADVANCE

                decl->setConstructor(parseConstructorDeclaration(valueCreator, first, last, id));
            }
            else
            {
                if (first->Type == TT_JS_CONST)
                {
                    ADVANCE
                    decl->addVariable(parseConstantDeclaration(valueCreator, first, last));
                }
                else
                {
                    ADVANCE
                    decl->addVariable(parseVariableDeclaration(valueCreator, first, last));
                }
                EXPECT(';', "';'")
                first++;
            }
        }
    }

    EXPECT('}', "'}' in class declaration")
    first++;

    return decl.release();
}

ref_ptr<expression> parseInstruction(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last)
{
    if (first == last)
        EXJS_THROW(ECJS_UNEXPECTED_EOF)

    string label;
    if (first + 1 != last && first[1].Type == ':')
    {
        label = first->Text;
        ADVANCE
        ADVANCE
    }

    ref_ptr<expression> result;

    if (first->Type == '{')
    {
        ADVANCE
        result = parseInstructionList(valueCreator, first, last, true);
        EXPECT('}', "'}'")
        first++;
    }
    else if (first->Type == TT_JS_VAR)
    {
        ADVANCE

        result = parseVariableDeclaration(valueCreator, first, last);

        EXPECT(';', "';'")
        first++;
    }
    else if (first->Type == TT_JS_CONST)
    {
        ADVANCE

        result = parseConstantDeclaration(valueCreator, first, last);

        EXPECT(';', "';'")
        first++;
    }
    else if (first->Type == TT_JS_FUNCTION)
    {
        ADVANCE
        result = parseFunctionDeclaration(valueCreator, first, last);
    }
    else if (first->Type == TT_JS_CLASS)
    {
        ADVANCE
        result = parseClassDeclaration(valueCreator, first, last);
    }
    else if (first->Type == TT_JS_IF)
    {
        code_location loc(*first);
        ADVANCE

        EXPECT('(', _("'(' in if statement"))
        ADVANCE

        ref_ptr<expression> cond = parseExpression(valueCreator, first, last);
        EXPECT(')', _("')' in if statement"))
        first++;
        ref_ptr<expression> if_expr = parseInstruction(valueCreator, first, last);
        ref_ptr<expression> else_expr;

        if (first != last && first->Type == TT_JS_ELSE)
        {
            ADVANCE
            else_expr = parseInstruction(valueCreator, first, last);
        }
        result = new js_if(valueCreator, cond, if_expr, else_expr, loc);
    }
    else if (first->Type == TT_JS_SWITCH)
    {
        result = parseSwitch(valueCreator, first, last, label);
    }
    else if (first->Type == TT_JS_WHILE)
    {
        code_location loc(*first);

        ADVANCE
        EXPECT('(', _("'(' in while statement"))
        ADVANCE

        ref_ptr<expression> cond = parseExpression(valueCreator, first, last);
        EXPECT(')', _("')' in while statement"))
        first++;

        ref_ptr<expression> loop_expr = parseInstruction(valueCreator, first, last);

        if (label.size())
        {
            result = new js_while(cond, loop_expr, label, loc);
            label = "";
        }
        else
            result = new js_while(cond, loop_expr, loc);
    }
    else if (first->Type == TT_JS_DO)
    {
        code_location loc(*first);

        ADVANCE
        ref_ptr<expression> loop_expr = parseInstruction(valueCreator, first, last);

        EXPECT(TT_JS_WHILE, _("'while' in do-while"))
        ADVANCE

        EXPECT('(', _("'(' in do-while statement"))
        ADVANCE

        ref_ptr<expression> cond = parseExpression(valueCreator, first, last);
        EXPECT(')', _("')' in do-while statement"))
        first++;

        if (label.size())
        {
            result = new js_do_while(cond, loop_expr, label, loc);
            label = "";
        }
        else
            result = new js_do_while(cond, loop_expr, loc);
    }
    else if (first->Type == TT_JS_FOR)
    {
        code_location loc(*first);

        ADVANCE

        EXPECT('(', _("'(' in for statement"))
        ADVANCE

        ref_ptr<expression> init_expr;
        if (first->Type == TT_JS_VAR)
        {
            ADVANCE

            init_expr = parseVariableDeclaration(valueCreator, first, last);
        }
        else
            init_expr = parseExpression(valueCreator, first, last);

        if (first->Type == TT_JS_IN)
        {
            // for (iterator in list)
            ADVANCE
            ref_ptr<expression> array_expr = parseExpression(valueCreator, first, last);

            EXPECT(')', _("')' in for statement"))
            first++;

            ref_ptr<expression> loop_expr = parseInstruction(valueCreator, first, last);

            if (label.size())
            {
                result = new js_for_in(init_expr, array_expr, loop_expr, label, loc);
                label = "";
            }
            else
                result = new js_for_in(init_expr, array_expr, loop_expr, label, loc);
        }
        else
        {
            // for (;;) ...
            EXPECT(';', _("';' in for statement"))
            ADVANCE

            ref_ptr<expression> cond_expr = parseExpression(valueCreator, first, last);

            EXPECT(';', _("';' in for statement"))
            ADVANCE

            ref_ptr<expression> update_expr = parseExpression(valueCreator, first, last);

            EXPECT(')', _("')' in for statement"))
            first++;

            ref_ptr<expression> loop_expr = parseInstruction(valueCreator, first, last);

            if (label.size())
            {
                result = new js_for(init_expr, cond_expr, update_expr, loop_expr, label, loc);
                label = "";
            }
            else
                result = new js_for(init_expr, cond_expr, update_expr, loop_expr, loc);
        }
    }
    else if (first->Type == TT_JS_RETURN)
    {
        code_location loc(*first);
        ADVANCE

        if (first->Type != ';')
            result = new js_return(parseExpression(valueCreator, first, last), loc);
        else
            result = new js_return(makeConstantExpression(valueCreator->MakeNull(), loc), loc);

        EXPECT(';', "';'")
        first++;
    }
    else if (first->Type == TT_JS_BREAK)
    {
        code_location loc(*first);
        ADVANCE

        if (first->Type != ';')
        {
            EXPECT(TT_JS_IDENTIFIER, _("break label"))
            result = new js_break(first->Text, loc);
            ADVANCE
        }
        else
            result = new js_break(loc);

        EXPECT(';', "';'")
        first++;
    }
    else if (first->Type == TT_JS_CONTINUE)
    {
        code_location loc(*first);
        ADVANCE

        if (first->Type != ';')
        {
            EXPECT(TT_JS_IDENTIFIER, _("continue label"))
            result = new js_continue(first->Text, loc);
            ADVANCE
        }
        else
            result = new js_continue(loc);

        EXPECT(';', "';'")
        first++;
    }
    else if (first->Type == ';')
    {
        result = makeConstantExpression(ref_ptr<value>(NULL), *first);
        first++;
    }
    else
    {
        // was nothing else, must be expression
        result = parseExpression(valueCreator, first, last);
        EXPECT(';', "';'")
        first++;
    }

    if (label.size())
    {
        result = new break_label(label, result, *first);
    }

    return result;
}

namespace {

int const PREC_COMMA = 10,  // , (if implemented)
    PREC_THROW = 20,        // throw (if implemented)
    PREC_ASSIGN = 30,       // += and friends [ok]
    PREC_CONDITIONAL = 40,  // ?: [ok]
    PREC_LOG_OR = 50,       // || [ok]
    PREC_LOG_AND = 60,      // && [ok]
    PREC_BIT_OR = 70,       // | [ok]
    PREC_BIT_XOR = 80,      // ^ [ok]
    PREC_BIT_AND = 90,      // & [ok]
    PREC_EQUAL = 100,       // == != === !=== [ok]
    PREC_COMP = 110,        // < <= > >= [ok]
    PREC_SHIFT = 120,       // << >> [ok]
    PREC_ADD = 130,         // + - [ok]
    PREC_MULT = 140,        // * / % [ok]
    PREC_UNARY = 160,       // new + - ++ -- ! ~
    PREC_FUNCALL = 170,     // ()
    PREC_SUBSCRIPT = 180,   // [] .
    PREC_TERM = 200;        // literal identifier

}

ref_ptr<expression> parseExpression(const std::shared_ptr<IValueCreatorService>& valueCreator, scanner::token_iterator &first, scanner::token_iterator const &last, int precedence)
{
    /*
    precedence:
    the called routine will continue parsing as long as the encountered
    operators all have precedence greater than or equal to the given parameter.
    */

    // an EOF condition cannot legally occur inside
    // or at the end of an expression.

    if (first == last)
        EXJS_THROW(ECJS_UNEXPECTED_EOF)

    ref_ptr<expression> expr;

    // parse prefix unaries -----------------------------------------------------
    if (precedence <= PREC_UNARY)
    {
        if (first->Type == TT_JS_NEW)
        {
            code_location loc(*first);
            ADVANCE

            ref_ptr<expression> cls = parseExpression(valueCreator, first, last, PREC_SUBSCRIPT);

            EXPECT('(', _("'(' in 'new' expression"))
            ADVANCE

            function_call::parameter_expression_list pexps;
            while (first->Type != ')')
            {
                pexps.push_back(parseExpression(valueCreator, first, last));

                if (first->Type == ',')
                {
                    ADVANCE
                }
                else
                {
                    EXPECT(')', _("')' in 'new' expression"))
                }
            }

            EXPECT(')', _("')' in 'new' expression"))
            ADVANCE

            expr = new construction(cls, pexps, loc);
        }
        if (first->Type == TT_JS_INCREMENT || first->Type == TT_JS_DECREMENT)
        {
            code_location loc(*first);
            value::operator_id op = value::token2operator(*first, true, true);
            ADVANCE
            expr = new modifying_unary_operator(op, parseExpression(valueCreator, first, last, PREC_UNARY), loc);
        }
        if (first->Type == '+' || first->Type == '-' || first->Type == '!' || first->Type == '~')
        {
            code_location loc(*first);
            value::operator_id op = value::token2operator(*first, true, true);
            ADVANCE
            expr = new unary_operator(op, parseExpression(valueCreator, first, last, PREC_UNARY), loc);
        }
    }

    // parse parentheses --------------------------------------------------------
    if (first->Type == '(')
    {
        ADVANCE
        expr = parseExpression(valueCreator, first, last);
        EXPECT(')', "')'")
        ADVANCE
    }

    bool isConstantExpression = false;
    // parse term ---------------------------------------------------------------
    if (expr.get() == NULL && precedence <= PREC_TERM)
    {
        if (first->Type == TT_JS_LIT_INT)
        {
            long lvalue = evalUnsigned(first->Text);
            double dvalue;
            if (sscanf_s(first->Text.c_str(), "%le", &dvalue) && dvalue > lvalue)
            {
                // Check bounds of input value.
                expr = makeConstantExpression(valueCreator->MakeConstant(dvalue), *first);
            }
            else
                expr = makeConstantExpression(valueCreator->MakeConstant(lvalue), *first);
            isConstantExpression = true;
            ADVANCE
        }
        else if (first->Type == TT_JS_LIT_FLOAT)
        {
            expr = makeConstantExpression(valueCreator->MakeConstant(evalFloat(first->Text)), *first);
            isConstantExpression = true;
            ADVANCE
        }
        else if (first->Type == TT_JS_LIT_STRING)
        {
            expr = makeConstantExpression(valueCreator->MakeConstant(parseCEscapes(first->Text.substr(1, first->Text.size() - 2))),
                                          *first);
            ADVANCE
        }
        else if (first->Type == TT_JS_LIT_TRUE)
        {
            expr = makeConstantExpression(valueCreator->MakeConstant(true), *first);
            ADVANCE
        }
        else if (first->Type == TT_JS_LIT_FALSE)
        {
            expr = makeConstantExpression(valueCreator->MakeConstant(false), *first);
            ADVANCE
        }
        else if (first->Type == TT_JS_LIT_UNDEFINED)
        {
            expr = makeConstantExpression(valueCreator->MakeUndefined(), *first);
            ADVANCE
        }
        else if (first->Type == TT_JS_IDENTIFIER)
        {
            expr = new lookup_operation(first->Text, *first);
            ADVANCE
        }
        else if (first->Type == TT_JS_NULL)
        {
            expr = makeConstantExpression(valueCreator->MakeNull(), *first);
            ADVANCE
        }
        else if (first->Type == TT_JS_FUNCTION)
        {
            ADVANCE
            expr = parseFunctionDeclaration(valueCreator, first, last);
        }
    }

    if (expr.get() == NULL)
        EXJS_THROWINFOTOKEN(ECJS_UNEXPECTED, ("'" + first->Text + "' instead of expression").c_str(), *first)

    bool parsed_something;
    do
    {
        parsed_something = false;

        // parse postfix "subscripts" ---------------------------------------------
        if (first->Type == '(' && precedence <= PREC_FUNCALL)
        {
            code_location loc(*first);
            ADVANCE

            function_call::parameter_expression_list pexps;
            while (first->Type != ')')
            {
                pexps.push_back(parseExpression(valueCreator, first, last));

                if (first->Type == ',')
                {
                    ADVANCE
                }
                else
                {
                    EXPECT(')', _("')' in 'new' expression"))
                }
            }

            EXPECT(')', _("')' in function call"))
            ADVANCE

            expr = new function_call(valueCreator, expr, pexps, loc);
            parsed_something = true;
        }
        // parse postfix unary ----------------------------------------------------
        else if ((first->Type == TT_JS_INCREMENT || first->Type == TT_JS_DECREMENT) && precedence <= PREC_UNARY)
        {
            value::operator_id op = value::token2operator(*first, true);
            expr = new modifying_unary_operator(op, expr, *first);
            parsed_something = true;
            ADVANCE
        }
        // parse special binary operators -----------------------------------------
        else if (first->Type == '.' && precedence <= PREC_SUBSCRIPT)
        {
            ADVANCE
            expr = new lookup_operation(expr, first->Text, *first);
            ADVANCE
            parsed_something = true;
        }
        else if (first->Type == '[' && precedence <= PREC_SUBSCRIPT)
        {
            code_location loc(*first);
            ADVANCE

            ref_ptr<expression> index = parseExpression(valueCreator, first, last);

            EXPECT(']', _("']' in subscript"))
            ADVANCE

            expr = new subscript_operation(expr, index, loc);

            parsed_something = true;
        }

// parse regular binary operators -----------------------------------------
#define BINARY_OP(PRECEDENCE, EXPRESSION, TYPE)                               \
    else if ((EXPRESSION) && precedence < PRECEDENCE)                         \
    {                                                                         \
        code_location loc(*first);                                            \
        value::operator_id op = value::token2operator(*first);                \
        ADVANCE                                                               \
        ref_ptr<expression> right = parseExpression(valueCreator, first, last, PRECEDENCE); \
        expr = new TYPE##_operator(op, expr, right, loc);                     \
        parsed_something = true;                                              \
    }

        BINARY_OP(PREC_MULT, first->Type == '*' || first->Type == '/' || first->Type == '%', binary)
        BINARY_OP(PREC_ADD, first->Type == '+' || first->Type == '-', binary)
        BINARY_OP(PREC_SHIFT, first->Type == TT_JS_LEFT_SHIFT || first->Type == TT_JS_RIGHT_SHIFT, binary)
        BINARY_OP(PREC_COMP, first->Type == '>' || first->Type == '<' || first->Type == TT_JS_LESS_EQUAL ||
                                 first->Type == TT_JS_GREATER_EQUAL || first->Type == TT_JS_IDENTICAL ||
                                 first->Type == TT_JS_NOT_IDENTICAL,
                  binary)
        BINARY_OP(PREC_EQUAL, first->Type == TT_JS_EQUAL || first->Type == TT_JS_NOT_EQUAL, binary)
        BINARY_OP(PREC_BIT_AND, first->Type == '&', binary)
        BINARY_OP(PREC_BIT_XOR, first->Type == '^', binary)
        BINARY_OP(PREC_BIT_OR, first->Type == '|', binary)
        BINARY_OP(PREC_LOG_AND, first->Type == TT_JS_LOGICAL_AND, binary_shortcut)
        BINARY_OP(PREC_LOG_OR, first->Type == TT_JS_LOGICAL_OR, binary_shortcut)
        else if (first->Type == '?')
        {
            code_location loc(*first);
            ADVANCE
            ref_ptr<expression> op2 = parseExpression(valueCreator, first, last);
            if (first->Type != ':')
                EXJS_THROWINFO(ECJS_UNEXPECTED, (first->Text + " instead of ':'").c_str())
            ADVANCE
            ref_ptr<expression> op3 = parseExpression(valueCreator, first, last, PREC_CONDITIONAL);
            expr = new ternary_operator(expr, op2, op3, loc);
            parsed_something = true;
        }
        else if (first->Type == '=' && precedence <= PREC_ASSIGN)
        {
            code_location loc(*first);
            ADVANCE
            ref_ptr<expression> op2 = parseExpression(valueCreator, first, last);
            expr = new assignment(expr, op2, loc);
            parsed_something = true;
        }
        BINARY_OP(PREC_ASSIGN, first->Type == TT_JS_PLUS_ASSIGN || first->Type == TT_JS_MINUS_ASSIGN ||
                                   first->Type == TT_JS_MULTIPLY_ASSIGN || first->Type == TT_JS_DIVIDE_ASSIGN ||
                                   first->Type == TT_JS_MODULO_ASSIGN || first->Type == TT_JS_BIT_XOR_ASSIGN ||
                                   first->Type == TT_JS_BIT_AND_ASSIGN || first->Type == TT_JS_BIT_OR_ASSIGN ||
                                   first->Type == TT_JS_LEFT_SHIFT_ASSIGN || first->Type == TT_JS_RIGHT_SHIFT_ASSIGN,
                  modifying_binary)
        /// Parse for units
        else if (isConstantExpression && g_parseUnit)
        {
            ref_ptr<expression> unit = g_parseUnit(expr, first, last, 0);
            if (unit.get() != NULL)
            {
                expr = unit;
                parsed_something = true;
            }
        }
    } while (parsed_something);

    return expr;
}
}

// interpreter -----------------------------------------------------------------
interpreter::interpreter()
    : interpreter(new list_scope)
{
    ref_ptr<value> ac = new js_array_constructor();
    RootScope->addMember("Array", ac);
}

interpreter::interpreter(ref_ptr<list_scope, value> scope)
    : interpreter(scope, std::make_shared<NativeValueCreatorService>())
{
}

interpreter::interpreter(ref_ptr<list_scope, value> scope, std::shared_ptr<IValueCreatorService> valueCreatorService)
{
    RootScope = scope;
    m_valueCreatorService = valueCreatorService;
}

interpreter::~interpreter()
{
}

ref_ptr<expression> interpreter::parse(string const &str)
{
    // *** FIXME: this works around a bug in istrstream
    if (str.size() == 0)
    {
        return ref_ptr<expression>(NULL);
    }

    istrstream strm(str.data(), str.size());

    return parse(strm);
}

ref_ptr<expression> interpreter::parse(istream &istr)
{
    jsFlexLexer lexer(&istr);
    scanner scanner(lexer);

    scanner::token_list tokenlist = scanner.scan();
    scanner::token_iterator text = tokenlist.begin();

    if (tokenlist.empty())
        return NULL;
    else
    {
        RootScope->setLastLineNumber((int)tokenlist.back().Line);
        return parseInstructionList(m_valueCreatorService, text, tokenlist.end(), false);
    }
}

ref_ptr<value> interpreter::execute(string const &str)
{
    return execute(parse(str));
}

ref_ptr<value> interpreter::execute(istream &istr)
{
    return execute(parse(istr));
}

ref_ptr<value> interpreter::execute(ref_ptr<expression> expr)
{
    if (expr.get() == NULL)
        return ref_ptr<value>(NULL);

    return evaluateCatchExits(expr);
}

ref_ptr<value> interpreter::evaluateCatchExits(ref_ptr<expression> expr)
{
    ref_ptr<value> result;

    try
    {
        context ctx(RootScope);
        result = expr->evaluate(ctx);
    }
    catch (return_exception &re)
    {
        EXJS_THROWINFOLOCATION(ECJS_INVALID_NON_LOCAL_EXIT, "return", re.Location)
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

    return result;
}
