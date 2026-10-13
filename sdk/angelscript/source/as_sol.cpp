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

END_AS_NAMESPACE
