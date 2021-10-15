#pragma once
#include "IValueCreatorService.h"
namespace ixion {namespace javascript {
    class NativeValueCreatorService : public IValueCreatorService
    {
    public:
        ref_ptr<value> MakeUndefined() const override;
        ref_ptr<value> MakeNull() const override;
        ref_ptr<value> MakeConstant(signed long val) const override;
        ref_ptr<value> MakeConstant(signed int val) const override;
        ref_ptr<value> MakeConstant(bool val) const override;
        ref_ptr<value> MakeConstant(unsigned long val) const override;
        ref_ptr<value> MakeConstant(_w64 unsigned int val) const override;
        ref_ptr<value> MakeConstant(unsigned __int64 val) const override;
        ref_ptr<value> MakeConstant(double val) const override;
        ref_ptr<value> MakeConstant(std::string const &val) const override;
        ref_ptr<value> MakeLValue(ref_ptr<value> target) const override;
        ref_ptr<value> WrapConstant(ref_ptr<value> val) const override;
        ref_ptr<value> MakeConditional(ref_ptr<value> conditional, ref_ptr<expression> ifExpression, ref_ptr<expression> ifNotExpression, context const &ctx) const override;
        ref_ptr<value> MakeFunction(const std::string& name, const std::vector<std::string>& ParameterNameList, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType, ref_ptr<expression> Body, context const &ctx) const override;
    };
}}