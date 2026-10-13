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
// as_sol.cpp
//
// This file is not part of the original library. It implements the
// features of the fork maintained for SolEngine, each behind its AS_SOL_
// definition. With none set, it compiles to nothing.
//


#include "as_config.h"
#include "as_sol.h"
#include "../include/angelscript_sol.h"
#include "as_scriptengine.h"
#include "as_module.h"
#include "as_objecttype.h"
#include "as_property.h"
#include "as_scriptcode.h"
#include "as_scriptnode.h"
#include "as_context.h"
#include "as_gc.h"
#include "as_builder.h"
#include "as_compiler.h"

BEGIN_AS_NAMESPACE

#ifdef AS_SOL_DECLARATION_POSITIONS

#ifndef AS_NO_COMPILER
void asSolSetDeclaredAt(int &scriptSectionIdx, int &declaredAt, asCScriptCode *file, asCScriptNode *node)
{
	if( file == 0 || node == 0 )
		return;

	int row, col;
	file->ConvertPosToRowCol(node->tokenPos, &row, &col);
	scriptSectionIdx = file->idx;
	declaredAt       = (row & 0xFFFFF)|((col & 0xFFF)<<20);
}

void asSolSetDeclaredAt(asSSolDeclaredAt &out, asCScriptCode *file, asCScriptNode *node)
{
	asSolSetDeclaredAt(out.scriptSectionIdx, out.declaredAt, file, node);
}
#endif

static int asSolReturnDeclaredAt(asCScriptEngine *engine, int scriptSectionIdx, int declaredAt, const char **scriptSection, int *row, int *col)
{
	if( scriptSectionIdx < 0 || engine == 0 )
	{
		if( scriptSection ) *scriptSection = 0;
		if( row ) *row = 0;
		if( col ) *col = 0;
		return asNOT_SUPPORTED;
	}

	if( scriptSection ) *scriptSection = engine->scriptSectionNames[scriptSectionIdx]->AddressOf();
	if( row ) *row = declaredAt & 0xFFFFF;
	if( col ) *col = (declaredAt >> 20) & 0xFFF;
	return asSUCCESS;
}

static int asSolInvalidDeclaredAt(const char **scriptSection, int *row, int *col)
{
	if( scriptSection ) *scriptSection = 0;
	if( row ) *row = 0;
	if( col ) *col = 0;
	return asINVALID_ARG;
}

extern "C"
{

AS_API int asSolGetTypeDeclaredAt(const asITypeInfo *type, const char **scriptSection, int *row, int *col)
{
	const asCTypeInfo *ti = reinterpret_cast<const asCTypeInfo*>(type);
	if( ti == 0 )
		return asSolInvalidDeclaredAt(scriptSection, row, col);

	return asSolReturnDeclaredAt(ti->engine, ti->scriptSectionIdx, ti->declaredAt, scriptSection, row, col);
}

AS_API int asSolGetPropertyDeclaredAt(const asITypeInfo *type, asUINT index, const char **scriptSection, int *row, int *col)
{
	const asCObjectType *ot = CastToObjectType(const_cast<asCTypeInfo*>(reinterpret_cast<const asCTypeInfo*>(type)));
	if( ot == 0 || index >= ot->properties.GetLength() )
		return asSolInvalidDeclaredAt(scriptSection, row, col);

	const asSSolDeclaredAt &at = ot->properties[index]->solDeclaredAt;
	return asSolReturnDeclaredAt(ot->engine, at.scriptSectionIdx, at.declaredAt, scriptSection, row, col);
}

AS_API int asSolGetGlobalVarDeclaredAt(const asIScriptModule *module, asUINT index, const char **scriptSection, int *row, int *col)
{
	const asCModule *mod = reinterpret_cast<const asCModule*>(module);
	if( mod == 0 )
		return asSolInvalidDeclaredAt(scriptSection, row, col);

	const asCGlobalProperty *prop = mod->m_scriptGlobals.Get(index);
	if( prop == 0 )
		return asSolInvalidDeclaredAt(scriptSection, row, col);

	return asSolReturnDeclaredAt(mod->m_engine, prop->solDeclaredAt.scriptSectionIdx, prop->solDeclaredAt.declaredAt, scriptSection, row, col);
}

}

