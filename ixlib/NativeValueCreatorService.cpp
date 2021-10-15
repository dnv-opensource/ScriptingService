#include "NativeValueCreatorService.h"
#include "ixlib_javascript.hh"
#include "ixlib_js_internals.hh"

namespace ixion {namespace javascript {

    ref_ptr<value> NativeValueCreatorService::MakeUndefined() const
    {
        return makeUndefined();
    }

    ref_ptr<value> NativeValueCreatorService::MakeNull() const
    {
        return makeNull();
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(signed long val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(signed int val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(unsigned long val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(_w64 unsigned int val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(bool val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(unsigned __int64 val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(double val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeConstant(std::string const &val) const
    {
        return makeConstant(val);
    }

    ref_ptr<value> NativeValueCreatorService::MakeLValue(ref_ptr<value> target) const
    {
        return makeLValue(target);
    }


    ref_ptr<value> NativeValueCreatorService::WrapConstant(ref_ptr<value> val) const
    {
        return wrapConstant(val);
    }    

    ixion::ref_ptr<value> NativeValueCreatorService::MakeConditional(ref_ptr<value> conditional, ref_ptr<expression> ifExpression, ref_ptr<expression> ifNotExpression, context const &ctx) const
    {
        throw;
    }

    ixion::ref_ptr<value> NativeValueCreatorService::MakeFunction(const std::string& name, const std::vector<std::string>& ParameterNameList, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType, ref_ptr<expression> Body, context const &ctx) const
    {
        return new function(std::make_shared<NativeValueCreatorService>(), ParameterNameList, Body, ctx);
    }

}}

