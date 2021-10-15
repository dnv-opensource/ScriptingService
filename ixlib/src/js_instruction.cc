// ----------------------------------------------------------------------------
//  Description      : Javascript interpreter
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by iXiONmedia, all rights reserved.
// ----------------------------------------------------------------------------

#include <ixlib_js_internals.hh>
#include <ixlib_token_javascript.hh>
#include <ixlib_iterator.hpp>

using namespace ixion;
using namespace javascript;
using namespace std;

// instruction_list -----------------------------------------------------------
ref_ptr<value> instruction_list::evaluate(context const &ctx) const
{
    ref_ptr<value> result;

    FOREACH_CONST (first, ExpressionList, expression_list)
        result = (*first)->evaluate(ctx);

    return result;
}

bool instruction_list::IsLookupExpression() const
{
    if (ExpressionList.size() == 1)
        return ExpressionList.front()->IsLookupExpression();
    return false;
}

bool instruction_list::TrySplitFunctionArguments(std::string& functionName, std::vector<std::string>& args) const
{
    if (ExpressionList.size() == 1)
        return ExpressionList.front()->TrySplitFunctionArguments(functionName, args);
    return false;
}

void instruction_list::add(ref_ptr<expression> expr)
{
    ExpressionList.push_back(expr);
}

string instruction_list::toString(int indent) const
{
    string result;

    for (size_t i = 0; i < ExpressionList.size(); ++i)
    {
        result += ExpressionList[i]->toString(indent);
        if (*result.rbegin() != '}')
            result += ";";
        result += "\r\n";
    }

    return result;
}

// scoped_instruction_list ----------------------------------------
ref_ptr<value> scoped_instruction_list::evaluate(context const &ctx) const
{
    /*
      ref_ptr<list_scope,value> scope = ctx.DeclarationScope->construct();
      scope->unite(ctx.LookupScope);
    */

    ref_ptr<value> result = instruction_list::evaluate(ctx);

    if (result.get())
        return result->duplicate();

    return ref_ptr<value>(NULL);

    // ATTENTION: this is a scope cancellation point.
}

// js_if ----------------------------------------------------------------------
js_if::js_if(const std::shared_ptr<IValueCreatorService>& valueCreator,ref_ptr<expression> cond, ref_ptr<expression> ifex, ref_ptr<expression> ifnotex, code_location const &loc)
    : expression(loc), Conditional(cond), IfExpression(ifex), IfNotExpression(ifnotex) , valueCreator(valueCreator)
{
}

ref_ptr<value> js_if::evaluate(context const &ctx) const
{
    ref_ptr<value> cond = Conditional->evaluate(ctx);
    try {
        if (cond->toBoolean())
            return IfExpression->evaluate(ctx);
        else if (IfNotExpression.get())
            return IfNotExpression->evaluate(ctx);
        else
            return ref_ptr<value>(NULL);
    }
    catch (std::exception)
    {
        return valueCreator->MakeConditional(cond, IfExpression, IfNotExpression, ctx);
    }
}

string js_if::toString(int indent, bool elseif) const
{
    string result = getIndent(indent);

    if (elseif)
        result += "else ";
    result += "if( " + Conditional->toString() + ")\r\n";

    if (dynamic_cast<instruction_list *>(IfExpression.get()))
    {
        result += getIndent(indent) + "{\r\n";
        result += IfExpression->toString(indent + 3);
        result += getIndent(indent) + "}";
    }
    else
        result += IfExpression->toString(indent + 3);

    if (dynamic_cast<instruction_list *>(IfNotExpression.get()))
    {
        if (*result.rbegin() != '}')
            result += ";";
        result += "\r\n" + getIndent(indent) + "else\r\n";
        result += getIndent(indent) + "{\r\n";
        result += IfNotExpression->toString(indent + 3);
        result += getIndent(indent) + "}";
    }
    else if (js_if *statement = dynamic_cast<js_if *>(IfNotExpression.get()))
    {
        if (*result.rbegin() != '}')
            result += ";\r\n";
        result += statement->toString(indent, true);
    }
    else if (IfNotExpression.get())
    {
        if (*result.rbegin() != '}')
            result += ";\r\n";
        result += getIndent(indent) + "else\r\n";
        result += IfNotExpression->toString(indent + 3);
    }

    return result;
}

