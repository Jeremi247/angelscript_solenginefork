// AS_SOL_SIZED_FREE: docs/decisions/0011-a-free-is-told-the-size.md

#include "FeatureTest.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <unordered_map>
#include <vector>

namespace
{
    // An allocator that needs the size to free, as SolEngine's does, and checks it.
    struct FLedger
    {
        std::unordered_map<void*, size_t> Live;
        size_t Allocations = 0;
        size_t Frees = 0;
        size_t WrongSize = 0;
        size_t Unknown = 0;
    };

    FLedger* GLedger = nullptr;

    void* LedgerAlloc(size_t Size)
    {
        void* Memory = std::malloc(Size ? Size : 1);
        if (GLedger)
        {
            GLedger->Live[Memory] = Size;
            ++GLedger->Allocations;
        }
        return Memory;
    }

    void LedgerFree(void* Memory, size_t Size)
    {
        if (GLedger)
        {
            auto Found = GLedger->Live.find(Memory);
            if (Found == GLedger->Live.end())
                ++GLedger->Unknown;
            else
            {
                if (Found->second != Size)
                    ++GLedger->WrongSize;
                GLedger->Live.erase(Found);
            }
            ++GLedger->Frees;
        }
        std::free(Memory);
    }

    struct FVec
    {
        float X;
        float Y;
    };

    struct FOwner
    {
        int Value;
    };

    void ConstructOwner(void* Memory) { new (Memory) FOwner{ 7 }; }
    void CopyConstructOwner(const FOwner& Other, void* Memory) { new (Memory) FOwner{ Other.Value }; }
    void DestructOwner(void*) {}

    float TakeVec(FVec V) { return V.X + V.Y; }

    struct FIntList
    {
        int Sum = 0;
    };

    FIntList GList;

    FIntList* MakeList(void* Buffer)
    {
        asUINT Count = *static_cast<asUINT*>(Buffer);
        const int* Values = reinterpret_cast<const int*>(static_cast<asUINT*>(Buffer) + 1);
        GList.Sum = 0;
        for (asUINT n = 0; n < Count; ++n)
            GList.Sum += Values[n];
        return &GList;
    }

    struct FStream : asIBinaryStream
    {
        std::vector<char> Bytes;
        size_t ReadAt = 0;

        int Write(const void* Data, asUINT Size) override
        {
            const char* From = static_cast<const char*>(Data);
            Bytes.insert(Bytes.end(), From, From + Size);
            return 0;
        }

        int Read(void* Data, asUINT Size) override
        {
            if (ReadAt + Size > Bytes.size())
                return -1;
            std::memcpy(Data, Bytes.data() + ReadAt, Size);
            ReadAt += Size;
            return 0;
        }
    };

