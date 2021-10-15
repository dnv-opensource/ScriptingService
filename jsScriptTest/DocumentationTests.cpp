#include <gtest/gtest.h>

#include "Services/ScopedServiceProvider.h"
#include "jsScript/jsScriptable.h"
#include "jsScript/jsClass.h"
#include "jsScript/jsDocumentation.h"
#include "sstream"
#include "Reflection/Classes/Class.h"
#include "ScriptingServiceFactory.h"

using DNVS::MoFa::Reflection::Members::Arg;
using namespace DNVS::MoFa::Reflection::Classes;
using DNVS::MoFa::Reflection::IgnoreAutoReflector;

class DocumentationTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scriptingService = ScriptingServiceFactory().CreateScriptingService();
    }

    DNVS::MoFa::Services::ScopedServiceProvider m_sp;
    DNVS::MoFa::Scripting::ScriptingServicePointer m_scriptingService;
};

class DocumentationTestClassBaseBase : public jsScriptable {
public:
    DocumentationTestClassBaseBase(const jsAutomation& automation) : jsScriptable(automation) {}
    DocumentationTestClassBaseBase() {}

    static void init(jsTypeLibrary& typeLibrary)
    {}
};
class DocumentationTestClassBase : public DocumentationTestClassBaseBase
{
public:
    DocumentationTestClassBase(const jsAutomation& automation) : DocumentationTestClassBaseBase(automation) {}
    DocumentationTestClassBase() {}

    std::string MyBaseTestFn(int a) { return ""; }
    void MyBaseSetter(double) {}
    static void init(jsTypeLibrary& typeLibrary)
    {}
};
class DocumentationTestClass : public DocumentationTestClassBase
{
public:
    DocumentationTestClass(const jsAutomation& automation) : DocumentationTestClassBase(automation) {}
    DocumentationTestClass(int a, double b) {}
    DocumentationTestClass(double a, const jsValue::Params& params) {}
    static DocumentationTestClass* CreateClass(std::string t) { return nullptr; }

    void Test(int a, double b) {}
    int Test2(double x) { return 42; }
    void SetX(double v) {}
    double GetX() const { return 1; }
    void SetY(double v) {}
    double GetZ() const { return 2; }
};

DNVS::MoFa::Doc::MemberPointer GetMember(jsDocumentation& doc, jsTypeInfo& info)
{
    return doc.GetTypeMap().CreateFromFunctionTypeInfo(&info);
}

DNVS::MoFa::Doc::TypePointer GetType(jsDocumentation& doc, jsValue* prototype)
{
    return doc.GetTypeMap().GetOrCreateFromPrototype(prototype);
}

DNVS::MoFa::Doc::TypePointer GetType(jsDocumentation& doc, const std::type_index& typeIndex)
{
    return doc.GetTypeMap().GetOrCreateFromTypeIndex(typeIndex);
}

TEST_F(DocumentationTests, TestAddTitle)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs,"DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream ss;
    doc.addTitle(ss, GetType(doc,prototype));
    EXPECT_EQ("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" http://www.w3.org/TR/html4/loose.dtd>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n<title>  : DocumentationTestClass\n</title>\n<link rel =\"stylesheet\" type=\"text/css\" href=\"jsDocumentation.css\" media=\"screen\">\n<link rel =\"stylesheet\" type=\"text/css\" href=\"print.css\" media=\"print\">\n<script type=\"text/javascript\" src=\"js/jquery-1.4.2.min.js\"></script> \n<script type=\"text/javascript\" src=\"js/genie.table.filter.min.js\"></script>\n<script type=\"text/javascript\" src=\"js/filterscript.js\"></script>\n</head>\n", ss.str());
}

TEST_F(DocumentationTests, TestAddTitleReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClass> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");

    jsDocumentation doc(typeLibraryJs);
    std::stringstream ss;
    doc.addTitle(ss, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" http://www.w3.org/TR/html4/loose.dtd>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n<title>  : DocumentationTestClass\n</title>\n<link rel =\"stylesheet\" type=\"text/css\" href=\"jsDocumentation.css\" media=\"screen\">\n<link rel =\"stylesheet\" type=\"text/css\" href=\"print.css\" media=\"print\">\n<script type=\"text/javascript\" src=\"js/jquery-1.4.2.min.js\"></script> \n<script type=\"text/javascript\" src=\"js/genie.table.filter.min.js\"></script>\n<script type=\"text/javascript\" src=\"js/filterscript.js\"></script>\n</head>\n", ss.str());
}


