//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef JC_CONFIG_PARSER_H
#define JC_CONFIG_PARSER_H

#include <string>
#include <list>
#include <stdio.h>

class CfgValue;
class CfgListElem;
class CfgList;
class CfgNumeric;
class CfgLiteral;

struct InputBuffer;


class CfgValueClass
{
public:
	virtual CfgValue* Create () = 0;
	virtual bool Identify (const InputBuffer& buf) = 0; // only lookahead
};

class CfgWriter
{
public:
	CfgWriter(const char *name);
	~CfgWriter();
	bool IsFailed();

	CfgWriter& operator<<(const string& s);
	CfgWriter& operator<<(char c);
	CfgWriter& operator<<(const char *str);

	void MakeIndent (char c);

	void IncIndent();
	void DecIndent();
protected:
	int indentLevel;
	FILE *out;
};

class CfgValue 
{
public:
	CfgValue() {}
	virtual ~CfgValue() {}

	virtual bool Parse (InputBuffer& buf) = 0;
	virtual void dbgPrint (int depth);
	virtual void Write (CfgWriter& w) = 0;

	static char Lookahead (InputBuffer& buf);
	static CfgValue* ParseValue(InputBuffer& buf);
	static CfgList* LoadNestedFile(InputBuffer& buf);
	static CfgList* LoadFile(const char *name);

	static void AddValueClass (CfgValueClass *vc);
protected:
	static vector<CfgValueClass *> classes;
};

class CfgListElem : public CfgValue
{
public:
	CfgListElem() { value=0; }
	~CfgListElem() { if (value) delete value; }
	bool Parse (InputBuffer& buf);
	void Write (CfgWriter& w);

	string name;
	CfgValue *value;
};

// list of cfg nodes
class CfgList : public CfgValue
{
public:
	bool Parse (InputBuffer& buf, bool root);
	bool Parse (InputBuffer& buf) { return Parse (buf, false); }
	void Write (CfgWriter& w) { Write(w,false); }
	void Write (CfgWriter& w, bool root);

	void dbgPrint (int depth);

	CfgValue* GetValue (const char *name);
	double GetNumeric(const char *name, double def=0.0f);
	const char* GetLiteral(const char *name, const char *def=0);
	int GetInt (const char *name, int def) { return GetNumeric (name, def); }

	void AddLiteral (const char *name, const char *val);
	void AddNumeric (const char *name, double val);
	void AddValue (const char *name,CfgValue *val);

	list<CfgListElem> childs;

// Serialization macro's and support
#define CFG_STORE(cfg, val) (cfg).Store(#val, val)
#define CFG_STOREN(cfg, val) (cfg).AddNumeric(#val, val)

	void Store(const char *name, const char *val) { AddLiteral (name,val); }
	void Store(const char *name, string& val) { AddLiteral (name,val.c_str()); }
	void Store(const char *name, CfgValue *val) { AddValue (name,val); }
	void Store(const char *name, bool val) { AddNumeric (name, val?1:0); }

#define CFG_LOAD(cfg, val) (cfg).Load(#val, val)
#define CFG_LOADN(cfg, val) (cfg).LoadNVal(#val, val)

	string& Load(const char *name, string& val) { const char *r = GetLiteral(name); if (r) val=r; return val; }
	CfgValue*& Load(const char *name, CfgValue*& val) { val=GetValue(name); return val; }
	bool& Load(const char* name, bool& val) { val=GetNumeric(name)!=0.0f; return val; }
	template<typename T> T& LoadNVal(const char *name, T& val) { val=(T)GetNumeric(name); return val; }
};

class CfgLiteral : public CfgValue  {
public:
	CfgLiteral () { ident=false; }
	bool Parse (InputBuffer& buf);
	void Write (CfgWriter& w);
	void dbgPrint (int depth);

	bool ident;
	string value;
};

class CfgNumeric : public CfgValue {
public:
	CfgNumeric () { value=0.0; }
	bool Parse (InputBuffer& buf);
	void Write (CfgWriter& w);
	void dbgPrint (int depth);

	double value;
};


#endif
