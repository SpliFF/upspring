// Doom3 MD5 import/export

#include "EditorIncl.h"
#include "EditorDef.h"

#include "Model.h"
#include "Util.h"


Model* ReadMD5Model (const char *filename)
{
	Model *mdl = new Model();

	FILE *file = fopen(filename, "rb");
	if(!file) {
		fltk::message( "Can't open file %s", filename);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	long len = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buffer = new char[len];
	fread(buffer, 1, len, file);
	fclose(file);

	InputBuffer ibuf;
	ibuf.data = buffer;
	ibuf.len = len;
	ibuf.filename = filename;

	ibuf.SkipWhitespace ();
	try {
		ibuf.SkipKeyword("MD5Version");
		ibuf.SkipKeyword("10");
	} catch (const content_error& e) {
		fltk::message("Parse error: %s", e.errMsg.c_str());
		return 0;
	}

	delete[] buffer;

	return mdl;
}
