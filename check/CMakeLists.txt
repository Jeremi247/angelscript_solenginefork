# Upstream's test_feature, built against the library as SolEngine builds it. Step 1 of
# the release check in docs/testing.md; the four ways SolEngine's build differs from
# upstream's CI are listed there. Three are reproduced below; why the allocator is not
# is written there too.
#
# Outside sdk/, so nothing here is vendored or conflicts with upstream
# (docs/decisions/0003-a-commit-of-ours-keeps-upstreams-shape.md).

cmake_minimum_required(VERSION 3.25)

project(SolAngelScriptCheck LANGUAGES CXX ASM_MASM)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    message(FATAL_ERROR "The check builds with MSVC and clang-cl only, as SolEngine does. Found: ${CMAKE_CXX_COMPILER_ID}")
endif()

set(Sdk "${CMAKE_CURRENT_SOURCE_DIR}/../sdk")

# --- SolEngine's root CMakeLists ------------------------------------------------------
# C++20 for the library, and Debug and Development with SolEngine's flags. Shipping
# differs from Development only in debug information (/Zi, /DEBUG), and SolEngine's no-tools builds add
# AS_NO_COMPILER, which the suite cannot run under (docs/testing.md).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CONFIGURATION_TYPES Debug Development CACHE STRING "Build configurations" FORCE)

set(CMAKE_CXX_FLAGS_DEVELOPMENT "/Zi /O2 /Ob2 /DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "/DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_DEVELOPMENT "" CACHE STRING "" FORCE)

# SolEngine strips the default /EHsc for every target, so a target that wants an
# exception model has to name one.
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# --- SolEngine's ThirdParty/AngelScript/CMakeLists.txt --------------------------------
# The same sources, definitions and flags, in step with that file by hand. Why the four
# per-target call conventions upstream also lists are left out is written there.
set(AngelScriptSources
    ${Sdk}/angelscript/source/as_atomic.cpp
    ${Sdk}/angelscript/source/as_builder.cpp
    ${Sdk}/angelscript/source/as_bytecode.cpp
    ${Sdk}/angelscript/source/as_callfunc.cpp
    ${Sdk}/angelscript/source/as_callfunc_x64_msvc.cpp
    ${Sdk}/angelscript/source/as_compiler.cpp
    ${Sdk}/angelscript/source/as_configgroup.cpp
    ${Sdk}/angelscript/source/as_context.cpp
    ${Sdk}/angelscript/source/as_datatype.cpp
    ${Sdk}/angelscript/source/as_gc.cpp
    ${Sdk}/angelscript/source/as_generic.cpp
    ${Sdk}/angelscript/source/as_globalproperty.cpp
    ${Sdk}/angelscript/source/as_memory.cpp
    ${Sdk}/angelscript/source/as_module.cpp
    ${Sdk}/angelscript/source/as_objecttype.cpp
    ${Sdk}/angelscript/source/as_outputbuffer.cpp
    ${Sdk}/angelscript/source/as_parser.cpp
    ${Sdk}/angelscript/source/as_restore.cpp
    ${Sdk}/angelscript/source/as_scriptcode.cpp
    ${Sdk}/angelscript/source/as_scriptengine.cpp
    ${Sdk}/angelscript/source/as_scriptfunction.cpp
    ${Sdk}/angelscript/source/as_scriptnode.cpp
    ${Sdk}/angelscript/source/as_scriptobject.cpp
    ${Sdk}/angelscript/source/as_sol.cpp
    ${Sdk}/angelscript/source/as_string.cpp
    ${Sdk}/angelscript/source/as_string_util.cpp
    ${Sdk}/angelscript/source/as_thread.cpp
    ${Sdk}/angelscript/source/as_tokenizer.cpp
    ${Sdk}/angelscript/source/as_typeinfo.cpp
    ${Sdk}/angelscript/source/as_variablescope.cpp
    ${Sdk}/angelscript/source/as_callfunc_x64_msvc_asm.asm
    ${Sdk}/angelscript/include/angelscript.h
    ${Sdk}/angelscript/include/angelscript_sol.h)

add_library(SolAngelScript STATIC ${AngelScriptSources})

target_include_directories(SolAngelScript SYSTEM PUBLIC "${Sdk}/angelscript/include")

