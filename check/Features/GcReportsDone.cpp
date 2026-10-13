// AS_SOL_GC_REPORTS_DONE: docs/decisions/0007-a-collector-step-says-when-it-is-done.md

#include "FeatureTest.h"

namespace
{
    const char* const Script =
        "class Node { Node@ Next; }\n"
        "void Garbage(int Count) { for (int n = 0; n < Count; n++) { Node a; @a.Next = Node(); } }\n"
        "void Cycles(int Count) { for (int n = 0; n < Count; n++) { Node a; @a.Next = Node(); @a.Next.Next = a; } }\n";

    asUINT Held(asIScriptEngine* Engine)
    {
        asUINT Size = 0;
        Engine->GetGCStatistics(&Size);
        return Size;
    }

    // Steps until the collector says it is done, and returns how many steps that took,
    // or -1 if it never did within Limit.
    int StepUntilDone(asIScriptEngine* Engine, int Limit)
    {
        for (int Steps = 1; Steps <= Limit; ++Steps)
            if (Engine->GarbageCollect(asGC_ONE_STEP) == 0)
                return Steps;
        return -1;
    }

    void Call(FeatureTest::FEngine& E, asIScriptModule* Module, const char* Declaration, int Count)
    {
        asIScriptContext* Context = E.Engine->CreateContext();
        Context->Prepare(Module->GetFunctionByDecl(Declaration));
        Context->SetArgDWord(0, static_cast<asDWORD>(Count));
        FT_CHECK(Context->Execute() == asEXECUTION_FINISHED);
        Context->Release();
    }
}

FT_TEST(GcReportsDoneIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_GC_REPORTS_DONE"));
}

FT_TEST(GcReportsDoneWhenEmpty)
{
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    FT_CHECK(E.Engine->GarbageCollect(asGC_ONE_STEP) == 0);
    FT_CHECK(E.Engine->GarbageCollect(asGC_FULL_CYCLE) == 0);
}

FT_TEST(GcReportsDoneOnlyOnceGarbageIsGone)
{
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    asIScriptModule* Module = E.Build("Gc.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    Call(E, Module, "void Garbage(int)", 50);
    FT_CHECK(Held(E.Engine) == 100);

    FT_CHECK(E.Engine->GarbageCollect(asGC_ONE_STEP) == 1);
    FT_CHECK(StepUntilDone(E.Engine, 10000) > 0);
    FT_CHECK(Held(E.Engine) == 0);
}

FT_TEST(GcReportsDoneWithLiveObjectsHeld)
{
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    asIScriptModule* Module = E.Build("Gc.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    // A live object never becomes garbage. Upstream's step returns 1 for it forever.
    asITypeInfo* Node = Module->GetTypeInfoByName("Node");
    void* Live = E.Engine->CreateScriptObject(Node);

    int First = StepUntilDone(E.Engine, 10000);
    FT_CHECK(First > 0);
    FT_CHECK(Held(E.Engine) == 1);

    // The call after a 0 starts over, and a settled collector settles again in as
    // many steps.
    FT_CHECK(StepUntilDone(E.Engine, 10000) == First);

    E.Engine->ReleaseScriptObject(Live, Node);
    FT_CHECK(StepUntilDone(E.Engine, 10000) > 0);
    FT_CHECK(Held(E.Engine) == 0);
}

FT_TEST(GcReportsDoneOnlyOnceCyclesAreBroken)
{
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    asIScriptModule* Module = E.Build("Gc.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    Call(E, Module, "void Cycles(int)", 20);
    FT_CHECK(Held(E.Engine) == 40);

    // Done may take more than one round: a round that ages objects into the old
    // list is not quiet, so the detection that finds the cycles comes later.
    for (int Round = 0; Round < 20 && Held(E.Engine) > 0; ++Round)
        FT_CHECK(StepUntilDone(E.Engine, 100000) > 0);
    FT_CHECK(Held(E.Engine) == 0);

    asUINT Detected = 0;
    E.Engine->GetGCStatistics(nullptr, nullptr, &Detected);
    FT_CHECK(Detected == 40);
}

FT_TEST(GcReportsDoneStopsIterating)
{
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    asIScriptModule* Module = E.Build("Gc.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    Call(E, Module, "void Garbage(int)", 10);
    FT_CHECK(E.Engine->GarbageCollect(asGC_ONE_STEP, 100000) == 0);
    FT_CHECK(Held(E.Engine) == 0);
}
