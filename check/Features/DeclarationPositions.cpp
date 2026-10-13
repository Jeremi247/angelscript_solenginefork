// AS_SOL_DECLARATION_POSITIONS: docs/decisions/0005-a-declaration-reports-where-it-was-made.md

#include "FeatureTest.h"

#include <cstring>

namespace
{
    struct FVector
    {
        float X;
    };

    struct FPosition
    {
        int Result = 0;
        const char* Section = nullptr;
        int Row = 0;
        int Col = 0;

        bool Is(const char* WantedSection, int WantedRow, int WantedCol) const
        {
            return Result == asSUCCESS && Section && std::strcmp(Section, WantedSection) == 0 && Row == WantedRow && Col == WantedCol;
        }
    };

    FPosition TypeAt(asITypeInfo* Type)
    {
        FPosition P;
        P.Result = asSolGetTypeDeclaredAt(Type, &P.Section, &P.Row, &P.Col);
        return P;
    }

    FPosition PropertyAt(asITypeInfo* Type, asUINT Index)
    {
        FPosition P;
        P.Result = asSolGetPropertyDeclaredAt(Type, Index, &P.Section, &P.Row, &P.Col);
        return P;
    }

    FPosition GlobalAt(asIScriptModule* Module, asUINT Index)
    {
        FPosition P;
        P.Result = asSolGetGlobalVarDeclaredAt(Module, Index, &P.Section, &P.Row, &P.Col);
        return P;
    }

    const char* const Script =
        "class Base\n"                    // 1
        "{\n"                             // 2
        "    int Inherited;\n"            // 3
        "}\n"                             // 4
        "class Derived : Base\n"          // 5
        "{\n"                             // 6
        "  float A, B;\n"                 // 7
        "}\n"                             // 8
        "interface IThing {}\n"           // 9
        "enum EColour { Red }\n"          // 10
        "funcdef void FCallback();\n"     // 11
        "typedef float real;\n"           // 12
        "const int Answer = 42;\n"        // 13
        "  Derived@ Held, Other;\n";      // 14
}

FT_TEST(DeclarationPositionsAreReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_DECLARATION_POSITIONS"));
}

FT_TEST(DeclarationPositionsOfTypes)
{
    FeatureTest::FEngine E;
    asIScriptModule* Module = E.Build("Types.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    FT_CHECK(TypeAt(Module->GetTypeInfoByName("Base")).Is("Types.as", 1, 7));
    FT_CHECK(TypeAt(Module->GetTypeInfoByName("Derived")).Is("Types.as", 5, 7));
    FT_CHECK(TypeAt(Module->GetTypeInfoByName("IThing")).Is("Types.as", 9, 11));
    FT_CHECK(TypeAt(Module->GetTypeInfoByName("EColour")).Is("Types.as", 10, 6));
    FT_CHECK(TypeAt(Module->GetTypeInfoByName("FCallback")).Is("Types.as", 11, 14));
    FT_CHECK(TypeAt(Module->GetTypeInfoByName("real")).Is("Types.as", 12, 15));
}

FT_TEST(DeclarationPositionsOfProperties)
{
    FeatureTest::FEngine E;
    asIScriptModule* Module = E.Build("Props.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    asITypeInfo* Derived = Module->GetTypeInfoByName("Derived");
    FT_CHECK(Derived && Derived->GetPropertyCount() == 3);
    if (!Derived || Derived->GetPropertyCount() != 3)
        return;

    // An inherited property reports where the base class declared it.
    FT_CHECK(PropertyAt(Derived, 0).Is("Props.as", 3, 9));
    FT_CHECK(PropertyAt(Derived, 1).Is("Props.as", 7, 9));
    FT_CHECK(PropertyAt(Derived, 2).Is("Props.as", 7, 12));

    FT_CHECK(PropertyAt(Derived, 3).Result == asINVALID_ARG);
    FT_CHECK(PropertyAt(Module->GetTypeInfoByName("EColour"), 0).Result == asINVALID_ARG);
}

FT_TEST(DeclarationPositionsOfGlobals)
{
    FeatureTest::FEngine E;
    asIScriptModule* Module = E.Build("Globals.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    FT_CHECK(Module->GetGlobalVarCount() == 3);
    for (asUINT n = 0; n < Module->GetGlobalVarCount(); ++n)
    {
        const char* Name = nullptr;
        Module->GetGlobalVar(n, &Name);
        FPosition P = GlobalAt(Module, n);
        if (std::strcmp(Name, "Answer") == 0)
            FT_CHECK(P.Is("Globals.as", 13, 11));
        else if (std::strcmp(Name, "Held") == 0)
            FT_CHECK(P.Is("Globals.as", 14, 12));
        else if (std::strcmp(Name, "Other") == 0)
            FT_CHECK(P.Is("Globals.as", 14, 18));
        else
            FT_CHECK(!"unexpected global");
    }

    FT_CHECK(GlobalAt(Module, 3).Result == asINVALID_ARG);
}

FT_TEST(DeclarationPositionsAreNotKeptForRegistered)
{
    FeatureTest::FEngine E;
    E.Engine->RegisterObjectType("FVector", sizeof(FVector), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<FVector>());
    E.Engine->RegisterObjectProperty("FVector", "float X", 0);

    asITypeInfo* Vector = E.Engine->GetTypeInfoByName("FVector");
    FT_CHECK(TypeAt(Vector).Result == asNOT_SUPPORTED);
    FT_CHECK(PropertyAt(Vector, 0).Result == asNOT_SUPPORTED);

    FPosition Missing = TypeAt(nullptr);
    FT_CHECK(Missing.Result == asINVALID_ARG && Missing.Section == nullptr && Missing.Row == 0);
}