#endif // AS_SOL_DECLARATION_POSITIONS

#ifdef AS_SOL_NO_SCRIPT_STATE

#ifndef AS_NO_COMPILER

// The first interface named stateless that the type implements, or null
static const asCString *asSolFindStatelessInterface(const asSSolNoScriptState &rules, asCObjectType *ot)
{
	for( asUINT n = 0; n < ot->interfaces.GetLength(); n++ )
	{
		asCObjectType *intf = ot->interfaces[n];
		asCString name = intf->name;
		if( intf->nameSpace && intf->nameSpace->name != "" )
			name = intf->nameSpace->name + "::" + intf->name;

		for( asUINT i = 0; i < rules.statelessInterfaces.GetLength(); i++ )
			if( rules.statelessInterfaces[i] == name )
				return &rules.statelessInterfaces[i];

		// An interface may inherit a stateless one
		const asCString *inherited = asSolFindStatelessInterface(rules, intf);
		if( inherited )
			return inherited;
	}

	return 0;
}

static bool asSolIsPlainValue(const asCDataType &dt)
{
	if( dt.IsObjectHandle() || dt.IsFuncdef() )
		return false;
	if( dt.IsPrimitive() || dt.IsEnumType() )
		return true;

	asCTypeInfo *ti = dt.GetTypeInfo();
	return ti && (ti->flags & asOBJ_VALUE) && (ti->flags & asOBJ_POD) && !(ti->flags & asOBJ_SCRIPT_OBJECT);
}

void asCBuilder::SolCheckScriptState()
{
	const asSSolNoScriptState &rules = engine->solNoScriptState;

	if( rules.statelessInterfaces.GetLength() )
	{
		for( asUINT n = 0; n < classDeclarations.GetLength(); n++ )
		{
			asCObjectType *ot = CastToObjectType(classDeclarations[n]->typeInfo);
			if( ot == 0 )
				continue;

			const asCString *intf = asSolFindStatelessInterface(rules, ot);
			if( intf )
				SolCheckClassHoldsNoState(classDeclarations[n], intf->AddressOf());
		}
	}

	if( rules.constantGlobalsOnly )
	{
		asCSymbolTableIterator<sGlobalVariableDescription> it = globVariables.List();
		for( ; it; it++ )
		{
			sGlobalVariableDescription *gvar = *it;
			if( gvar->isEnumValue )
				continue;

			if( gvar->datatype.IsReadOnly() && asSolIsPlainValue(gvar->datatype) )
				continue;

			asCString str;
			str.Format("Global variable '%s' must be a constant of a primitive, an enum or a POD value type", (gvar->datatype.Format(gvar->ns) + " " + gvar->name).AddressOf());
			WriteError(str, gvar->script, gvar->declaredAtNode);
		}
	}
}

void asCBuilder::SolCheckClassHoldsNoState(sClassDeclaration *decl, const char *interfaceName)
{
	asCObjectType *ot = CastToObjectType(decl->typeInfo);
	for( asUINT n = 0; n < ot->properties.GetLength(); n++ )
	{
		asCObjectProperty *prop = ot->properties[n];

		// Report at the member where it is declared in this build, or else at the class
		asCScriptCode *file = decl->script;
		asCScriptNode *node = decl->node;
		SolFindPropertyNode(ot, prop->name, &file, &node);

		asCString str;
		str.Format("Class '%s' implements '%s' and may not declare member '%s'", ot->name.AddressOf(), interfaceName, (prop->type.Format(ot->nameSpace) + " " + prop->name).AddressOf());
		WriteError(str, file, node);
	}
}