    void Register(asIScriptEngine* Engine)
    {
        Engine->RegisterObjectType("FVec", sizeof(FVec), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<FVec>());
        Engine->RegisterObjectProperty("FVec", "float X", asOFFSET(FVec, X));
        Engine->RegisterObjectProperty("FVec", "float Y", asOFFSET(FVec, Y));
        Engine->RegisterGlobalFunction("float TakeVec(FVec)", asFUNCTION(TakeVec), asCALL_CDECL);

        Engine->RegisterObjectType("FOwner", sizeof(FOwner), asOBJ_VALUE | asOBJ_APP_CLASS_CK);
        Engine->RegisterObjectBehaviour("FOwner", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ConstructOwner), asCALL_CDECL_OBJLAST);
        Engine->RegisterObjectBehaviour("FOwner", asBEHAVE_CONSTRUCT, "void f(const FOwner &in)", asFUNCTION(CopyConstructOwner), asCALL_CDECL_OBJLAST);
        Engine->RegisterObjectBehaviour("FOwner", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructOwner), asCALL_CDECL_OBJLAST);
        Engine->RegisterObjectProperty("FOwner", "int Value", asOFFSET(FOwner, Value));

        Engine->RegisterObjectType("FIntList", 0, asOBJ_REF | asOBJ_NOCOUNT);
        Engine->RegisterObjectBehaviour("FIntList", asBEHAVE_LIST_FACTORY, "FIntList@ f(int &in) {repeat int}", asFUNCTION(MakeList), asCALL_CDECL);
        Engine->RegisterObjectProperty("FIntList", "int Sum", asOFFSET(FIntList, Sum));
    }

    // Exercises every kind of free the library has: compiled code and its buffers,
    // script objects and cycles, value objects on the heap, list buffers, a deep
    // stack, an exception unwinding locals, globals, and bytecode saved and loaded.
    const char* const Workload =
        "class Node { Node@ Next; FOwner Owned; string_like Name; }\n"
        "class string_like { int Length = 12345678; }\n"
        "FOwner GlobalOwner;\n"
        "FVec GlobalVec;\n"
        "int Depth(int n) { FOwner local; return n == 0 ? local.Value : Depth(n - 1); }\n"
        "int Cycles() { for (int n = 0; n < 50; n++) { Node a; @a.Next = Node(); @a.Next.Next = a; } return 0; }\n"
        "int List() { FIntList@ l = {1, 2, 3, 4}; return l.Sum; }\n"
        "float Vec() { FVec v; v.X = 1; v.Y = 2; return TakeVec(v); }\n"
        "int Divide(int d) { FOwner held; Node n; return held.Value / d; }\n"
        "int ByValue(FOwner o) { int zero = 0; return o.Value / zero; }\n"
        "int Second(int b, FOwner a) { return b; }\n"
        "int Unwind() { return Second(Divide(0), GlobalOwner); }\n"
        "int Run() { return Depth(3000) + Cycles() + List() + int(Vec()) + GlobalOwner.Value; }\n";

    void RunWorkload(asIScriptEngine* Engine, FeatureTest::FEngine& E)
    {
        asIScriptModule* Module = E.Build("Sized.as", Workload);
        FT_CHECK(Module);
        if (!Module)
        {
            std::fputs(E.Messages.c_str(), stdout);
            return;
        }

        asIScriptContext* Context = Engine->CreateContext();
        Context->Prepare(Module->GetFunctionByDecl("int Run()"));
        FT_CHECK(Context->Execute() == asEXECUTION_FINISHED);
        FT_CHECK(Context->GetReturnDWord() == 7 + 0 + 10 + 3 + 7);

        Context->Prepare(Module->GetFunctionByDecl("int Divide(int)"));
        Context->SetArgDWord(0, 0);
        FT_CHECK(Context->Execute() == asEXECUTION_EXCEPTION);

        Context->Prepare(Module->GetFunctionByDecl("int Unwind()"));
        FT_CHECK(Context->Execute() == asEXECUTION_EXCEPTION);

        Context->Prepare(Module->GetFunctionByDecl("int ByValue(FOwner)"));
        FOwner Arg{ 1 };
        Context->SetArgObject(0, &Arg);
        FT_CHECK(Context->Execute() == asEXECUTION_EXCEPTION);

        Context->Prepare(Module->GetFunctionByDecl("int Second(int, FOwner)"));
        Context->SetArgDWord(0, 5);
        Context->SetArgObject(1, &Arg);
        FT_CHECK(Context->Execute() == asEXECUTION_FINISHED && Context->GetReturnDWord() == 5);
        Context->Release();

        asITypeInfo* Owner = Engine->GetTypeInfoByName("FOwner");
        void* Made = Engine->CreateScriptObject(Owner);
        void* Copied = Engine->CreateScriptObjectCopy(Made, Owner);
        FT_CHECK(Made && Copied && static_cast<FOwner*>(Copied)->Value == 7);
        Engine->ReleaseScriptObject(Made, Owner);
        Engine->ReleaseScriptObject(Copied, Owner);

        Engine->GarbageCollect(asGC_FULL_CYCLE);

        FStream Stream;
        FT_CHECK(Module->SaveByteCode(&Stream) >= 0);
        asIScriptModule* Loaded = Engine->GetModule("Loaded", asGM_ALWAYS_CREATE);
        FT_CHECK(Loaded->LoadByteCode(&Stream) >= 0);
        Loaded->Discard();
        Module->Discard();
    }
}

FT_TEST(SizedFreeIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_SIZED_FREE"));
}

FT_TEST(SizedFreeIsToldTheSizeOfEveryBlock)
{
    FLedger Ledger;
    GLedger = &Ledger;
    FT_CHECK(asSolSetGlobalMemoryFunctions(&LedgerAlloc, &LedgerFree) == asSUCCESS);

    {
        FeatureTest::FEngine E;
        Register(E.Engine);
        RunWorkload(E.Engine, E);
    }
    asThreadCleanup();

    FT_CHECK(Ledger.Allocations > 1000);
    FT_CHECK(Ledger.Frees == Ledger.Allocations);
    FT_CHECK(Ledger.WrongSize == 0);
    FT_CHECK(Ledger.Unknown == 0);
    FT_CHECK(Ledger.Live.empty());

    asResetGlobalMemoryFunctions();
    GLedger = nullptr;
}

FT_TEST(SizedFreeKeepsAllocMemWorking)
{
    FLedger Ledger;
    GLedger = &Ledger;
    asSolSetGlobalMemoryFunctions(&LedgerAlloc, &LedgerFree);

    void* A = asAllocMem(40);
    void* B = asAllocMem(3);
    FT_CHECK(A && B);
    // The header keeps the alignment malloc gives, 16 bytes on x64.
    FT_CHECK(reinterpret_cast<size_t>(A) % 16 == 0 && reinterpret_cast<size_t>(B) % 16 == 0);
    asFreeMem(A);
    asSolFreeMem(B, 3);
    asFreeMem(nullptr);

    FT_CHECK(Ledger.WrongSize == 0 && Ledger.Unknown == 0 && Ledger.Live.empty());

    asResetGlobalMemoryFunctions();
    GLedger = nullptr;
}

FT_TEST(SizedFreeFallsBackToTheUnsizedFree)
{
    // asSetGlobalMemoryFunctions after the sized form returns to an unsized free.
    asSolSetGlobalMemoryFunctions(&LedgerAlloc, &LedgerFree);
    FT_CHECK(asSolSetGlobalMemoryFunctions(nullptr, &LedgerFree) == asINVALID_ARG);
    asSetGlobalMemoryFunctions(std::malloc, std::free);
    {
        FeatureTest::FEngine E;
        Register(E.Engine);
        RunWorkload(E.Engine, E);
    }
    asResetGlobalMemoryFunctions();
}
