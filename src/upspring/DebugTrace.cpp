//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"
#include "DebugTrace.h"

#ifdef WIN32
#include <windows.h>
#endif

static char g_logfile[64];

void d_setlogfile (const char *f)
{
	strncpy (g_logfile, f, 64);
}

void d_trace(const char *fmt, ...)
{
	static char buf[256];

	va_list ap;
	va_start(ap,fmt);
	VSNPRINTF (buf,256,fmt,ap);
	va_end(ap);

	d_puts (buf);
}

/*
d_puts - writes a string of any length to the debug output
	(as opposed to d_trace, which is limited by the buffer)
*/
void d_puts (const char *buf)
{
#ifdef WIN32
	OutputDebugString(buf);
#else
	fprintf(stderr, "%s", buf);
#endif

	if(g_logfile[0])
	{
		FILE *p = fopen (g_logfile, "a");
		if(p) {
			fputs(buf, p);
			fclose (p);
		}
	}

#ifdef WIN32
	printf (buf);
#endif
}

void d_clearlog()
{
	remove (g_logfile);
}


void d_assert(char *str, char *file, int line)
{
	d_trace ("Assertion \"%s\" failed at line %d in \'%s\'\n", str, line, file);

#ifdef _DEBUG
	usDebugBreak ();
#endif

	exit (-1);
}