TEST_F(DocumentationTests, TestMakeHRefFromNameAndPrototype)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    EXPECT_EQ("<a href=\"abc.html\">abc</a>", doc.makeHREF("abc", GetType(doc, prototype)));
    EXPECT_EQ("DocumentationTestClass", doc.makeHREF("DocumentationTestClass", GetType(doc, prototype)));
}

TEST_F(DocumentationTests, TestMakeHRefFromFunctionAndPrototype)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    jsTypeInfo& info = cls.Function("Test", &DocumentationTestClass::Test);
    info.AddSignature((Arg("a"), Arg("b", "with doc")));
    info.AddDocumentation("Bye world");

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    EXPECT_EQ("<a href=\"DocumentationTestClass.html#Test(int, double)\">Test</a>", doc.makeHREF(GetMember(doc, info), GetType(doc, prototype)));
    EXPECT_EQ("<a name=\"Test(int, double)\"></a>", doc.makeHREF(GetMember(doc, info), GetType(doc, prototype),false, false));
}

TEST_F(DocumentationTests, TestFunctionArgument)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    jsTypeInfo& info = cls.Function("Test", &DocumentationTestClass::Test);
    info.AddSignature((Arg("a"), Arg("b", "with doc")));
    info.AddDocumentation("Bye world");
    EXPECT_EQ("a", info.argument(0)->varName());
    EXPECT_EQ("b", info.argument(1)->varName());

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    EXPECT_EQ("<b>int</b> a", doc.argument(GetType(doc,prototype), GetMember(doc, info), 0, false));
    EXPECT_EQ("int", doc.argument(GetType(doc, prototype), GetMember(doc, info), 0, true));
    EXPECT_EQ("<b>double</b><span title=\"with doc\"> b</span>", doc.argument(GetType(doc, prototype), GetMember(doc, info), 1, false));
    EXPECT_EQ("double", doc.argument(GetType(doc, prototype), GetMember(doc, info), 1, true));
}

