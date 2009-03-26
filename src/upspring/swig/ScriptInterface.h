
#define SCRIPT_GETNAME 1
#define SCRIPT_SHOWHELP 2
#define SCRIPT_EXEC 3

#ifndef SWIG
class ScriptedMenuItem
{
public:
	std::string funcName, name;
};
#endif

IEditor* upsGetEditor();

void upsAddMenuItem(const char *name, const char *funcname);