bool asCBuilder::SolFindPropertyNode(asCObjectType *ot, const asCString &name, asCScriptCode **outFile, asCScriptNode **outNode)
{
	for( ; ot; ot = ot->derivedFrom )
	{
		sClassDeclaration *decl = 0;
		for( asUINT n = 0; n < classDeclarations.GetLength(); n++ )
			if( classDeclarations[n]->typeInfo == ot )
				decl = classDeclarations[n];
		if( decl == 0 || decl->node == 0 )
			return false;

		for( asCScriptNode *member = decl->node->firstChild; member; member = member->next )
		{
			if( member->nodeType != snDeclaration )
				continue;

			for( asCScriptNode *nd = member->firstChild; nd; nd = nd->next )
			{
				if( nd->nodeType == snIdentifier && decl->script->TokenEquals(nd->tokenPos, nd->tokenLength, name.AddressOf()) )
				{
					*outFile = decl->script;
					*outNode = nd;
					return true;
				}
			}
		}
	}

	return false;
}

#endif // AS_NO_COMPILER

extern "C"
{

AS_API int asSolAddStatelessInterface(asIScriptEngine *engine, const char *name)
{
	asCScriptEngine *e = reinterpret_cast<asCScriptEngine*>(engine);
	if( e == 0 || name == 0 || name[0] == 0 )
		return asINVALID_ARG;

	e->solNoScriptState.statelessInterfaces.PushLast(asCString(name));
	return asSUCCESS;
}

AS_API int asSolSetConstantGlobalsOnly(asIScriptEngine *engine, bool on)
{
	asCScriptEngine *e = reinterpret_cast<asCScriptEngine*>(engine);
	if( e == 0 )
		return asINVALID_ARG;

	e->solNoScriptState.constantGlobalsOnly = on;
	return asSUCCESS;
}

}

#endif // AS_SOL_NO_SCRIPT_STATE

#ifdef AS_SOL_SAFE_VALUE_INOUT

bool asSolIsValueType(const asCDataType &dt)
{
	return dt.IsObject() && !dt.IsObjectHandle() && dt.GetTypeInfo() && (dt.GetTypeInfo()->flags & asOBJ_VALUE);
}

#ifndef AS_NO_COMPILER

// Whether the expression is a variable followed by property accesses only, which
// has no side effects and can be evaluated a second time to assign to it
static bool asSolIsPlainLValue(asCScriptNode *node)
{
	while( node && (node->nodeType == snAssignment || node->nodeType == snCondition || node->nodeType == snExpression) )
	{
		if( node->firstChild == 0 || node->firstChild != node->lastChild )
			return false;
		node = node->firstChild;
	}

	if( node == 0 || node->nodeType != snExprTerm )
		return false;

	asCScriptNode *n = node->firstChild;
	if( n == 0 || n->nodeType != snExprValue || n->firstChild == 0 || n->firstChild != n->lastChild || n->firstChild->nodeType != snVariableAccess )
		return false;

	for( n = n->next; n; n = n->next )
		if( n->nodeType != snExprPostOp || n->tokenType != ttDot || n->firstChild == 0 || n->firstChild->nodeType != snIdentifier )
			return false;

	return true;
}

int asCCompiler::SolPrepareValueInoutCopy(asCExprContext *ctx, asCScriptNode *node, bool isReadOnly)
{
	// A const reference is not written to, so the copy needs no assignment back
	if( isReadOnly )
	{
		PrepareTemporaryVariable(node, ctx, false, true);
		return 0;
	}

	// The node is where the expression starts, so find the whole argument
	asCScriptNode *arg = node;
	while( arg && arg->parent && arg->parent->nodeType != snArgList && arg->parent->nodeType != snNamedArgument )
		arg = arg->parent;

	if( arg == 0 || arg->parent == 0 || !asSolIsPlainLValue(arg) || ctx->type.isTemporary )
	{
		Error("A value type passed to '&inout' must be a local variable, a parameter or a property", node);
		return -1;
	}

	// Compile the argument a second time, to be assigned to after the call
	asCExprContext *assignBack = asNEW(asCExprContext)(engine);
	if( assignBack == 0 )
		return -1;
	assignBack->exprNode = arg;
	if( CompileAssignment(arg, assignBack) < 0 )
	{
		asDELETE(assignBack, asCExprContext);
		return -1;
	}

	PrepareTemporaryVariable(node, ctx, false, true);
	ctx->origExpr = assignBack;
	return 0;
}

#endif // AS_NO_COMPILER

#endif // AS_SOL_SAFE_VALUE_INOUT

