// AS_SOL_DOUBLE_NARROWING_WARNS: docs/decisions/0013-a-double-narrows-to-a-float-only-when-asked.md

#include "FeatureTest.h"

#include <string>

namespace
{
    const char* const Warning = "Double value narrowed in implicit conversion to float";

    // Builds Code and says whether it built and warned that a double narrowed.
    bool Warns(const char* Code)
    {
        FeatureTest::FEngine E;
        return E.Build("DoubleNarrowing.as", Code) != nullptr && E.HasMessage(Warning);
    }

    // Builds Code and says whether it built with no warning of any kind.
    bool BuildsQuietly(const char* Code)
    {
        FeatureTest::FEngine E;
        return E.Build("DoubleNarrowing.as", Code) != nullptr && E.Messages.empty();
    }
}

FT_TEST(DoubleNarrowingWarnsIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_DOUBLE_NARROWING_WARNS"));
}

FT_TEST(DoubleNarrowingWarnsOnEveryImplicitNarrowing)
{
    // An initialisation, an assignment, a compound assignment, an argument and a return.
    FT_CHECK(Warns("void Run(double d) { float f = d; }\n"));
    FT_CHECK(Warns("void Run(double d) { float f; f = d; }\n"));
    FT_CHECK(Warns("void Run(double d) { float f = 1; f += d; }\n"));
    FT_CHECK(Warns("void Take(float) {}\nvoid Run(double d) { Take(d); }\n"));
    FT_CHECK(Warns("float Run(double d) { return d; }\n"));

    // A mixed expression is a double, so storing it narrows.
    FT_CHECK(Warns("void Run(double d) { float f = 2.0f; float g = f * d; }\n"));
}

FT_TEST(DoubleNarrowingWarnsOncePerConversion)
{
    FeatureTest::FEngine E;
    FT_CHECK(E.Build("DoubleNarrowing.as", "void Run(double d) { float f = d; }\n") != nullptr);

    const size_t First = E.Messages.find(Warning);
    FT_CHECK(First != std::string::npos);
    FT_CHECK(E.Messages.find(Warning, First + 1) == std::string::npos);
}

FT_TEST(DoubleNarrowingWarnsOnAnInexactConstant)
{
    // As upstream warns on an integer constant a float cannot hold, and not as a variable.
    for (const char* Code : { "void Run() { float f = 0.1; }\n", "void Run() { float f = 1.0e11; }\n" })
    {
        FeatureTest::FEngine E;
        FT_CHECK(E.Build("DoubleNarrowing.as", Code) != nullptr);
        FT_CHECK(E.HasMessage("Implicit conversion of value is not exact"));
        FT_CHECK(!E.HasMessage(Warning));
    }
}

FT_TEST(DoubleNarrowingKeepsWhatIsAskedOrExact)
{
    // Explicit, widening, and a constant a float holds exactly.
    FT_CHECK(BuildsQuietly("void Run(double d) { float f = float(d); }\n"));
    FT_CHECK(BuildsQuietly("void Run() { float f = float(0.1); }\n"));
    FT_CHECK(BuildsQuietly("void Run(float f) { double d = f; d += f; }\n"));
    FT_CHECK(BuildsQuietly("void Run() { float f = 0.5; f = 1.0e10; }\n"));
}

FT_TEST(DoubleNarrowingConvertsAsBefore)
{
    FeatureTest::FEngine E;
    asIScriptModule* Module = E.Build("DoubleNarrowing.as",
        "double Wide = 1.5;\n"
        "float Narrowed;\n"
        "void Run() { Narrowed = Wide; }\n");
    FT_CHECK(Module);
    if (!Module)
        return;

    FT_CHECK(E.Run(Module, "void Run()") == asEXECUTION_FINISHED);
    const float* Narrowed = static_cast<const float*>(Module->GetAddressOfGlobalVar(Module->GetGlobalVarIndexByName("Narrowed")));
    FT_CHECK(Narrowed && *Narrowed == 1.5f);
}