string js_if::toString(int indent) const
{
    return toString(indent, false);
}

// js_while -------------------------------------------------------------------
js_while::js_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, code_location const &loc)
    : expression(loc), Conditional(cond), LoopExpression(loopex), HasLabel(false)
{
}

js_while::js_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, string const &label, code_location const &loc)
    : expression(loc), Conditional(cond), LoopExpression(loopex), HasLabel(true), Label(label)
{
}

ref_ptr<value> js_while::evaluate(context const &ctx) const
{
    ref_ptr<value> result;
    while (Conditional->evaluate(ctx)->toBoolean())
    {
        try
        {
            result = LoopExpression->evaluate(ctx);
        }
        catch (break_exception &be)
        {
            if (!be.HasLabel || (HasLabel && be.HasLabel && be.Label == Label))
                break;
            else
                throw;
        }
        catch (continue_exception &ce)
        {
            if (!ce.HasLabel || (HasLabel && ce.HasLabel && ce.Label == Label))
                continue;
            else
                throw;
        }
    }
    return result;
}

string js_while::toString(int indent) const
{
    string result = getIndent(indent) + "while(" + Conditional->toString() + ")\r\n";

    result += getIndent(indent) + "{\r\n";
    result += LoopExpression->toString(indent + 3);
    result += getIndent(indent) + "\r\n}";

    return result;
}

// js_do_while ----------------------------------------------------------------
js_do_while::js_do_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, code_location const &loc)
    : expression(loc), Conditional(cond), LoopExpression(loopex), HasLabel(false)
{
}

js_do_while::js_do_while(ref_ptr<expression> cond, ref_ptr<expression> loopex, string const &label,
                         code_location const &loc)
    : expression(loc), Conditional(cond), LoopExpression(loopex), HasLabel(true), Label(label)
{
}

ref_ptr<value> js_do_while::evaluate(context const &ctx) const
{
    ref_ptr<value> result;
    do
    {
        try
        {
            result = LoopExpression->evaluate(ctx);
        }
        catch (break_exception &be)
        {
            if (!be.HasLabel || (HasLabel && be.HasLabel && be.Label == Label))
                break;
            else
                throw;
        }
        catch (continue_exception &ce)
        {
            if (!ce.HasLabel || (HasLabel && ce.HasLabel && ce.Label == Label))
                continue;
            else
                throw;
        }
    } while (Conditional->evaluate(ctx)->toBoolean());

    return result;
}

string js_do_while::toString(int indent) const
{
    string result = getIndent(indent) + "do {\r\n";

    result += LoopExpression->toString(indent + 3);
    result += getIndent(indent) + "\r\n} while(" + Conditional->toString() + ")";

    return result;
}

// js_for ---------------------------------------------------------------------
js_for::js_for(ref_ptr<expression> init, ref_ptr<expression> cond, ref_ptr<expression> update, ref_ptr<expression> loop,
               code_location const &loc)
    : expression(loc), Initialization(init), Conditional(cond), Update(update), LoopExpression(loop), HasLabel(false)
{
}

js_for::js_for(ref_ptr<expression> init, ref_ptr<expression> cond, ref_ptr<expression> update, ref_ptr<expression> loop,
               string const &label, code_location const &loc)
    : expression(loc),
      Initialization(init),
      Conditional(cond),
      Update(update),
      LoopExpression(loop),
      HasLabel(true),
      Label(label)
{
}

ref_ptr<value> js_for::evaluate(context const &ctx) const
{
    /*
      ref_ptr<list_scope,value> scope = ctx.DeclarationScope->construct();
      scope->unite(ctx.LookupScope);
      context inner_context(scope);
    */
    const context &inner_context = ctx;

    ref_ptr<value> result;
    for (Initialization->evaluate(inner_context); Conditional->evaluate(inner_context)->toBoolean();
         Update->evaluate(inner_context))
    {
        try
        {
            result = LoopExpression->evaluate(inner_context);
        }
        catch (break_exception &be)
        {
            if (!be.HasLabel || (HasLabel && be.HasLabel && be.Label == Label))
                break;
            else
                throw;
        }
        catch (continue_exception &ce)
        {
            if (!ce.HasLabel || (HasLabel && ce.HasLabel && ce.Label == Label))
                continue;
            else
                throw;
        }
    }
    return result;
}

