// The features' own tests: step 2 of the release check in docs/testing.md. Each file
// tests one AS_SOL_ feature against the library built with every feature on, as
// decided in docs/decisions/0004-a-feature-is-tested-beside-upstreams-suite.md.
//
// Outside sdk/, so nothing here is vendored or conflicts with upstream.

#pragma once

#include <angelscript.h>
#include <angelscript_sol.h>

#include <string>
#include <vector>

namespace FeatureTest
{
    struct FTest
    {
        const char* Name;
        void (*Run)();
    };

    std::vector<FTest>& Tests();

    struct FRegister
    {
        FRegister(const char* Name, void (*Run)()) { Tests().push_back({ Name, Run }); }
    };

    void Fail(const char* File, int Line, const char* Expression);

    // Whether asGetLibraryOptions() lists the definition, followed by a space as it
    // writes every option.
    bool IsReported(const char* Definition);

    // An engine whose message callback keeps every message, one per line, as
    // "section:row:col: text".
    struct FEngine
    {
        FEngine();
        ~FEngine();

        FEngine(const FEngine&) = delete;
        FEngine& operator=(const FEngine&) = delete;

        // Builds Code as one section into a fresh module, and returns the module, or
        // null when the build fails.
        asIScriptModule* Build(const char* Section, const char* Code, const char* ModuleName = "Test");

        // Runs a function of the module by declaration, and returns the context's
        // result, asEXECUTION_FINISHED on success.
        int Run(asIScriptModule* Module, const char* Declaration);

        bool HasMessage(const char* Text) const { return Messages.find(Text) != std::string::npos; }

        asIScriptEngine* Engine = nullptr;
        std::string Messages;
    };
}

#define FT_CHECK(Expression) \
    do { if (!(Expression)) ::FeatureTest::Fail(__FILE__, __LINE__, #Expression); } while (0)

#define FT_TEST(Name) \
    static void Name(); \
    static ::FeatureTest::FRegister Name##Registration(#Name, &Name); \
    static void Name()
