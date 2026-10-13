#include "FeatureTest.h"

#include <cstdio>
#include <cstring>

namespace FeatureTest
{
    static int GFailures = 0;
    static const char* GCurrent = "";

    std::vector<FTest>& Tests()
    {
        static std::vector<FTest> All;
        return All;
    }

    void Fail(const char* File, int Line, const char* Expression)
    {
        ++GFailures;
        std::printf("FAILED %s, %s(%d): %s\n", GCurrent, File, Line, Expression);
    }

    bool IsReported(const char* Definition)
    {
        std::string Wanted = std::string(" ") + Definition + " ";
        return std::strstr(asGetLibraryOptions(), Wanted.c_str()) != nullptr;
    }

    static void MessageCallback(const asSMessageInfo* Message, void* Param)
    {
        std::string& Out = *static_cast<std::string*>(Param);
        char Position[64];
        std::snprintf(Position, sizeof(Position), ":%d:%d: ", Message->row, Message->col);
        Out += Message->section;
        Out += Position;
        Out += Message->message;
        Out += "\n";
    }

    FEngine::FEngine()
    {
        Engine = asCreateScriptEngine();
        Engine->SetMessageCallback(asFUNCTION(MessageCallback), &Messages, asCALL_CDECL);
    }

    FEngine::~FEngine()
    {
        if (Engine)
            Engine->ShutDownAndRelease();
    }

    asIScriptModule* FEngine::Build(const char* Section, const char* Code, const char* ModuleName)
    {
        asIScriptModule* Module = Engine->GetModule(ModuleName, asGM_ALWAYS_CREATE);
        Module->AddScriptSection(Section, Code);
        if (Module->Build() < 0)
        {
            Module->Discard();
            return nullptr;
        }
        return Module;
    }

    int FEngine::Run(asIScriptModule* Module, const char* Declaration)
    {
        asIScriptFunction* Function = Module->GetFunctionByDecl(Declaration);
        if (!Function)
            return asNO_FUNCTION;

        asIScriptContext* Context = Engine->CreateContext();
        Context->Prepare(Function);
        int Result = Context->Execute();
        if (Result == asEXECUTION_EXCEPTION)
        {
            Messages += "exception: ";
            Messages += Context->GetExceptionString();
            Messages += "\n";
        }
        Context->Release();
        return Result;
    }
}

int main()
{
    using namespace FeatureTest;

    std::printf("AngelScript options:%s\n", asGetLibraryOptions());

    for (const FTest& Test : Tests())
    {
        GCurrent = Test.Name;
        int Before = GFailures;
        Test.Run();
        std::printf("%s %s\n", GFailures == Before ? "passed" : "FAILED", Test.Name);
    }

    std::printf("%d tests, %d failed checks\n", static_cast<int>(Tests().size()), GFailures);
    return GFailures == 0 && !Tests().empty() ? 0 : 1;
}