string js_for::toString(int indent) const
{
    string result = getIndent(indent) + "for(" + Initialization->toString() + "; " + Conditional->toString() + "; " +
                    Update->toString() + ")\r\n";

    result += getIndent(indent) + "{\r\n";
    result += LoopExpression->toString(indent + 3);
    result += getIndent(indent) + "}";

    return result;
}

// js_for_in ------------------------------------------------------------------
js_for_in::js_for_in(ref_ptr<expression> iter, ref_ptr<expression> array, ref_ptr<expression> loop,
                     code_location const &loc)
    : expression(loc), Iterator(iter), Array(array), LoopExpression(loop), HasLabel(false)
{
}

js_for_in::js_for_in(ref_ptr<expression> iter, ref_ptr<expression> array, ref_ptr<expression> loop, string const &label,
                     code_location const &loc)
    : expression(loc), Iterator(iter), Array(array), LoopExpression(loop), HasLabel(true), Label(label)
{
}

ref_ptr<value> js_for_in::evaluate(context const &ctx) const
{
    /*
      ref_ptr<list_scope,value> scope = ctx.DeclarationScope->construct();
      scope->unite(ctx.LookupScope);
      context inner_context(scope);
    */
    const context &inner_context = ctx;

    ref_ptr<value> result;
    ref_ptr<value> iterator = Iterator->evaluate(inner_context);
    ref_ptr<value> array = Array->evaluate(inner_context);

    ref_ptr<value> dup = array->duplicateAndResolveDelegate();

    if (dup->size() == 0)
        return ref_ptr<value>(NULL);

    for (ixlib_iterator it = dup->begin(); it != dup->end(); ++it)
    {
        try
        {
            iterator->assign(*it);
            result = LoopExpression->evaluate(inner_context);
        }
        catch (break_exception &be)
        {
            if (!be.HasLabel || (HasLabel && be.HasLabel && be.Label == Label))
                break;
            else
                throw;
        }
        catch (continue_exception &ce)
        {
            if (!ce.HasLabel || (HasLabel && ce.HasLabel && ce.Label == Label))
                continue;
            else
                throw;
        }
    }

    if (result.get())
        return result->duplicate();

    return ref_ptr<value>(NULL);

    // ATTENTION: this is a scope cancellation point.
}

string js_for_in::toString(int indent) const
{
    string result = getIndent(indent) + "for(" + Iterator->toString() + " in " + Array->toString() + ")\r\n";

    result += getIndent(indent) + "{\r\n";
    result += LoopExpression->toString(indent + 3);
    result += getIndent(indent) + "}\r\n";

    return result;
}

// js_return ------------------------------------------------------------------
js_return::js_return(ref_ptr<expression> retval, code_location const &loc) : expression(loc), ReturnValue(retval)
{
}

ref_ptr<value> js_return::evaluate(context const &ctx) const
{
    ref_ptr<value> retval;
    if (ReturnValue.get())
        retval = ReturnValue->evaluate(ctx);

    throw return_exception(retval, getCodeLocation());
}

string js_return::toString(int indent) const
{
    if (ReturnValue.get())
        return getIndent(indent) + "return " + ReturnValue->toString();
    else
        return getIndent(indent) + "return";
}

// js_break -------------------------------------------------------------------
js_break::js_break(code_location const &loc) : expression(loc), HasLabel(false)
{
}

js_break::js_break(string const &label, code_location const &loc) : expression(loc), HasLabel(true), Label(label)
{
}

ref_ptr<value> js_break::evaluate(context const &ctx) const
{
    throw break_exception(HasLabel, Label, getCodeLocation());
}