#ifdef AS_SOL_SIZED_FREE

asSIZEDFREEFUNC_t userFreeSized = 0;

void asCTypeInfo::SolDelete() const
{
	// Each class is told apart by the flags its creator gives it
	size_t size = sizeof(asCTypeInfo);
	if( flags & asOBJ_FUNCDEF )
		size = sizeof(asCFuncdefType);
	else if( flags & asOBJ_TYPEDEF )
		size = sizeof(asCTypedefType);
	else if( flags & asOBJ_ENUM )
		size = sizeof(asCEnumType);
	else if( flags & (asOBJ_VALUE | asOBJ_REF | asOBJ_LIST_PATTERN) )
		size = sizeof(asCObjectType);

	asCTypeInfo *self = const_cast<asCTypeInfo*>(this);
	self->~asCTypeInfo();
	asSolFree(self, size);
}

size_t asSolAllocSize(const asCObjectType *objType)
{
	// Padded as CallAlloc pads it
	asUINT size = objType->size;
	if( size & 0x3 )
		size += 4 - (size & 0x3);
	return size;
}

// The header keeps the block 16 byte aligned, as the allocator gave it
static const size_t asSOL_SIZE_HEADER = 16;

static void *asSolAllocWithSize(size_t size)
{
	asBYTE *mem = asNEWARRAY(asBYTE, size + asSOL_SIZE_HEADER);
	if( mem == 0 )
		return 0;
	*reinterpret_cast<size_t*>(mem) = size;
	return mem + asSOL_SIZE_HEADER;
}

static void asSolFreeWithSize(void *ptr)
{
	if( ptr == 0 )
		return;
	asBYTE *mem = reinterpret_cast<asBYTE*>(ptr) - asSOL_SIZE_HEADER;
	asSolFree(mem, *reinterpret_cast<size_t*>(mem) + asSOL_SIZE_HEADER);
}

void *asSolAllocListBuffer(size_t size)
{
	return asSolAllocWithSize(size);
}

void asSolFreeListBuffer(void *buffer)
{
	asSolFreeWithSize(buffer);
}

void *asSolAllocMem(size_t size)
{
	return asSolAllocWithSize(size);
}

void asSolFreeMemUnsized(void *mem)
{
	asSolFreeWithSize(mem);
}

extern "C"
{

AS_API int asSolSetGlobalMemoryFunctions(asALLOCFUNC_t allocFunc, asSIZEDFREEFUNC_t freeFunc)
{
	if( allocFunc == 0 || freeFunc == 0 )
		return asINVALID_ARG;

	// Clean-up thread local memory first, as asSetGlobalMemoryFunctions does
	asThreadCleanup();

	userAlloc     = allocFunc;
	userFreeSized = freeFunc;
	return asSUCCESS;
}

AS_API void asSolFreeMem(void *mem, size_t size)
{
	if( mem == 0 )
		return;
	asASSERT( *reinterpret_cast<size_t*>(reinterpret_cast<asBYTE*>(mem) - asSOL_SIZE_HEADER) == size );
	UNUSED_VAR(size);
	asSolFreeWithSize(mem);
}

}

#endif // AS_SOL_SIZED_FREE

#ifdef AS_SOL_CYCLE_CREATION_SITE

void asSolGetScriptPosition(int &sectionIdx, int &position)
{
	sectionIdx = -1;
	position   = 0;

	asCContext *ctx = reinterpret_cast<asCContext*>(asGetActiveContext());
	if( ctx == 0 )
		return;

	asUINT stackSize = ctx->GetCallstackSize();
	for( asUINT level = 0; level < stackSize; level++ )
	{
		asCScriptFunction *func;
		asDWORD *bytePos;
		if( level == 0 )
		{
			func    = ctx->m_currentFunction;
			bytePos = ctx->m_regs.programPointer;
		}
		else
		{
			// The frame holds the position after the call, so step back into it
			asPWORD *s = ctx->m_callStack.AddressOf() + (stackSize-level-1)*CALLSTACK_FRAME_SIZE;
			func    = (asCScriptFunction*)s[1];
			bytePos = (asDWORD*)s[2] - 1;
		}

		if( func == 0 || func->scriptData == 0 || func->scriptData->lineNumbers.GetLength() == 0 || bytePos == 0 )
			continue;

		int idx;
		int line = func->GetLineNumber(int(bytePos - func->scriptData->byteCode.AddressOf()), &idx);
		if( (line & 0xFFFFF) == 0 )
			continue;

		sectionIdx = idx;
		position   = line;
		return;
	}
}