target_compile_definitions(SolAngelScript PRIVATE
    AS_NO_EXCEPTIONS
    _CRT_SECURE_NO_WARNINGS)

# SolThirdPartyFlags, from SolEngine's CMake/SolAddModule.cmake.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(SolAngelScript PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W0 /utf-8 /MP /GR- /EHs-c->)
else()
    target_compile_options(SolAngelScript PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W0 /utf-8 /GR- /EHs-c->)
endif()

# --- Upstream's test_feature ----------------------------------------------------------
# The sources are read out of upstream's own CMakeLists rather than listed again, so a
# test upstream adds is built here without an edit. Its library is not used: that is
# the half this file replaces.
set(UpstreamProject "${Sdk}/tests/test_feature/projects/cmake")
file(STRINGS "${UpstreamProject}/CMakeLists.txt" UpstreamLines REGEX "^[ \t]*[^ \t#]+\\.cpp[ \t]*$")

set(TestFeatureSources "")
foreach(Line IN LISTS UpstreamLines)
    string(STRIP "${Line}" Relative)
    cmake_path(ABSOLUTE_PATH Relative BASE_DIRECTORY "${UpstreamProject}" NORMALIZE OUTPUT_VARIABLE Absolute)
    list(APPEND TestFeatureSources "${Absolute}")
endforeach()

list(LENGTH TestFeatureSources TestFeatureSourceCount)
if(TestFeatureSourceCount LESS 100)
    message(FATAL_ERROR "Read ${TestFeatureSourceCount} sources out of ${UpstreamProject}/CMakeLists.txt; its shape has changed")
endif()

add_executable(test_feature ${TestFeatureSources})

target_link_libraries(test_feature PRIVATE SolAngelScript)

# The suite and the add-ons are upstream's test code, not what SolEngine ships, and they
# are built as upstream builds them: their own standard, RTTI and exceptions on. clang-cl
# refuses try and dynamic_cast without them. The tests that would throw through the
# library skip themselves when it reports AS_NO_EXCEPTIONS.
set_target_properties(test_feature PROPERTIES CXX_STANDARD 11)
target_compile_options(test_feature PRIVATE /EHsc /W0)
target_compile_definitions(test_feature PRIVATE _CRT_SECURE_NO_WARNINGS)

# --- The fork's features --------------------------------------------------------------
# Step 2 of the release check: the same library again, with every AS_SOL_ feature on,
# and the features' own tests against it
# (docs/decisions/0004-a-feature-is-tested-beside-upstreams-suite.md). PUBLIC, because
# angelscript_sol.h declares a feature only where its definition is set.
set(SolFeatures
    AS_SOL_DECLARATION_POSITIONS
    AS_SOL_CYCLE_CREATION_SITE
    AS_SOL_GC_REPORTS_DONE
    AS_SOL_DEEP_CONST
    AS_SOL_NO_SCRIPT_STATE
    AS_SOL_SAFE_VALUE_INOUT
    AS_SOL_SIZED_FREE)

add_library(SolAngelScriptFeatures STATIC ${AngelScriptSources})

target_include_directories(SolAngelScriptFeatures SYSTEM PUBLIC "${Sdk}/angelscript/include")

target_compile_definitions(SolAngelScriptFeatures
    PRIVATE AS_NO_EXCEPTIONS _CRT_SECURE_NO_WARNINGS
    PUBLIC ${SolFeatures})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(SolAngelScriptFeatures PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W0 /utf-8 /MP /GR- /EHs-c->)
else()
    target_compile_options(SolAngelScriptFeatures PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W0 /utf-8 /GR- /EHs-c->)
endif()

# The tests are compiled as SolEngine compiles its own code against the library: C++20,
# warnings as errors, no RTTI and no exceptions.
file(GLOB FeatureTestSources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Features/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Features/*.h")

add_executable(test_sol_features ${FeatureTestSources})

target_link_libraries(test_sol_features PRIVATE SolAngelScriptFeatures)
target_compile_options(test_sol_features PRIVATE /W4 /WX /utf-8 /permissive- /GR- /EHs-c- /external:W0)
target_compile_definitions(test_sol_features PRIVATE _HAS_EXCEPTIONS=0 _CRT_SECURE_NO_WARNINGS)
