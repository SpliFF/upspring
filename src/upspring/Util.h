//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "DebugTrace.h"

// ------------------------------------------------------------------------------------------------
// Text Output Interface
// ------------------------------------------------------------------------------------------------
class esTextOutput
{
public:
	virtual void Print (const char *fmt, ...) = 0;
};

// ------------------------------------------------------------------------------------------------
// Text Input Interface
// ------------------------------------------------------------------------------------------------
class esTextInput
{
public:
	virtual bool Read (char *dst, int num) = 0; // returns the number of bytes readed, 0 on EOF
};

// ------------------------------------------------------------------------------------------------
// Global logging device
// ------------------------------------------------------------------------------------------------
enum LogNotifyLevel
{
	NL_Msg	= 1, // normal msg
	NL_Debug= 2, // debug msg
	NL_Warn = 4,
	NL_Error= 8,
	NL_NoTag=16,
	NL_DefaultFilter = NL_Warn | NL_Error,
	NL_DebugFilter = NL_Debug | NL_Warn | NL_Msg | NL_Error
};

class Logger : public esTextOutput
{
public:
	Logger ();
	~Logger ();

	void PrintBuf (LogNotifyLevel lev, const char *buf);
	void Trace (LogNotifyLevel lev, const char *fmt, ...);
	void Print (const char *fmt, ...); // LogNotifyLevel is NL_Msg here

	typedef void (*CallbackProc)(LogNotifyLevel level, const char *str, void *user_data);
	void AddCallback (CallbackProc proc, void *user_data);
	void RemoveCallback (CallbackProc proc, void *user_data);

	void SetDebugMode (bool enable);

	unsigned int filter; // LogNotifyLevel bitmask
protected:
	void Realloc ();

	int numcb, maxcb;
	struct Callback
	{
		CallbackProc proc;
		void *user_data;
	};
	Callback *cb;
};

extern Logger logger;


struct InputBuffer
{
	InputBuffer () : pos(0), len(0), line(1), data(0), filename(0) {}
	char operator*() const { return data[pos]; }
	bool end() const {  return pos == len; }
	InputBuffer& operator++() { next(); return *this; }
	InputBuffer operator++(int) { InputBuffer t=*this; next(); return t; }
	char operator[](int i)const { return data[pos+i]; }
	void ShowLocation() const;
	bool CompareIdent (const char *str) const;
	char get() const { return data[pos]; }
	char get(int i) const { return data[pos+i]; }
	void next() { if (pos<len) pos++; }
	bool SkipWhitespace();
	std::string Location() const;
	void SkipKeyword(const char *s);
	void Expecting(const char *s); // show an error message saying that 's' was expected
	std::string ParseIdent();

	int pos;
	int len;
	int line;
	char *data;
	const char *filename;
};


std::string ReadString (int offset, FILE *f);
std::string ReadZStr (FILE*f);
void WriteZStr (FILE *f, const std::string& s);
std::string GetFilePath (const std::string& fn);
void AddTrailingSlash(std::string& tld);


std::string SPrintf(const char *fmt, ...);
