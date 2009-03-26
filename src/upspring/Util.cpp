//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "Util.h"

Logger logger;


// Printf style string formatting
string SPrintf(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	char buf[256];
	VSNPRINTF(buf, sizeof(buf), fmt, vl);
	va_end(vl);

	return buf;
}


string ReadZStr(FILE *f)
{
	std::string s;
	int c,i=0;
	while((c = fgetc(f)) != EOF && c) 
		s += c;
	return s;
}


void WriteZStr(FILE *f, const string& s)
{
	int c;
	c = s.length ();
	fwrite(&s[0],c+1,1,f);
}


string GetFilePath (const string& fn)
{
	string mdlPath = fn;
	string::size_type pos = mdlPath.rfind ('\\');
	string::size_type pos2 = mdlPath.rfind ('/');
	if (pos!=string::npos)
		mdlPath.erase (pos+1, mdlPath.size());
	else if (pos2 != string::npos)
		mdlPath.erase (pos2+1, mdlPath.size());
	return mdlPath;
}

void AddTrailingSlash(std::string& tld)
{
	if (tld.rfind ('/') != tld.length () - 1 && tld.rfind ('\\') != tld.length () - 1)
		tld.push_back ('/');
}


string ReadString (int offset, FILE *f)
{
	int oldofs = ftell(f);
	fseek (f, offset, SEEK_SET);
	string str= ReadZStr (f);
	fseek (f, oldofs, SEEK_SET);
	return str;
}

// ------------------------------------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------------------------------------

void usDebugBreak()
{
#ifdef _DEBUG
	__asm int 3 
#else
	fltk::message ("An error has occured in the program, so it has to abort now.");
#endif
}

void usAssertFailed (const char *condition, const char *file, int line)
{
	d_trace ("%s(%d): Assertion failed (Condition \'%s\')\n", file, line, condition);
	usDebugBreak ();
}

// ------------------------- Logger ------------------------

Logger::Logger ()
{
	filter = NL_DefaultFilter | NL_NoTag;

	numcb = maxcb = 0;
	cb = 0;
}

Logger::~Logger()
{
	filter = NL_DefaultFilter | NL_NoTag;

	if (cb) {
		delete[] cb;
		cb=0;
	}
}

void Logger::AddCallback (CallbackProc proc, void *data)
{
	if (numcb == maxcb)
		Realloc ();

	int c = numcb ++;
	cb[c].proc = proc;
	cb[c].user_data = data;
}

void Logger::Realloc ()
{
	if (maxcb)
		maxcb *= 2;
	else
		maxcb = 8;

	Callback *nc = new Callback [maxcb];
	
	if (cb)
	{
		for (int a=0;a<numcb;a++)
			nc[a] = cb[a];
		delete [] cb;
	}

	cb = nc;
}

void Logger::RemoveCallback (CallbackProc proc, void *data)
{
	for (int a=0;a<numcb;a++)
		if (cb[a].proc == proc && cb[a].user_data == data)
		{
			if (a != numcb-1)
				std::swap (cb[a], cb[numcb-1]);
			numcb --;
		}
}

void Logger::Trace (LogNotifyLevel lev, const char *fmt, ...)
{
	char buf[512];

	if (!(lev & filter) && lev!=NL_NoTag)
		return;

	va_list ap;
	va_start(ap,fmt);
	VSNPRINTF (buf, 512, fmt, ap);
	va_end (ap);

	for (int a=0;a<numcb;a++)
		cb [a].proc (lev,buf, cb[a].user_data);

	PrintBuf (lev, buf);
}

void Logger::PrintBuf (LogNotifyLevel lev, const char *buf)
{
	switch (lev) {
	case NL_Msg: d_puts ("msg: ");break;
	case NL_Debug: d_puts ("dbg: ");break;
	case NL_Error: d_puts ("error: ");break;
	case NL_Warn: d_puts ("warning: ");break;
	}

	d_puts (buf);
}

void Logger::SetDebugMode (bool mask)
{
	if (mask) 
		filter |= NL_Debug;
	else
		filter &= ~NL_Debug;
}

void Logger::Print (const char *fmt,...)
{
	char buf[256];
	va_list ap;
	va_start(ap,fmt);
	VSNPRINTF (buf, 256, fmt, ap);
	va_end (ap);

	for (int a=0;a<numcb;a++)
		cb [a].proc (NL_NoTag,buf, cb[a].user_data);

	PrintBuf (NL_NoTag, buf);
}





//-------------------------------------------------------------------------
// InputBuffer - serves as an input for the config value nodes
//-------------------------------------------------------------------------

void InputBuffer::ShowLocation() const
{
	logger.Trace (NL_Debug, Location().c_str());
}

string InputBuffer::Location() const
{
	return SPrintf("In %s on line %d:", filename, line);
}

bool InputBuffer::CompareIdent (const char *str) const
{
	int i = 0;
	while (str[i] && i+pos < len)
	{
		if (str[i] != data[pos+i])
			return false;

		i++;
	}

	return !str[i];
}



bool InputBuffer::SkipWhitespace ()
{
	// Skip whitespaces and comments
	for(;!end();next())
	{
		if (get() == ' ' || get() == '\t' || get() == '\r' || get() == '\f' || get() == 'v')
			continue;
		if (get() == '\n')
		{
			line ++;
			continue;
		}
		if (get(0) == '/' && get(1) == '/')
		{
			pos += 2;
			while (get() != '\n' && !end())
				next();
			++line;
			continue;
		}
		if (get() == '/' && get(1) == '*')
		{
			pos += 2;
			while (!end())
			{
				if(get(0) == '*' && get(1) == '/')
				{
					pos+=2;
					break;
				}
				if(get() == '\n')
					line++;
				next();
			}
			continue;
		}
		break;
	}

	return end();
}


void InputBuffer::SkipKeyword (const char *kw)
{
	SkipWhitespace();

	int a=0;
	while (!end() && kw[a])
	{
		if (get() != kw[a])
			break;
		next(), ++a;
	}

	if (kw[a])
		throw content_error (SPrintf("%s: Expecting keyword %s\n", Location().c_str(), kw));
}

void InputBuffer::Expecting (const char *s)
{
	ShowLocation();
	logger.Trace (NL_Debug, "Expecting %s\n",  s);
}

string InputBuffer::ParseIdent ()
{
	string name;

	if(isalnum (get()))
	{
		name += get();
		for (next(); isalnum (get()) || get() == '_' || get() == '-'; next())
			name += get();

		return name;
	}
	else 
		throw content_error(SPrintf("%s: Expecting an identifier instead of '%c'\n", Location().c_str(), get()));
}
