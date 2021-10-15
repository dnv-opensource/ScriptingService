#include <Test/asserter.h>
#include <jsUnits/jsLength.h>

TEST(QuantityFormatTests,LengthFormatting)
{
    std::stringstream a;
    std::string result=a.str();
    MOFA_ASSERT_EQUAL(std::string("1.5"),jsLength(1.5).format(10,std::ios_base::floatfield));
}