string js_break::toString(int indent) const
{
    if (HasLabel)
        return getIndent(indent) + "break " + Label;
    else
        return getIndent(indent) + "break";
}

// js_continue ----------------------------------------------------------------
js_continue::js_continue(code_location const &loc) : expression(loc), HasLabel(false)
{
}

js_continue::js_continue(string const &label, code_location const &loc) : expression(loc), HasLabel(true), Label(label)
{
}

ref_ptr<value> js_continue::evaluate(context const &ctx) const
{
    throw continue_exception(HasLabel, Label, getCodeLocation());
}

string js_continue::toString(int indent) const
{
    if (HasLabel)
        return getIndent(indent) + "continue " + Label;
    else
        return getIndent(indent) + "continue";
}

// break_label ----------------------------------------------------------------
break_label::break_label(string const &label, ref_ptr<expression> expr, code_location const &loc)
    : expression(loc), Label(label), Expression(expr)
{
}

ref_ptr<value> break_label::evaluate(context const &ctx) const
{
    try
    {
        return Expression->evaluate(ctx);
    }
    catch (break_exception &be)
    {
        if (be.HasLabel && be.Label == Label)
            return ref_ptr<value>(NULL);
        else
            throw;
    }
}

string break_label::toString(int indent) const
{
    return getIndent(indent) + Label + ":\r\n" + Expression->toString();
}

// js_switch  -----------------------------------------------------------------
js_switch::js_switch(ref_ptr<expression> discriminant, code_location const &loc)
    : expression(loc), HasLabel(false), Discriminant(discriminant)
{
}

js_switch::js_switch(ref_ptr<expression> discriminant, string const &label, code_location const &loc)
    : expression(loc), HasLabel(true), Label(label), Discriminant(discriminant)
{
}

ref_ptr<value> js_switch::evaluate(context const &ctx) const
{
    /*
      ref_ptr<list_scope,value> scope = ctx.DeclarationScope->construct();
      scope->unite(ctx.LookupScope);
      context inner_context(scope);
    */
    const context &inner_context = ctx;

    ref_ptr<value> discr = Discriminant->evaluate(inner_context);

    case_list::const_iterator expr, def;
    bool expr_found = false, def_found = false;

    FOREACH_CONST (first, CaseList, case_list)
    {
        if (first->DiscriminantValue.get())
        {
            if (first->DiscriminantValue->evaluate(inner_context)
                    ->operatorBinary(value::OP_EQUAL, Discriminant->evaluate(inner_context))
                    ->toBoolean())
            {
                expr = first;
                expr_found = true;
                break;
            }
        }
        else
        {
            if (!def_found)
            {
                def = first;
                def_found = true;
            }
        }
    }

    try
    {
        case_list::const_iterator exec, last = CaseList.end();

        if (expr_found)
            exec = expr;
        else if (def_found)
            exec = def;
        else
            return ref_ptr<value>(NULL);

        ref_ptr<value> result;

        while (exec != last)
        {
            result = exec->Expression->evaluate(inner_context);
            exec++;
        }

        if (result.get())
            return result->duplicate();

        return ref_ptr<value>(NULL);
    }
    catch (break_exception &be)
    {
        if (!be.HasLabel || (HasLabel && be.HasLabel && be.Label == Label))
            return ref_ptr<value>(NULL);
        else
            throw;
    }

    // ATTENTION: this is a scope cancellation point.
}

void js_switch::addCase(ref_ptr<expression> dvalue, ref_ptr<expression> expr)
{
    case_label cl;
    cl.DiscriminantValue = dvalue;
    cl.Expression = expr;
    CaseList.push_back(cl);
}

string js_switch::toString(int indent) const
{
    string result = getIndent(indent) + "switch(" + Discriminant->toString() + ")\r\n";

    result += getIndent(indent) + "{\r\n";

    for (size_t i = 0; i < CaseList.size(); ++i)
    {
        result += getIndent(indent) + "case " + CaseList[i].DiscriminantValue->toString() + ":\r\n";
        result += CaseList[i].Expression->toString(indent + 3);
    }

    result += getIndent(indent) + "}";

    return result;
}
