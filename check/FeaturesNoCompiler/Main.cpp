// Links against the library built with every feature on and AS_NO_COMPILER, as SolEngine's
// builds without tools configure it, and touches each feature's interface so a symbol the
// configuration lost fails at link time. Nothing here compiles script.

#include <angelscript.h>
#include <angelscript_sol.h>

#include <cstdlib>

namespace
{
    void* Alloc(size_t Size) { return std::malloc(Size); }
    void Free(void* Memory, size_t) { std::free(Memory); }

    struct FVec
    {
        float X;
    };

    void Bump(FVec& V) { V.X += 1.0f; }
}

int main()
{
    if (asSolSetGlobalMemoryFunctions(&Alloc, &Free) < 0)
        return 1;

    asIScriptEngine* Engine = asCreateScriptEngine();
    if (!Engine)
        return 1;

    int Failures = 0;
    Failures += Engine->RegisterObjectType("FVec", sizeof(FVec), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<FVec>()) < 0;
    Failures += Engine->RegisterGlobalFunction("void Bump(FVec &inout)", asFUNCTION(Bump), asCALL_CDECL) < 0;
    Failures += asSolAddStatelessInterface(Engine, "IGameplaySystem") < 0;
    Failures += asSolSetConstantGlobalsOnly(Engine, true) < 0;
    Failures += asSolGetTypeDeclaredAt(Engine->GetTypeInfoByName("FVec"), nullptr, nullptr, nullptr) != asNOT_SUPPORTED;
    Failures += Engine->GarbageCollect(asGC_ONE_STEP) != 0;

    int Local = 0;
    Failures += asSolGetObjectCreatedAt(Engine, &Local, nullptr, nullptr, nullptr) != asINVALID_ARG;

    void* Block = asAllocMem(24);
    asSolFreeMem(Block, 24);

    Engine->ShutDownAndRelease();
    asResetGlobalMemoryFunctions();
    return Failures;
}
