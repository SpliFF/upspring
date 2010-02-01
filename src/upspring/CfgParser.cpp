//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"
#include "CfgParser.h"
#include "Util.h"


// this keeps a list of implemented value types
vector<CfgValueClass*> CfgValue::classes;



//-------------------------------------------------------------------------
// CfgWriter - "outputdevice" for the config value nodes
//-------------------------------------------------------------------------

CfgWriter::CfgWriter (const char *name)
{
	indentLevel =0;

	out = fopen (name, "w");
}

CfgWriter::~CfgWriter() 
{
	if (out) {
		fclose (out);
		out = 0;
	}
}

bool CfgWriter::IsFailed() { return out == 0 || ferror(out); }

void CfgWriter::DecIndent () { indentLevel--; }
void CfgWriter::IncIndent () { indentLevel++; }

CfgWriter& CfgWriter::operator <<(char c)
{
	fputc (c,out);
	MakeIndent(c);
	return *this;
}

CfgWriter&  CfgWriter::operator<<(const string& s)
{
	fputs (s.c_str(),out);
	if (!s.empty()) MakeIndent(s.at(s.size()-1));
	return *this;
}

CfgWriter& CfgWriter::operator<<(const char* str)
{
	fputs (str,out);
	int l = strlen(str);
	if (l) MakeIndent (str[l-1]);
	return *this;
}

void CfgWriter::MakeIndent(char c)
{
	if (c != 0x0D && c != 0x0A) return;

	for (int a=0;a<indentLevel;a++)
		fputs("  ",out);
}

//-------------------------------------------------------------------------
// CfgValue - base config parsing class
//-------------------------------------------------------------------------

void CfgValue::dbgPrint (int depth)
{
	logger.Trace (NL_Debug, "??\n");
}

void CfgValue::AddValueClass (CfgValueClass *vc)
{
	if (find(classes.begin(), classes.end(), vc) == classes.end())
		classes.push_back (vc);
}

CfgValue* CfgValue::ParseValue (InputBuffer& buf)
{
	CfgValue *v = 0;

	if (buf.SkipWhitespace ()) {
		buf.Expecting ("Value");
		return 0;
	}

	for (unsigned int a=0;a<classes.size();a++)
	{
		if (classes[a]->Identify (buf))
		{
			v = classes[a]->Create ();

			if (!v->Parse (buf)) {
				delete v;
				return 0;
			}

			return v;
		}
	}

	// parse standard value types:
	char r = Lookahead (buf);

	if(buf.CompareIdent ("file"))
	{
		// load a nested config file
		return LoadNestedFile (buf);
	}
	else if(isalpha (*buf)) {
		v = new CfgLiteral;
		((CfgLiteral*)v)->ident = true;
	}
	else if(isdigit (r) || *buf == '.' || *buf == '-')
		v = new CfgNumeric;
	else if(*buf == '"')
		v = new CfgLiteral;
	else if(*buf == '{')
		v = new CfgList;

	if (v && !v->Parse (buf))
	{
		delete v;
		return 0;
	}

	return v;
}

void CfgValue::Write (CfgWriter& w) {}

CfgList* CfgValue::LoadNestedFile (InputBuffer& buf)
{
	string s;
	buf.SkipKeyword ("file");

	buf.SkipWhitespace ();

	CfgLiteral l;
	if (!l.Parse (buf))
		return 0;
	s = l.value;

	// insert the path of the current file in the string
	int i = strlen (buf.filename)-1;
	while (i > 0) {
		if (buf.filename [i]=='\\' || buf.filename [i] == '/')
			break;
		i--;
	}

	s.insert (s.begin(), buf.filename, buf.filename + i + 1);
	return LoadFile (s.c_str());
}

CfgList* CfgValue::LoadFile (const char *name)
{
	InputBuffer buf;

	FILE *f = fopen (name, "rb");
	if (!f) {
		logger.Trace (NL_Debug, "Failed to open file %s\n", name);
		return 0;
	}

	fseek (f, 0, SEEK_END);
	buf.len = ftell(f);
	buf.data = new char [buf.len];
	fseek (f, 0, SEEK_SET);
	if (!fread (buf.data, buf.len, 1, f))
	{
		logger.Trace (NL_Debug, "Failed to read file %s\n", name);
		fclose (f);
		delete[] buf.data;
		return 0;
	}
	buf.filename = name;

	fclose (f);

	CfgList *nlist = new CfgList;
	if (!nlist->Parse (buf,true))
	{
		delete nlist;
		delete[] buf.data;
		return 0;
	}

	delete[] buf.data;
    return nlist;
}


char CfgValue::Lookahead (InputBuffer& buf)
{
	InputBuffer cp = buf;

	if (!cp.SkipWhitespace ())
		return *cp;

	return 0;
}

//-------------------------------------------------------------------------
// CfgNumeric - parses int/float/double
//-------------------------------------------------------------------------

bool CfgNumeric::Parse (InputBuffer& buf)
{
	bool dot=*buf=='.';
	string str;
	str+=*buf;
	++buf;
	while (1) {
		if(*buf=='.') {
			if(dot) break;
			else dot=true;
		}
		if(!(isdigit(*buf) || *buf=='.'))
			break;
		str += *buf;
		++buf;
	}
	if(dot) value = atof (str.c_str());
	else value = atoi (str.c_str());
	return true;
}

