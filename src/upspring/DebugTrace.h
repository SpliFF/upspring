//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef ES_CORE_TRACE_H
#define ES_CORE_TRACE_H

void d_trace(const char *fmt,...);	// these will only be seen in the debug output
void d_puts(const char *str);
void d_assert(char *str, char *file, int line);
void d_clearlog();
void d_setlogfile (const char *f);


#endif
