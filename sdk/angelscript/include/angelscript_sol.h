/*
   AngelCode Scripting Library
   Copyright (c) 2003-2026 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// angelscript_sol.h
//
// This file is not part of the original library. It is added by the
// fork maintained for SolEngine, and declares what that fork's features
// add to the interface. Each feature is compiled only when its AS_SOL_
// definition is set, and the definition must be set both where the
// library is compiled and where this header is included.
//


#ifndef ANGELSCRIPT_SOL_H
#define ANGELSCRIPT_SOL_H

#include "angelscript.h"

BEGIN_AS_NAMESPACE

#ifdef AS_SOL_SIZED_FREE
typedef void (*asSIZEDFREEFUNC_t)(void *, size_t);
#endif

#ifndef ANGELSCRIPT_DLL_MANUAL_IMPORT
extern "C"
{
#ifdef AS_SOL_DECLARATION_POSITIONS
	// Where a script declared a type, the property at index of a type, or the
	// global variable at index of a module. The position is that of the
	// declared name. A declaration that was registered by the application, or
	// loaded from bytecode, has no position and asNOT_SUPPORTED is returned,
	// as asIScriptFunction::GetDeclaredAt returns for a function without one.
	AS_API int asSolGetTypeDeclaredAt(const asITypeInfo *type, const char **scriptSection, int *row, int *col);
	AS_API int asSolGetPropertyDeclaredAt(const asITypeInfo *type, asUINT index, const char **scriptSection, int *row, int *col);
	AS_API int asSolGetGlobalVarDeclaredAt(const asIScriptModule *module, asUINT index, const char **scriptSection, int *row, int *col);
#endif

#ifdef AS_SOL_GC_REPORTS_DONE
	// No new function. asIScriptEngine::GarbageCollect without asGC_FULL_CYCLE
	// returns 0 once each incremental step it runs has ended a pass that found
	// nothing to do, with nothing destroyed, detected, added or aged since,
	// and stops iterating there. It returns 1 otherwise, as upstream always
	// does. The call after a 0 starts over.
#endif

#ifdef AS_SOL_DEEP_CONST
	// No new function. A handle member read through a const object, or
	// through this in a const method, is a handle to a const object. A
	// funcdef member is not changed.
#endif

#ifdef AS_SOL_NO_SCRIPT_STATE
	// Tells the engine what a module it builds may not hold. Both apply to
	// source compiled after the call, and refuse it with an ordinary compile
	// error at the declaration. Bytecode that is loaded is not checked.
	//
	// A script class implementing the named interface, directly or through a
	// base class or another interface, may declare no members, inherited ones
	// included. The name is the interface's, qualified with its namespace when
	// it has one, as "ns::IName". Returns asINVALID_ARG for an empty name.
	AS_API int asSolAddStatelessInterface(asIScriptEngine *engine, const char *name);

	// With the rule on, a global variable must be const, and of a primitive,
	// an enum or a value type registered as asOBJ_POD. A handle, a script
	// object and a value type with behaviours are refused even when const.
	AS_API int asSolSetConstantGlobalsOnly(asIScriptEngine *engine, bool on);
#endif

#ifdef AS_SOL_SAFE_VALUE_INOUT
	// No new function. With asEP_ALLOW_UNSAFE_REFERENCES off, a value type may
	// be declared &inout, by a script or by the application. An argument the
	// compiler knows outlives the call, such as a local variable or a
	// parameter, is passed by address. Any other must be a variable followed
	// by property accesses, and is passed as a copy that is assigned back to
	// it after the call. A const &inout takes a copy of any expression.
#endif

#ifdef AS_SOL_SIZED_FREE
	// As asSetGlobalMemoryFunctions, but the free function is told the size the
	// block was allocated with. Like it, this must be called before any engine
	// is created. asSetGlobalMemoryFunctions and asResetGlobalMemoryFunctions
	// return to an unsized free.
	AS_API int  asSolSetGlobalMemoryFunctions(asALLOCFUNC_t allocFunc, asSIZEDFREEFUNC_t freeFunc);

	// asAllocMem and asFreeMem still work, as each block they hand out keeps
	// its own size in front of it. asSolFreeMem frees such a block too, and
	// checks the size given against the one it kept.
	AS_API void asSolFreeMem(void *mem, size_t size);
#endif

#ifdef AS_SOL_CYCLE_CREATION_SITE
	// Where a garbage collected object was created: the line executing in the
	// innermost script function of the active context when the collector was
	// told of the object. An object created with no script executing returns
	// asNOT_SUPPORTED, and one the collector does not hold asINVALID_ARG. Meant
	// for the circular reference callback, which is handed the object.
	AS_API int asSolGetObjectCreatedAt(asIScriptEngine *engine, void *obj, const char **scriptSection, int *row, int *col);
#endif
}
#endif // ANGELSCRIPT_DLL_MANUAL_IMPORT

END_AS_NAMESPACE

#endif
