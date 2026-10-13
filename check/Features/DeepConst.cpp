// AS_SOL_DEEP_CONST: docs/decisions/0008-const-reaches-through-a-handle-member.md

#include "FeatureTest.h"

#include <string>

namespace
{
    const char* const Types =
        "class F { int X; void Bump() { X++; } int Peek() const { return X; } }\n"
        "class V { F@ H = F(); }\n"
        "funcdef void FCallback();\n"
        "void Nothing() {}\n"
        "class Holder\n"
        "{\n"
        "    F@ H = F();\n"
        "    V Value;\n"
        "    FCallback@ Callback = Nothing;\n";

    // Builds Types plus the rest of Holder and any globals, and says whether it built.
    bool Builds(FeatureTest::FEngine& E, const char* Members, const char* Globals = "")
    {
        std::string Code = std::string(Types) + Members + "}\n" + Globals;
        return E.Build("DeepConst.as", Code.c_str()) != nullptr;
    }

    // Whether the build was refused for writing through const, rather than for anything else.
    bool RefusedAsConst(FeatureTest::FEngine& E, const char* Members, const char* Globals = "")
    {
        if (Builds(E, Members, Globals))
            return false;
        return E.HasMessage("Reference is read-only") || E.HasMessage("from 'const F@'") || E.HasMessage("'F::Bump() const'");
    }
}

FT_TEST(DeepConstIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_DEEP_CONST"));
}

FT_TEST(DeepConstRefusesWritesThroughThisInConstMethods)
{
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "void Tick() const { H.X = 1; }"));
    }
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "void Tick() const { this.H.X = 1; }"));
    }
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "void Tick() const { H.Bump(); }"));
    }
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "void Tick() const { Value.H.X = 1; }"));
    }
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "void Tick() const { F@ Writable = H; }"));
    }
}

FT_TEST(DeepConstRefusesWritesThroughConstObjects)
{
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "", "const Holder Config;\nvoid Tick() { Config.H.X = 1; }\n"));
    }
    {
        FeatureTest::FEngine E;
        FT_CHECK(RefusedAsConst(E, "", "void Tick(const Holder@ Held) { Held.H.X = 1; }\n"));
    }
}

FT_TEST(DeepConstKeepsReadsAndMutableAccess)
{
    FeatureTest::FEngine E;
    asIScriptModule* Module = E.Build("DeepConst.as",
        (std::string(Types) +
         "    int Read() const { const F@ Readable = H; return H.X + H.Peek() + Value.H.X + Readable.X; }\n"
         "    void Call() const { Callback(); }\n"
         "    void Write() { H.X = 2; Value.H.X = 3; H.Bump(); }\n"
         "}\n"
         // A const handle is not a handle to const: the object stays writable.
         "F@ const Fixed = F();\n"
         "int Run() { Holder h; h.Write(); h.Call(); Fixed.X = 1; return h.Read(); }\n").c_str());
    FT_CHECK(Module);
    if (!Module)
        return;

    asIScriptContext* Context = E.Engine->CreateContext();
    Context->Prepare(Module->GetFunctionByDecl("int Run()"));
    FT_CHECK(Context->Execute() == asEXECUTION_FINISHED);
    FT_CHECK(Context->GetReturnDWord() == 3 + 3 + 3 + 3);
    Context->Release();
}
