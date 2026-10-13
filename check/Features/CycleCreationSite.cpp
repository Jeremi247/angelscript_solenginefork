// AS_SOL_CYCLE_CREATION_SITE: docs/decisions/0006-a-collected-object-says-where-it-was-created.md

#include "FeatureTest.h"

#include <cstring>
#include <string>
#include <vector>

namespace
{
    struct FSite
    {
        std::string Type;
        int Result = 0;
        std::string Section;
        int Row = 0;
        int Col = 0;
    };

    struct FCapture
    {
        asIScriptEngine* Engine = nullptr;
        std::vector<FSite> Sites;
    };

    void OnCycle(asITypeInfo* Type, const void* Object, void* Param)
    {
        FCapture& Capture = *static_cast<FCapture*>(Param);
        FSite Site;
        Site.Type = Type->GetName();
        const char* Section = nullptr;
        Site.Result = asSolGetObjectCreatedAt(Capture.Engine, const_cast<void*>(Object), &Section, &Site.Row, &Site.Col);
        Site.Section = Section ? Section : "";
        Capture.Sites.push_back(Site);
    }

    const char* const Script =
        "class Node\n"                           // 1
        "{\n"                                    // 2
        "    Node@ Next;\n"                      // 3
        "}\n"                                    // 4
        "Node@ Make() { return Node(); }\n"      // 5
        "void Cycle()\n"                         // 6
        "{\n"                                    // 7
        "    Node a;\n"                          // 8
        "    @a.Next = Make();\n"                // 9
        "    @a.Next.Next = a;\n"                // 10
        "}\n";                                   // 11
}

FT_TEST(CycleCreationSiteIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_CYCLE_CREATION_SITE"));
}

FT_TEST(CycleCreationSiteNamesTheLineThatCreated)
{
    FeatureTest::FEngine E;
    FCapture Capture;
    Capture.Engine = E.Engine;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    E.Engine->SetCircularRefDetectedCallback(&OnCycle, &Capture);

    asIScriptModule* Module = E.Build("Nodes.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    FT_CHECK(E.Run(Module, "void Cycle()") == asEXECUTION_FINISHED);
    E.Engine->GarbageCollect(asGC_FULL_CYCLE);

    // The local is created by its declaration, and the other by the line in Make
    // that called the constructor, not by the line that called Make.
    FT_CHECK(Capture.Sites.size() == 2);
    bool SawLocal = false, SawMade = false;
    for (const FSite& Site : Capture.Sites)
    {
        FT_CHECK(Site.Type == "Node" && Site.Result == asSUCCESS && Site.Section == "Nodes.as");
        if (Site.Row == 8)
            SawLocal = true;
        if (Site.Row == 5)
            SawMade = true;
    }
    FT_CHECK(SawLocal);
    FT_CHECK(SawMade);
}

FT_TEST(CycleCreationSiteIsAbsentOutsideScript)
{
    FeatureTest::FEngine E;
    E.Engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);

    asIScriptModule* Module = E.Build("Nodes.as", Script);
    FT_CHECK(Module);
    if (!Module)
        return;

    asITypeInfo* Node = Module->GetTypeInfoByName("Node");
    void* FromCpp = E.Engine->CreateScriptObject(Node);
    FT_CHECK(FromCpp);

    const char* Section = "unset";
    int Row = -1, Col = -1;
    FT_CHECK(asSolGetObjectCreatedAt(E.Engine, FromCpp, &Section, &Row, &Col) == asNOT_SUPPORTED);
    FT_CHECK(Section == nullptr && Row == 0 && Col == 0);

    int NotInCollector = 0;
    FT_CHECK(asSolGetObjectCreatedAt(E.Engine, &NotInCollector, nullptr, nullptr, nullptr) == asINVALID_ARG);

    E.Engine->ReleaseScriptObject(FromCpp, Node);
}
