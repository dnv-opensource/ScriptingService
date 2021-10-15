#pragma once
#include "ixlib_garbage.hh"
#include <vector>

namespace ixion {namespace javascript {
    class value;
    class expression;
    struct context;
    class IValueCreatorService
    {
    public:
        virtual ~IValueCreatorService() {}
        virtual ref_ptr<value> MakeUndefined() const = 0;
        virtual ref_ptr<value> MakeNull() const = 0;
        virtual ref_ptr<value> MakeConstant(signed long val) const = 0;
        virtual ref_ptr<value> MakeConstant(signed int val) const = 0;
        virtual ref_ptr<value> MakeConstant(bool val) const = 0;
        virtual ref_ptr<value> MakeConstant(unsigned long val) const = 0;
        virtual ref_ptr<value> MakeConstant(_w64 unsigned int val) const = 0;
        virtual ref_ptr<value> MakeConstant(unsigned __int64 val) const = 0;
        virtual ref_ptr<value> MakeConstant(double val) const = 0;
        virtual ref_ptr<value> MakeConstant(std::string const &val) const = 0;
        virtual ref_ptr<value> MakeLValue(ref_ptr<value> target) const = 0;
        virtual ref_ptr<value> WrapConstant(ref_ptr<value> val) const = 0;
        virtual ref_ptr<value> MakeConditional(ref_ptr<value> conditional, ref_ptr<expression> ifExpression, ref_ptr<expression> ifNotExpression, context const &ctx) const = 0;
        virtual ref_ptr<value> MakeFunction(const std::string& name, const std::vector<std::string>& ParameterNameList, const std::vector<std::string>& ParameterTypeList, const std::string& ReturnType, ref_ptr<expression> Body, context const &ctx) const = 0;
    };
}}