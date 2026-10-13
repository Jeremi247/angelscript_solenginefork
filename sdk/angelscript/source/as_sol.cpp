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