void CfgNumeric::dbgPrint (int depth)
{
	logger.Trace (NL_Debug, "%g\n", value);
}

void CfgNumeric::Write (CfgWriter &w)
{
	CfgValue::Write (w);

	char tmp[20];
	SNPRINTF(tmp,20,"%g", value);
	w << tmp;
}

//-------------------------------------------------------------------------
// CfgLiteral - parses string constants
//-------------------------------------------------------------------------
bool CfgLiteral::Parse (InputBuffer& buf)
{
	if (ident) {
		value = buf.ParseIdent ();
		return true;
	}

	++buf;
	while (*buf != '\n')
	{
		if(*buf == '\\')
			if(buf[1] == '"') {
				value += buf[1];
				buf.pos += 2;
				continue;
			}

		if(*buf == '"')
			break;

		value += *buf;
		++buf;
	}
	++buf;
	return true;
}

void CfgLiteral::dbgPrint (int depth)
{
	logger.Trace (NL_Debug, "%s\n", value.c_str());
}

void CfgLiteral::Write (CfgWriter& w)
{
	if (ident)
		w << value;
	else
		w << '"' << value << '"';
}

//-------------------------------------------------------------------------
// CfgList & CfgListElem - parses a list of values enclosed with { }
//-------------------------------------------------------------------------

bool CfgListElem::Parse (InputBuffer& buf)
{
	if (buf.SkipWhitespace ())
	{
		logger.Trace (NL_Debug, "%d: Unexpected end of file in list element\n", buf.line);
		return false;
	}

	// parse name
	name = buf.ParseIdent();
	buf.SkipWhitespace ();

	if (*buf == '=') {
		++buf;

		value = ParseValue (buf);

		return value!=0;
	} else
		return true;
}

void CfgListElem::Write (CfgWriter& w)
{
	w << name;

	if (value) {
		w << " = ";
		value->Write (w);
	}
	w << "\n";
}

bool CfgList::Parse (InputBuffer& buf, bool root)
{
	if (!root)
	{
		buf.SkipWhitespace ();

		if (*buf != '{') {
			buf.Expecting ("{");
			return false;
		}

		++buf;
	}

	while (!buf.SkipWhitespace ())
	{
		if (*buf == '}') {
			++buf;
			return true;
		}

		childs.push_back (CfgListElem());
		if (!childs.back ().Parse (buf))
			return false;
	}

	if (!root && buf.end())
	{
		buf.ShowLocation (), logger.Trace (NL_Debug, "Unexpected end of node at line\n");
		return false;
	}

	return true;
}

void CfgList::Write (CfgWriter &w, bool root)
{
	if (!root) {
		w << '{';
		w.IncIndent (); w << "\n";
	}
	for(list<CfgListElem>::iterator i = childs.begin(); i != childs.end();++i)
		i->Write (w);
	if (!root) {
		w << '}';
		w.DecIndent (); w << "\n";
	}
}

CfgValue* CfgList::GetValue (const char *name)
{
	for (list<CfgListElem>::iterator i = childs.begin();i != childs.end(); ++i)
		if (!STRCASECMP (i->name.c_str(), name))
			return i->value;
	return 0;
}

double CfgList::GetNumeric (const char *name, double def)
{
	CfgValue *v = GetValue (name);
	if (!v) return def;

	CfgNumeric *n = dynamic_cast<CfgNumeric*>(v);
	if (!n) return def;

	return n->value;
}

const char* CfgList::GetLiteral (const char *name, const char *def)
{
	CfgValue *v = GetValue (name);
	if (!v) return def;

	CfgLiteral *n = dynamic_cast<CfgLiteral*>(v);
	if (!n) return def;

	return n->value.c_str();
}

void CfgList::dbgPrint(int depth)
{
	int n = 0;
	if (depth) 
		logger.Trace (NL_Debug, "list of %d elements\n", childs.size());

	for (list<CfgListElem>::iterator i = childs.begin(); i != childs.end(); ++i)
	{
		for (int a=0;a<depth;a++) logger.Trace (NL_Debug, "  ");
		logger.Trace (NL_Debug, "List element(%d): %s", n++, i->name.c_str());
		if (i->value) {
			logger.Trace (NL_Debug, " = ");
			i->value->dbgPrint (depth+1);
		} else
			logger.Trace (NL_Debug, "\n");
	}
}

void CfgList::AddLiteral (const char *name, const char *val)
{
	CfgLiteral *l=new CfgLiteral;
	l->value = val;
	childs.push_back(CfgListElem());
	childs.back().value=l;
	childs.back().name=name;
}

void CfgList::AddNumeric (const char *name, double val)
{
	CfgNumeric *n=new CfgNumeric;
	n->value=val;
	childs.push_back(CfgListElem());
	childs.back().value=n;
	childs.back().name=name;
}

void CfgList::AddValue (const char *name,CfgValue *val)
{
	childs.push_back(CfgListElem());
	childs.back().value=val;
	childs.back().name=name;
}