int asCGarbageCollector::SolGetCreatedAt(void *obj, int *sectionIdx, int *createdAt)
{
	int r = asINVALID_ARG;

	ENTERCRITICALSECTION(gcCritical);
	for( asUINT n = 0; n < gcNewObjects.GetLength() + gcOldObjects.GetLength(); n++ )
	{
		const asSObjTypePair &o = n < gcNewObjects.GetLength() ? gcNewObjects[n] : gcOldObjects[n - gcNewObjects.GetLength()];
		if( o.obj != obj )
			continue;

		*sectionIdx = o.solCreatedSectionIdx;
		*createdAt  = o.solCreatedAt;
		r = o.solCreatedSectionIdx >= 0 ? asSUCCESS : asNOT_SUPPORTED;
		break;
	}
	LEAVECRITICALSECTION(gcCritical);

	return r;
}

extern "C"
{

AS_API int asSolGetObjectCreatedAt(asIScriptEngine *engine, void *obj, const char **scriptSection, int *row, int *col)
{
	if( scriptSection ) *scriptSection = 0;
	if( row ) *row = 0;
	if( col ) *col = 0;

	asCScriptEngine *e = reinterpret_cast<asCScriptEngine*>(engine);
	if( e == 0 || obj == 0 )
		return asINVALID_ARG;

	int sectionIdx = -1, createdAt = 0;
	int r = e->gc.SolGetCreatedAt(obj, &sectionIdx, &createdAt);
	if( r != asSUCCESS )
		return r;

	if( scriptSection ) *scriptSection = e->scriptSectionNames[sectionIdx]->AddressOf();
	if( row ) *row = createdAt & 0xFFFFF;
	if( col ) *col = (createdAt >> 20) & 0xFFF;
	return asSUCCESS;
}

}

#endif // AS_SOL_CYCLE_CREATION_SITE

#ifdef AS_SOL_GC_REPORTS_DONE

// Anything that can give a step more to do changes one of these
void asCGarbageCollector::SolGetSignature(asUINT *signature) const
{
	signature[0] = numDestroyed;
	signature[1] = numDetected;
	signature[2] = numAdded;
	signature[3] = asUINT(gcOldObjects.GetLength());
}

void asCGarbageCollector::SolStartPass(int step)
{
	SolGetSignature(solPasses[step].start);
}

static bool asSolSameSignature(const asUINT *a, const asUINT *b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

bool asCGarbageCollector::SolIsDone(bool doDestroy, bool doDetect)
{
	asUINT signature[4];
	SolGetSignature(signature);

	// A step that is back in its initial state has just ended a pass, or found its list empty
	bool ended[3] = { destroyNewState == destroyGarbage_init,
	                  destroyOldState == destroyGarbage_init,
	                  detectState == clearCounters_init };

	// A step this call does not run has nothing to do
	bool runs[3] = { doDestroy, doDestroy, doDetect && gcOldObjects.GetLength() > 0 };

	bool done = true;
	for( int n = 0; n < 3; n++ )
	{
		if( !runs[n] )
			continue;

		asSSolPass &pass = solPasses[n];
		if( ended[n] && asSolSameSignature(pass.start, signature) )
		{
			pass.hasQuiet = true;
			for( int s = 0; s < 4; s++ )
				pass.quiet[s] = signature[s];
		}

		if( !pass.hasQuiet || !asSolSameSignature(pass.quiet, signature) )
			done = false;
	}

	// The next call starts over, as the application may have released objects since
	if( done )
		for( int n = 0; n < 3; n++ )
			solPasses[n].hasQuiet = false;

	return done;
}

#endif // AS_SOL_GC_REPORTS_DONE

END_AS_NAMESPACE