TEST_F(DocumentationTests, TestaddFunctionDetail)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    jsTypeInfo& info = cls.Function("Test", &DocumentationTestClass::Test);
    info.AddSignature((Arg("a"), Arg("b", "with doc")));
    info.AddDocumentation("Bye world");
    jsTypeInfo& info2 = cls.Function("Test2", &DocumentationTestClass::Test2);
    jsExampleInfo& example = info2.AddExample("Hello world");
    example.AddComment("//addd");
    example.AddScript("a = b;");

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addFunctionDetail(sstream, GetType(doc, prototype));
    EXPECT_EQ("<!-- ========= FUNCTION DETAIL ======== -->\n\n<a name=\"function_detail\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=1>\nFunction Detail</td>\n</tr>\n</table>\n<a name=\"Test(int, double)\"></a>\n<h3>Test</h3>\n<p class=\"function\">\n<b>&nbsp;</b> <b>Test</b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>Bye world\n</dl>\n<dl><dt>Parameters:</dt>\n<dd><code>b</code> - with doc</dl>\n<hr>\n\n<a name=\"Test2(double)\"></a>\n<h3>Test2</h3>\n<p class=\"function\">\n<b>int</b> <b>Test2</b>(<b>double</b>)</p>\n<p>\n<dt>Example of use: </dt></dl><table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\"><tr class=\"header\">  <td colspan=2>Hello world</td></tr><tr>  <td>     <code>a = b;<br/>     </code>\t\t<div class=\"explanation\">//addd<br/>\t\t</div>   </td></tr></table><hr>\n\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddFunctionDetailReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClass> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.Function("Test", &DocumentationTestClass::Test)
        .AddSignature(Arg("a"), Arg("b", "with doc"))
        .AddDocumentation("Bye world");
    cls.Function("Test2", &DocumentationTestClass::Test2);

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addFunctionDetail(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("<!-- ========= FUNCTION DETAIL ======== -->\n\n<a name=\"function_detail\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=1>\nFunction Detail</td>\n</tr>\n</table>\n<a name=\"Test(int, double)\"></a>\n<h3>Test</h3>\n<p class=\"function\">\n<b>&nbsp;</b> <b>Test</b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>Bye world\n</dl>\n<dl><dt>Parameters:</dt>\n<dd><code>b</code> - with doc</dl>\n<hr>\n\n<a name=\"Test2(double)\"></a>\n<h3>Test2</h3>\n<p class=\"function\">\n<b>int</b> <b>Test2</b>(<b>double</b>)</p>\n<p>\n<hr>\n\n", sstream.str());
}


TEST_F(DocumentationTests, TestaddFunctionSummary)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClassBase> clsb(typeLibraryJs, "DocumentationTestClassBase");
    clsb.Function("MyBaseTestFn", &DocumentationTestClassBase::MyBaseTestFn);
    jsClass<DocumentationTestClass, DocumentationTestClassBase> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    jsTypeInfo& info = cls.Function("Test", &DocumentationTestClass::Test);
    info.AddSignature((Arg("a"), Arg("b", "with doc")));
    info.AddDocumentation("Bye world");
    jsTypeInfo& info2 = cls.Function("Test2", &DocumentationTestClass::Test2);
    jsExampleInfo& example = info2.AddExample("Hello world");
    example.AddComment("//addd");
    example.AddScript("a = b;");

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addFunctionSummary(sstream, GetType(doc,prototype));
    EXPECT_EQ("&nbsp;\n<!-- ========= FUNCTION SUMMARY ======== -->\n\n<a name=\"function_summary\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=2>\nFunction Summary</td>\n</tr>\n<tr \nclass=\"inherited\">\n<td class=\"returntype\" align=\"right\" valign=\"top\" width=\"1%\"><code>&nbsp;<b>string</b></code>\n</td>\n<td class=\"functionsummary\"><code><b><a href=\"DocumentationTestClassBase.html#MyBaseTestFn(int)\">MyBaseTestFn</a></b>(<b>int</b>)</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td class=\"returntype\" align=\"right\" valign=\"top\" width=\"1%\"><code>&nbsp;<b>&nbsp;</b></code>\n</td>\n<td class=\"functionsummary\"><code><b><a href=\"DocumentationTestClass.html#Test(int, double)\">Test</a></b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</code>\n<div class=\"explanation\">\nBye world</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td class=\"returntype\" align=\"right\" valign=\"top\" width=\"1%\"><code>&nbsp;<b>int</b></code>\n</td>\n<td class=\"functionsummary\"><code><b><a href=\"DocumentationTestClass.html#Test2(double)\">Test2</a></b>(<b>double</b>)</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n</table>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddFunctionSummaryReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClassBase> clsb(typeLibrary, "DocumentationTestClassBase");
    clsb.Function("MyBaseTestFn", &DocumentationTestClassBase::MyBaseTestFn);
    
    Class<DocumentationTestClass, Public<DocumentationTestClassBase>, IgnoreAutoReflector> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.Function("Test", &DocumentationTestClass::Test)
        .AddSignature(Arg("a"), Arg("b", "with doc"))
        .AddDocumentation("Bye world");
    cls.Function("Test2", &DocumentationTestClass::Test2);

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addFunctionSummary(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("&nbsp;\n<!-- ========= FUNCTION SUMMARY ======== -->\n\n<a name=\"function_summary\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=2>\nFunction Summary</td>\n</tr>\n<tr \nclass=\"inherited\">\n<td class=\"returntype\" align=\"right\" valign=\"top\" width=\"1%\"><code>&nbsp;<b>string</b></code>\n</td>\n<td class=\"functionsummary\"><code><b><a href=\"DocumentationTestClassBase.html#MyBaseTestFn(int)\">MyBaseTestFn</a></b>(<b>int</b>)</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td class=\"returntype\" align=\"right\" valign=\"top\" width=\"1%\"><code>&nbsp;<b>&nbsp;</b></code>\n</td>\n<td class=\"functionsummary\"><code><b><a href=\"DocumentationTestClass.html#Test(int, double)\">Test</a></b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</code>\n<div class=\"explanation\">\nBye world</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td class=\"returntype\" align=\"right\" valign=\"top\" width=\"1%\"><code>&nbsp;<b>int</b></code>\n</td>\n<td class=\"functionsummary\"><code><b><a href=\"DocumentationTestClass.html#Test2(double)\">Test2</a></b>(<b>double</b>)</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n</table>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddConstructorDetail)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.Constructor<int, double>("XXZ")
        .AddSignature((Arg("a"), Arg("b", "with doc")))
        .AddDocumentation("Hswr");
    auto& info = cls.Constructor<double, const jsValue::Params&>();
    info.AddSignature((Arg("a")));
    info.argument(1)->document("jalla", "doc jalla", 0);
    info.argument(1)->document("new jalla", "doc new jalla", 1);

    cls.Constructor("MyConstructor", &DocumentationTestClass::CreateClass);

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addConstructorDetail(sstream, GetType(doc, prototype));
    EXPECT_EQ("<!-- ========= CONSTRUCTOR DETAIL ======== -->\n\n<a name=\"constructor_detail\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=1>\nConstructor Detail</td>\n</tr>\n</table>\n<a name=\"DocumentationTestClass(double, ...)\"></a>\n<h3>DocumentationTestClass</h3>\n<p class=\"function\">\n<b>DocumentationTestClass</b>(<b>double</b> a, [jalla ..., new jalla ...])</p>\n<p>\n<dl><dt>Parameters:</dt>\n<dd><code>jalla</code> - doc jalla<dd><code>new jalla</code> - doc new jalla</dl>\n<hr>\n\n<a name=\"MyConstructor(string)\"></a>\n<h3>MyConstructor</h3>\n<p class=\"function\">\n<b>MyConstructor</b>(<b>string</b>)</p>\n<p>\n<hr>\n\n<a name=\"XXZ(int, double)\"></a>\n<h3>XXZ</h3>\n<p class=\"function\">\n<b>XXZ</b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>Hswr\n</dl>\n<dl><dt>Parameters:</dt>\n<dd><code>b</code> - with doc</dl>\n<hr>\n\n", sstream.str());
              
}

TEST_F(DocumentationTests, TestaddConstructorDetailReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClass> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.NamedConstructor<int, double>("XXZ")
        .AddSignature(Arg("a"), Arg("b", "with doc"))
        .AddDocumentation("Hswr");

    cls.NamedConstructor("MyConstructor", &DocumentationTestClass::CreateClass);

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addConstructorDetail(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("<!-- ========= CONSTRUCTOR DETAIL ======== -->\n\n<a name=\"constructor_detail\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=1>\nConstructor Detail</td>\n</tr>\n</table>\n<a name=\"MyConstructor(string)\"></a>\n<h3>MyConstructor</h3>\n<p class=\"function\">\n<b>MyConstructor</b>(<b>string</b>)</p>\n<p>\n<hr>\n\n<a name=\"XXZ(int, double)\"></a>\n<h3>XXZ</h3>\n<p class=\"function\">\n<b>XXZ</b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>Hswr\n</dl>\n<dl><dt>Parameters:</dt>\n<dd><code>b</code> - with doc</dl>\n<hr>\n\n", sstream.str());

}
TEST_F(DocumentationTests, TestaddConstructorSummary)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.Constructor<int, double>("XXZ")
        .AddSignature((Arg("a"), Arg("b", "with doc")))
        .AddDocumentation("Hswr");
    auto& info = cls.Constructor<double, const jsValue::Params&>();
    info.AddSignature((Arg("a")));
    info.argument(1)->document("jalla", "doc jalla", 0);
    info.argument(1)->document("new jalla", "doc new jalla", 1);

    cls.Constructor("MyConstructor", &DocumentationTestClass::CreateClass);

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addConstructorSummary(sstream, GetType(doc, prototype));
    EXPECT_EQ("&nbsp;\n<!-- ========= CONSTRUCTOR SUMMARY ======== -->\n\n<a name=\"constructor_summary\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=2>\nConstructor Summary</td>\n</tr>\n<tr >\n<td><code><b><a href=\"DocumentationTestClass.html#DocumentationTestClass(double, ...)\">DocumentationTestClass</a></b>(<b>double</b> a, [jalla ..., new jalla ...])</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr >\n<td><code><b><a href=\"DocumentationTestClass.html#MyConstructor(string)\">MyConstructor</a></b>(<b>string</b>)</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr >\n<td><code><b><a href=\"DocumentationTestClass.html#XXZ(int, double)\">XXZ</a></b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</code>\n<div class=\"explanation\">\nHswr</div></td>\n</tr>\n</table>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddConstructorSummaryReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClass> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.NamedConstructor<int, double>("XXZ")
        .AddSignature("a", Arg("b", "with doc"))
        .AddDocumentation("Hswr");

    cls.NamedConstructor("MyConstructor", &DocumentationTestClass::CreateClass);
    
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addConstructorSummary(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("&nbsp;\n<!-- ========= CONSTRUCTOR SUMMARY ======== -->\n\n<a name=\"constructor_summary\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=2>\nConstructor Summary</td>\n</tr>\n<tr >\n<td><code><b><a href=\"DocumentationTestClass.html#MyConstructor(string)\">MyConstructor</a></b>(<b>string</b>)</code>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr >\n<td><code><b><a href=\"DocumentationTestClass.html#XXZ(int, double)\">XXZ</a></b>(<b>int</b> a, <b>double</b><span title=\"with doc\"> b</span>)</code>\n<div class=\"explanation\">\nHswr</div></td>\n</tr>\n</table>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddPropertiesDetail)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClass> cls(typeLibraryJs, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.Get("X", &DocumentationTestClass::GetX).AddDocumentation("grunken");
    cls.Set("X", &DocumentationTestClass::SetX);
    cls.Set("Y", &DocumentationTestClass::SetY).AddDocumentation("Hello world");
    cls.Get("Z", &DocumentationTestClass::GetZ);

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addPropertyDetail(sstream, GetType(doc, prototype));
    EXPECT_EQ("<!-- ========= PROPERTY DETAIL ======== -->\n\n<a name=\"property_detail\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=1>\nProperty Detail</td>\n</tr>\n</table>\n<a name=\"X()\"></a>\n<h3>X</h3>\n<p class=\"function\">\n<b>double</b> <b>X</b>()</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>grunken\n</dl>\n<a name=\"Y()\"></a>\n<h3>Y</h3>\n<p class=\"function\">\n<b>double</b> <b>Y</b>()</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>Hello world\n</dl>\n<a name=\"Z()\"></a>\n<h3>Z</h3>\n<p class=\"function\">\n<b>double</b> <b>Z</b>()</p>\n<p>\n", sstream.str());

}

TEST_F(DocumentationTests, TestaddPropertiesDetailReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClass> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");
    cls.Get("X", &DocumentationTestClass::GetX).AddDocumentation("grunken");
    cls.Set("X", &DocumentationTestClass::SetX);
    cls.Set("Y", &DocumentationTestClass::SetY).AddDocumentation("Hello world");
    cls.Get("Z", &DocumentationTestClass::GetZ);

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addPropertyDetail(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("<!-- ========= PROPERTY DETAIL ======== -->\n\n<a name=\"property_detail\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=1>\nProperty Detail</td>\n</tr>\n</table>\n<a name=\"X()\"></a>\n<h3>X</h3>\n<p class=\"function\">\n<b>double</b> <b>X</b>()</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>grunken\n</dl>\n<a name=\"Y()\"></a>\n<h3>Y</h3>\n<p class=\"function\">\n<b>double</b> <b>Y</b>()</p>\n<p>\n<dl>\n<dt>Description:</dt>\n<dd>Hello world\n</dl>\n<a name=\"Z()\"></a>\n<h3>Z</h3>\n<p class=\"function\">\n<b>double</b> <b>Z</b>()</p>\n<p>\n", sstream.str());

}

TEST_F(DocumentationTests, TestaddPropertiesSummary)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClassBase> clsb(typeLibraryJs, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    jsClass<DocumentationTestClass, DocumentationTestClassBase> cls(typeLibraryJs, "DocumentationTestClass");

    cls.AddDocumentation("Hello world!!");
    cls.Get("X", &DocumentationTestClass::GetX).AddDocumentation("grunken");
    cls.Set("X", &DocumentationTestClass::SetX);
    cls.Set("Y", &DocumentationTestClass::SetY).AddDocumentation("Hello world");
    cls.Get("Z", &DocumentationTestClass::GetZ);

    jsValue* prototype = typeLibraryJs.lookup("DocumentationTestClass");
    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addPropertySummary(sstream, GetType(doc, prototype));
    EXPECT_EQ("&nbsp;\n<!-- ========= PROPERTY SUMMARY ======== -->\n\n<a name=\"property_summary\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=2>\nProperty Summary</td>\n</tr>\n<tr \nclass=\"inherited\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClassBase.html#MyBaseSetter()\">MyBaseSetter</a></b></code> <span class=\"propertyStatus\"> write only</span>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClass.html#X()\">X</a></b></code> <span class=\"propertyStatus\"> read/write</span>\n<div class=\"explanation\">\ngrunken</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClass.html#Y()\">Y</a></b></code> <span class=\"propertyStatus\"> write only</span>\n<div class=\"explanation\">\nHello world</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClass.html#Z()\">Z</a></b></code> <span class=\"propertyStatus\"> read only</span>\n<div class=\"explanation\">\n</div></td>\n</tr>\n</table>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddPropertiesSummaryReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClassBase> clsb(typeLibrary, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    Class<DocumentationTestClass, Public<DocumentationTestClassBase>, IgnoreAutoReflector> cls(typeLibrary, "DocumentationTestClass");

    cls.AddDocumentation("Hello world!!");
    cls.Get("X", &DocumentationTestClass::GetX).AddDocumentation("grunken");
    cls.Set("X", &DocumentationTestClass::SetX);
    cls.Set("Y", &DocumentationTestClass::SetY).AddDocumentation("Hello world");
    cls.Get("Z", &DocumentationTestClass::GetZ);

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addPropertySummary(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("&nbsp;\n<!-- ========= PROPERTY SUMMARY ======== -->\n\n<a name=\"property_summary\"><!-- --></a>\n<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\">\n<tr class=\"header\">\n<td colspan=2>\nProperty Summary</td>\n</tr>\n<tr \nclass=\"inherited\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClassBase.html#MyBaseSetter()\">MyBaseSetter</a></b></code> <span class=\"propertyStatus\"> write only</span>\n<div class=\"explanation\">\n</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClass.html#X()\">X</a></b></code> <span class=\"propertyStatus\"> read/write</span>\n<div class=\"explanation\">\ngrunken</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClass.html#Y()\">Y</a></b></code> <span class=\"propertyStatus\"> write only</span>\n<div class=\"explanation\">\nHello world</div></td>\n</tr>\n<tr \nclass=\"own\">\n<td align=\"right\" valign=\"top\" width=\"1%\">\n<code>&nbsp;<b>double</b></code>\n</td>\n<td width=\"90%\"><code><b><a href=\"DocumentationTestClass.html#Z()\">Z</a></b></code> <span class=\"propertyStatus\"> read only</span>\n<div class=\"explanation\">\n</div></td>\n</tr>\n</table>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddClassChildren)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClassBase> clsb(typeLibraryJs, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    jsClass<DocumentationTestClass, DocumentationTestClassBase> cls(typeLibraryJs, "DocumentationTestClass");

    cls.AddDocumentation("Hello world!!");

    jsValue* prototypeBase = typeLibraryJs.lookup("DocumentationTestClassBase");
    jsValue* prototypeDerived = typeLibraryJs.lookup("DocumentationTestClass");

    jsDocumentation doc(typeLibraryJs);
    doc.addType(prototypeBase);
    doc.addType(prototypeDerived);
    std::stringstream sstream;
    doc.addClassChildren(sstream, GetType(doc, prototypeBase));
    EXPECT_EQ("<dl>\n<dt>Direct Known Subclasses:</dt> <dd><b><a href=\"DocumentationTestClass.html\">DocumentationTestClass</a></b></dd>\n</dl>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddClassChildrenReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    using namespace DNVS::MoFa::Reflection::Classes;
    using DNVS::MoFa::Reflection::IgnoreAutoReflector;
    Class<DocumentationTestClassBase, IgnoreAutoReflector> clsb(typeLibrary, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    Class<DocumentationTestClass, Public<DocumentationTestClassBase>, IgnoreAutoReflector> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addClassChildren(sstream, GetType(doc, typeid(DocumentationTestClassBase)));
    EXPECT_EQ("<dl>\n<dt>Direct Known Subclasses:</dt> <dd><b><a href=\"DocumentationTestClass.html\">DocumentationTestClass</a></b></dd>\n</dl>\n", sstream.str());
}
TEST_F(DocumentationTests, TestaddClassHierarchy)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClassBaseBase> clsbb(typeLibraryJs, "DocumentationTestClassBaseBase");
    jsClass<DocumentationTestClassBase, DocumentationTestClassBaseBase> clsb(typeLibraryJs, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    jsClass<DocumentationTestClass, DocumentationTestClassBase> cls(typeLibraryJs, "DocumentationTestClass");

    cls.AddDocumentation("Hello world!!");

    jsValue* prototypeBase = typeLibraryJs.lookup("DocumentationTestClassBase");
    jsValue* prototypeDerived = typeLibraryJs.lookup("DocumentationTestClass");

    jsDocumentation doc(typeLibraryJs);
    doc.addType(prototypeBase);
    doc.addType(prototypeDerived);
    std::stringstream sstream;
    doc.addClassHierarchy(sstream, GetType(doc, prototypeDerived));
    EXPECT_EQ("<br/>\n<dl>\n<dt><h1>Details</h1></dt>\n<dl>\n<dt>Class hierarchy:</dt><dd>\n</dl>\n<pre>\n<a href=\"DocumentationTestClassBaseBase.html\">DocumentationTestClassBaseBase</a>\n  |\n  +--<a href=\"DocumentationTestClassBase.html\">DocumentationTestClassBase</a>\n       |\n       +--<b>DocumentationTestClass</b>\n</pre>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddClassHierarchyReflection)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    auto typeLibrary = jsStack::stack()->GetTypeLibrary();
    Class<DocumentationTestClassBaseBase, IgnoreAutoReflector> clsbb(typeLibrary, "DocumentationTestClassBaseBase");    
    Class<DocumentationTestClassBase, Public<DocumentationTestClassBaseBase>, IgnoreAutoReflector> clsb(typeLibrary, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    Class<DocumentationTestClass, Public<DocumentationTestClassBase>, IgnoreAutoReflector> cls(typeLibrary, "DocumentationTestClass");
    cls.AddDocumentation("Hello world!!");

    jsDocumentation doc(typeLibraryJs);
    std::stringstream sstream;
    doc.addClassHierarchy(sstream, GetType(doc, typeid(DocumentationTestClass)));
    EXPECT_EQ("<br/>\n<dl>\n<dt><h1>Details</h1></dt>\n<dl>\n<dt>Class hierarchy:</dt><dd>\n</dl>\n<pre>\n<a href=\"DocumentationTestClassBaseBase.html\">DocumentationTestClassBaseBase</a>\n  |\n  +--<a href=\"DocumentationTestClassBase.html\">DocumentationTestClassBase</a>\n       |\n       +--<b>DocumentationTestClass</b>\n</pre>\n", sstream.str());
}

TEST_F(DocumentationTests, TestaddClassData)
{
    jsTypeLibrary& typeLibraryJs = jsStack::stack()->GetJsTypeLibrary();
    jsClass<DocumentationTestClassBaseBase> clsbb(typeLibraryJs, "DocumentationTestClassBaseBase");
    jsClass<DocumentationTestClassBase, DocumentationTestClassBaseBase> clsb(typeLibraryJs, "DocumentationTestClassBase");
    clsb.Set("MyBaseSetter", &DocumentationTestClassBase::MyBaseSetter);
    clsb.AddDocumentation("Hello world!!");
    clsb.AddDocumentation("More detailed doc");
    clsb.AddExample("My example").AddScript("c = f;");
    jsClass<DocumentationTestClass, DocumentationTestClassBase> cls(typeLibraryJs, "DocumentationTestClass");
    
    jsValue* prototypeBase = typeLibraryJs.lookup("DocumentationTestClassBase");
    jsValue* prototypeDerived = typeLibraryJs.lookup("DocumentationTestClass");

    jsDocumentation doc(typeLibraryJs);
    doc.addType(prototypeBase);
    doc.addType(prototypeDerived);
    std::stringstream sstream;
    doc.addClassData(sstream, GetType(doc, prototypeBase));
    EXPECT_EQ("<!-- ======== START OF CLASS DATA ======== -->\n<dd>\n<dl><dt>Description:</dt>\nHello world!!More detailed doc\n</dl>\n<dl>\n<dt>Example of use: </dt></dl><table border=\"1\" cellpadding=\"3\" cellspacing=\"0\" width=\"100%\"><tr class=\"header\">  <td colspan=2>My example</td></tr><tr>  <td>     <code>c = f;<br/>     </code>\t\t<div class=\"explanation\">\t\t</div>   </td></tr></table><br/>\n<dl>\n<dt><h1>Details</h1></dt>\n<dl>\n<dt>Class hierarchy:</dt><dd>\n</dl>\n<pre>\n<a href=\"DocumentationTestClassBaseBase.html\">DocumentationTestClassBaseBase</a>\n  |\n  +--<b>DocumentationTestClassBase</b>\n</pre>\n<dl>\n<dt>Direct Known Subclasses:</dt> <dd><b><a href=\"DocumentationTestClass.html\">DocumentationTestClass</a></b></dd>\n</dl>\n<hr/>\n", sstream.str());
}