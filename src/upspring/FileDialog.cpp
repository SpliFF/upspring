//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"

#include <FL/Fl_Native_File_Chooser.H>
/*
#include <windows.h>
static UINT APIENTRY FileDlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NOTIFY) {
		OFNOTIFY *info = (OFNOTIFY*)lParam;
		//switch (info->code) {
		//	case CDN_TYPECHANGE:
		//		info->lpOFN->nFilterIndex 
	}
}

struct FileExtensionType {
	const char *desc, *ext;
};

static char lastFilename[256]="";

vector<FileExtensionType> ConvertWinExtensionList (const char *extlist) 
{
	int num = 0;
	vector<FileExtensionType> list;

	const char *p = extlist;
	while (*p) {
		list.push_back(FileExtensionType());
		list.back().desc = p;
		while (*(p++));
		list.back().ext = p+2; 
		while (*(p++));
	}
	return list;
}

int GetExtensionIndex (const char *filter, const char *fn)
{
	vector<FileExtensionType> types=ConvertWinExtensionList(filter);
	const char *extension=fltk::filename_ext(fn);
	if(extension) {
		for (int a=0;a<types.size();a++) {
			if (!STRCASECMP(types[a].ext, extension+1)) {
				return a;
				break;
			}
		}
	}
	return 0;
}

bool FileOpenDlg (const char *msg, const char *pattern, std::string& fn)
{
	OPENFILENAME ofn;

	memset (&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=GetActiveWindow();
	ofn.lpstrFilter=pattern;
	ofn.lpstrFile=lastFilename;
	ofn.nMaxFile=sizeof(lastFilename);
	ofn.lpstrTitle=msg;
	ofn.Flags = OFN_FILEMUSTEXIST;
	if (lastFilename[0] && pattern) ofn.nFilterIndex = GetExtensionIndex (pattern, lastFilename)+1;

	if(GetOpenFileName(&ofn))
	{
		fn = lastFilename;
		return true;
	}
	return false;
}

bool FileSaveDlg (const char *msg, const char *ext, std::string& fn)
{
	OPENFILENAME ofn;

	memset (&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=GetActiveWindow();
	ofn.lpstrFilter=ext;
	ofn.lpstrFile=lastFilename;
	ofn.nMaxFile=sizeof(lastFilename);
	ofn.lpstrTitle=msg;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	if (lastFilename[0] && ext) ofn.nFilterIndex = GetExtensionIndex (ext, lastFilename)+1;

	if (GetSaveFileName (&ofn)) {
		const char *e = fltk::filename_ext(lastFilename);
		if (!e[0]) {
			// find the extension in the list
			vector<FileExtensionType> extl = ConvertWinExtensionList (ext);
			strncat (lastFilename, extl[ofn.nFilterIndex-1].ext, sizeof(lastFilename));
		}
		fn = lastFilename;

		return true;
	}
	return false;
}
*/

static std::string ConvertPattern(const char *p)
{
	std::string r;
	while (*p)
	{
		while (*p) 
			r+= *(p++);
		r += "\t*.{";
		p++;
		while (*p == ' ')
			p ++;
		while (*p)
			r += *(p++);
		r += "}\n";
		p++;
	}
	return r;
}

bool SelectDirectory(const char *msg, std::string& dir)
{
	Fl_Native_File_Chooser fc;
	fc.title (msg);
	fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY);
	fc.directory(dir.c_str());

	if (fc.show() == 0) {
		dir = fc.filename ();
		return true;
	}

	return false;
}

bool FileOpenDlg (const char *msg, const char *pattern, std::string& fn)
{
	Fl_Native_File_Chooser fc;

	fc.title(msg);
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	std::string convp = ConvertPattern(pattern);
	fc.filter(convp.c_str());
	fc.preset_file(fn.c_str());

	if (fc.show() == 0) {
		fn = fc.filename();
		return true;
	}

	return false;
}

bool FileSaveDlg (const char *msg, const char *ext, std::string& fn)
{
	Fl_Native_File_Chooser fc;
	fc.title (msg);
	fc.type (Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	std::string convp = ConvertPattern(ext);
	fc.filter(convp.c_str());
	fc.preset_file(fn.c_str());

	if (fc.show() == 0) {
		fn = fc.filename();
		return true;
	}
	return false;
}
