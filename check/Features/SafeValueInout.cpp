// AS_SOL_SAFE_VALUE_INOUT: docs/decisions/0010-a-value-type-crosses-as-inout-safely.md

#include "FeatureTest.h"

#include <string>

namespace
{
    struct FPosition
    {
        float X;
        float Y;
    };

    void AppBump(FPosition& P) { P.X += 10.0f; }

    void Register(asIScriptEngine* Engine)
    {
        Engine->RegisterObjectType("FPosition", sizeof(FPosition), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<FPosition>());
        Engine->RegisterObjectProperty("FPosition", "float X", asOFFSET(FPosition, X));
        Engine->RegisterObjectProperty("FPosition", "float Y", asOFFSET(FPosition, Y));
        FT_CHECK(Engine->RegisterGlobalFunction("void AppBump(FPosition &inout)", asFUNCTION(AppBump), asCALL_CDECL) >= 0);
    }

    const char* const Common =
        "class Box { FPosition P; }\n"
        "void Bump(FPosition &inout P) { P.X += 1; }\n"
        "float Read(const FPosition &inout P) { return P.X; }\n"
        "Box@ MakeBox() { return Box(); }\n"
        "FPosition Global;\n";

    // Builds Common plus Code with unsafe references off, runs float Run(), and returns
    // the context's result, keeping the return value in Value.
    int RunScript(FeatureTest::FEngine& E, const char* Code, float& Value)
    {
        Register(E.Engine);
        asIScriptModule* Module = E.Build("Inout.as", (std::string(Common) + Code).c_str());
        if (!Module)
            return asERROR;

        asIScriptContext* Context = E.Engine->CreateContext();
        Context->Prepare(Module->GetFunctionByDecl("float Run()"));
        int Result = Context->Execute();
        if (Result == asEXECUTION_FINISHED)
            Value = Context->GetReturnFloat();
        else if (Result == asEXECUTION_EXCEPTION)
            E.Messages += std::string("exception: ") + Context->GetExceptionString() + "\n";
        Context->Release();
        return Result;
    }
}

FT_TEST(SafeValueInoutIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_SAFE_VALUE_INOUT"));
}

FT_TEST(SafeValueInoutIsDeclaredWithoutUnsafeReferences)
{
    FeatureTest::FEngine E;
    FT_CHECK(E.Engine->GetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES) == 0);
    Register(E.Engine);
    FT_CHECK(E.Build("Inout.as", Common));
}

FT_TEST(SafeValueInoutPassesLocalsAndParametersByAddress)
{
    FeatureTest::FEngine E;
    float Value = 0;
    FT_CHECK(RunScript(E,
        "void Twice(FPosition &inout P) { Bump(P); AppBump(P); }\n"
        "float Run() { FPosition p; p.X = 0; Bump(p); Twice(p); return p.X; }\n", Value) == asEXECUTION_FINISHED);
    FT_CHECK(Value == 12.0f);
}

FT_TEST(SafeValueInoutAssignsPropertiesBack)
{
    FeatureTest::FEngine E;
    float Value = 0;
    FT_CHECK(RunScript(E,
        "float Run()\n"
        "{\n"
        "    Box@ b = Box(); b.P.X = 0;\n"
        "    Global.X = 0;\n"
        "    Bump(b.P); AppBump(b.P); Bump(Global);\n"
        "    return b.P.X + Global.X * 100;\n"
        "}\n", Value) == asEXECUTION_FINISHED);
    FT_CHECK(Value == 111.0f);
}

FT_TEST(SafeValueInoutCopiesAnObjectThatDiesDuringTheCall)
{
    // Under asEP_ALLOW_UNSAFE_REFERENCES this writes to the freed Box. Here the write
    // goes to the copy, and assigning it back finds the handle null.
    FeatureTest::FEngine E;
    float Value = 0;
    FT_CHECK(RunScript(E,
        "Box@ Held;\n"
        "void Kill(FPosition &inout P) { @Held = null; P.X = 1; }\n"
        "float Run() { @Held = Box(); Kill(Held.P); return 0; }\n", Value) == asEXECUTION_EXCEPTION);
    FT_CHECK(E.HasMessage("exception: Null pointer access"));
}

FT_TEST(SafeValueInoutRefusesArgumentsItCannotAssignBack)
{
    {
        FeatureTest::FEngine E;
        float Value = 0;
        FT_CHECK(RunScript(E, "float Run() { Bump(MakeBox().P); return 0; }\n", Value) == asERROR);
        FT_CHECK(E.HasMessage("A value type passed to '&inout' must be a local variable, a parameter or a property"));
    }
    {
        FeatureTest::FEngine E;
        float Value = 0;
        FT_CHECK(RunScript(E, "float Run() { Box@ a = Box(), b = Box(); Bump(true ? a.P : b.P); return 0; }\n", Value) == asERROR);
        FT_CHECK(E.HasMessage("A value type passed to '&inout' must be a local variable, a parameter or a property"));
    }
}

FT_TEST(SafeValueInoutCopiesAnyExpressionForConst)
{
    FeatureTest::FEngine E;
    float Value = -1;
    FT_CHECK(RunScript(E, "float Run() { Box@ b = MakeBox(); b.P.X = 4; return Read(MakeBox().P) + Read(b.P); }\n", Value) == asEXECUTION_FINISHED);
    FT_CHECK(Value == 4.0f);
}

FT_TEST(SafeValueInoutLeavesUnsafeReferencesAsUpstream)
{
    // With the property on, the argument is passed by address as upstream passes it,
    // and an expression upstream accepts is not refused.
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
    float Value = 0;
    FT_CHECK(RunScript(E,
        "void Alias(FPosition &inout P, Box@ B) { P.X = 5; P.Y = B.P.X; }\n"
        "float Run() { Box@ b = Box(); b.P.X = 0; Alias(b.P, b); Bump(MakeBox().P); return b.P.Y; }\n", Value) == asEXECUTION_FINISHED);
    FT_CHECK(Value == 5.0f);
}

FT_TEST(SafeValueInoutCopyIsNotAnAlias)
{
    // The callee writes the copy, so reading the original through another path sees
    // the value from before the call.
    FeatureTest::FEngine E;
    float Value = 0;
    FT_CHECK(RunScript(E,
        "void Alias(FPosition &inout P, Box@ B) { P.X = 5; P.Y = B.P.X; }\n"
        "float Run() { Box@ b = Box(); b.P.X = 0; Alias(b.P, b); return b.P.X * 10 + b.P.Y; }\n", Value) == asEXECUTION_FINISHED);
    FT_CHECK(Value == 50.0f);
}
