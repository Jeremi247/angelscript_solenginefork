// AS_SOL_NO_SCRIPT_STATE: docs/decisions/0009-a-module-can-be-told-to-hold-no-state.md

#include "FeatureTest.h"

#include <new>

namespace
{
    struct FVector
    {
        float X;
    };

    struct FOwner
    {
        int* Data;
    };

    void ConstructOwner(void* Memory) { new (Memory) FOwner{ nullptr }; }
    void DestructOwner(void*) {}

    void Register(asIScriptEngine* Engine)
    {
        Engine->RegisterObjectType("FVector", sizeof(FVector), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<FVector>());
        Engine->RegisterObjectProperty("FVector", "float X", 0);

        Engine->RegisterObjectType("FOwner", sizeof(FOwner), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
        Engine->RegisterObjectBehaviour("FOwner", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructOwner), asCALL_CDECL_OBJLAST);
        Engine->RegisterObjectBehaviour("FOwner", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructOwner), asCALL_CDECL_OBJLAST);
    }

    const char* const Systems =
        "interface IGameplaySystem {}\n"                         // 1
        "namespace Game { interface IRenderSystem {} }\n"        // 2
        "interface ISpecialSystem : IGameplaySystem {}\n"        // 3
        "class Base\n"                                           // 4
        "{\n"                                                    // 5
        "    int Inherited;\n"                                   // 6
        "}\n";                                                   // 7
}

FT_TEST(NoScriptStateIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_NO_SCRIPT_STATE"));
}

FT_TEST(NoScriptStateRefusesMembersOfASystem)
{
    FeatureTest::FEngine E;
    FT_CHECK(asSolAddStatelessInterface(E.Engine, "IGameplaySystem") == asSUCCESS);

    FT_CHECK(!E.Build("Systems.as", (std::string(Systems) +
        "class MoveSystem : IGameplaySystem\n"                   // 8
        "{\n"                                                    // 9
        "    int Count;\n"                                       // 10
        "    void Tick() {}\n"                                   // 11
        "}\n").c_str()));
    FT_CHECK(E.HasMessage("Systems.as:10:9: Class 'MoveSystem' implements 'IGameplaySystem' and may not declare member 'int Count'"));
}

FT_TEST(NoScriptStateRefusesInheritedMembers)
{
    FeatureTest::FEngine E;
    asSolAddStatelessInterface(E.Engine, "IGameplaySystem");

    FT_CHECK(!E.Build("Systems.as", (std::string(Systems) +
        "class MoveSystem : Base, IGameplaySystem {}\n").c_str()));
    FT_CHECK(E.HasMessage("Systems.as:6:9: Class 'MoveSystem' implements 'IGameplaySystem' and may not declare member 'int Inherited'"));
}

FT_TEST(NoScriptStateFollowsInterfaceInheritance)
{
    FeatureTest::FEngine E;
    asSolAddStatelessInterface(E.Engine, "IGameplaySystem");

    FT_CHECK(!E.Build("Systems.as", (std::string(Systems) +
        "class Special : ISpecialSystem { int Count; }\n").c_str()));
    FT_CHECK(E.HasMessage("implements 'IGameplaySystem' and may not declare member 'int Count'"));
}

FT_TEST(NoScriptStateMatchesQualifiedNames)
{
    {
        FeatureTest::FEngine E;
        asSolAddStatelessInterface(E.Engine, "Game::IRenderSystem");
        FT_CHECK(!E.Build("Systems.as", (std::string(Systems) + "class Draw : Game::IRenderSystem { int Count; }\n").c_str()));
    }
    {
        // The unqualified name does not match an interface in a namespace.
        FeatureTest::FEngine E;
        asSolAddStatelessInterface(E.Engine, "IRenderSystem");
        FT_CHECK(E.Build("Systems.as", (std::string(Systems) + "class Draw : Game::IRenderSystem { int Count; }\n").c_str()));
    }
}

FT_TEST(NoScriptStateLeavesOtherClassesAlone)
{
    {
        FeatureTest::FEngine E;
        asSolAddStatelessInterface(E.Engine, "IGameplaySystem");
        FT_CHECK(E.Build("Systems.as", (std::string(Systems) +
            "class Helper { int Count; }\n"
            "class MoveSystem : IGameplaySystem { void Tick() { Helper h; h.Count++; } }\n").c_str()));
    }
    {
        // Nothing is refused until the application names an interface.
        FeatureTest::FEngine E;
        FT_CHECK(E.Build("Systems.as", (std::string(Systems) + "class MoveSystem : IGameplaySystem { int Count; }\n").c_str()));
    }
    FT_CHECK(asSolAddStatelessInterface(nullptr, "IGameplaySystem") == asINVALID_ARG);
    {
        FeatureTest::FEngine E;
        FT_CHECK(asSolAddStatelessInterface(E.Engine, "") == asINVALID_ARG);
    }
}

FT_TEST(NoScriptStateAcceptsConstantPlainGlobals)
{
    FeatureTest::FEngine E;
    Register(E.Engine);
    FT_CHECK(asSolSetConstantGlobalsOnly(E.Engine, true) == asSUCCESS);

    FT_CHECK(E.Build("Globals.as",
        "enum EMode { Fast }\n"
        "const int Limit = 3;\n"
        "const double Scale = 0.5;\n"
        "const EMode Mode = Fast;\n"
        "const FVector Origin;\n"
        "const auto Inferred = 1.5f;\n"));
}

FT_TEST(NoScriptStateRefusesOtherGlobals)
{
    const char* const Refused[][2] = {
        { "int Count;", "'int Count'" },
        { "FVector Origin;", "'FVector Origin'" },
        { "class F {} const F@ Held;", "'const F@ Held'" },
        { "class F {} F@ const Held = F();", "'F@const Held'" },
        { "class F {} const F Config;", "'const F Config'" },
        { "const FOwner Owner;", "'const FOwner Owner'" },
        { "funcdef void FCallback(); const FCallback@ Callback;", "'const FCallback@ Callback'" },
    };

    for (const auto& Case : Refused)
    {
        FeatureTest::FEngine E;
        Register(E.Engine);
        asSolSetConstantGlobalsOnly(E.Engine, true);
        FT_CHECK(!E.Build("Globals.as", Case[0]));
        FT_CHECK(E.HasMessage((std::string("Global variable ") + Case[1] + " must be a constant of a primitive, an enum or a POD value type").c_str()));
    }

    {
        FeatureTest::FEngine E;
        asSolSetConstantGlobalsOnly(E.Engine, true);
        FT_CHECK(!E.Build("Globals.as", "\n  int Count;\n"));
        FT_CHECK(E.HasMessage("Globals.as:2:7: Global variable"));
    }
    {
        FeatureTest::FEngine E;
        asSolSetConstantGlobalsOnly(E.Engine, true);
        asSolSetConstantGlobalsOnly(E.Engine, false);
        FT_CHECK(E.Build("Globals.as", "int Count;"));
    }
}
