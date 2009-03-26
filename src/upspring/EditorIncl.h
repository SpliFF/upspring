//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef EDITOR_PCH
#define EDITOR_PCH

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef WIN32
// KLOOTNOTE: not supplied with gcc (since not defined by ANSI/ISO C or POSIX)
#include <process.h>
#endif

#include <assert.h>

// STL
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <list>
#include <set>

#include <stdexcept>

#include "creg/creg.h" 
#include "math/Mathlib.h"

// FLTK
#include "Fltk.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
}

#endif
