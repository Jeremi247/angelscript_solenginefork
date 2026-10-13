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
// as_sol.h
//
// This file is not part of the original library. It holds what the
// features of the fork maintained for SolEngine add to the internals.
// The interface they add is in angelscript_sol.h.
//


#ifndef AS_SOL_H
#define AS_SOL_H

#include "as_config.h"

BEGIN_AS_NAMESPACE

#ifdef AS_SOL_DECLARATION_POSITIONS

class asCScriptCode;
class asCScriptNode;

// Where a declaration was made, encoded as asCScriptFunction's declaredAt
struct asSSolDeclaredAt
{
	asSSolDeclaredAt() : scriptSectionIdx(-1), declaredAt(0) {}

	int scriptSectionIdx;
	int declaredAt;
};

#ifndef AS_NO_COMPILER
void asSolSetDeclaredAt(int &scriptSectionIdx, int &declaredAt, asCScriptCode *file, asCScriptNode *node);
void asSolSetDeclaredAt(asSSolDeclaredAt &out, asCScriptCode *file, asCScriptNode *node);
#endif

#endif // AS_SOL_DECLARATION_POSITIONS

#ifdef AS_SOL_CYCLE_CREATION_SITE

// The line executing in the active context, in the innermost script function
// that has line numbers, encoded as asCScriptFunction's declaredAt. Without
// an active context the section index is -1.
void asSolGetScriptPosition(int &sectionIdx, int &position);

#endif // AS_SOL_CYCLE_CREATION_SITE

END_AS_NAMESPACE

#endif
