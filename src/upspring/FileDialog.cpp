//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"

/*
#ifdef USE_FLTK2_DEFAULT_FILECHOOSER
  #include <fltk/FileChooser.h>
#else
  #ifdef USE_FLTK1_NATIVEFILECHOOSER
    // relative to  ./Fl_Native_File_Chooser/
    #include <FL/Fl_Native_File_Chooser.H>
  #else
    // relative to ./FLNativeFileChooser/
    #include <fltk/NativeFileChooser.h>
  #endif
#endif
*/

#include <fltk/file_chooser.h>


static std::string ConvertPattern(const char* p)
{
	std::string r;
	while (*p) {
		while (*p) 
			r += *(p++);
		r += "\t*.{";
		p++;
		while (*p == ' ')
			p++;
		while (*p)
			r += *(p++);
		r += "}\n";
		p++;
	}

	return r;
}

bool SelectDirectory(const char* msg, std::string& dir)
{
	/*
	#ifdef USE_FLTK2_DEFAULT_FILECHOOSER
		// use FLTK 2.x's own FileChooser (doesn't work)
		fltk::FileChooser fc(dir.c_str(), NULL, fltk::FileChooser::DIRECTORY, msg);

		f.show();
		if (fc.shown() != 0) {
			dir = fc.directory();
			return true;
		}
	#else
		// use the NativeFileChooser wrapper
		// widget for FLTK 1.7 or FLTK 2.x
		#ifdef USE_FLTK1_NATIVEFILECHOOSER
			Fl_Native_File_Chooser fc;
			fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY);
		#else
			fltk::NativeFileChooser fc;
			fc.type(fltk::NativeFileChooser::BROWSE_SAVE_DIRECTORY);
		#endif

		fc.title(msg);
		fc.directory(dir.c_str());

		if (fc.show() == 0) {
			dir = fc.filename(); // no equivalent in FLTK2?
			return true;
		}
	#endif
	*/
	return false;
}

bool FileOpenDlg(const char* msg, const char* pattern, std::string& fn)
{
	std::string convp = ConvertPattern(pattern);
	fltk::use_system_file_chooser(true);

	const char *newfile = fltk::file_chooser(msg, "*", fn.c_str());
	if (newfile != NULL) {
		fn = newfile;
		return true;
	}
	return false;
}

bool FileSaveDlg(const char* msg, const char* ext, std::string& fn)
{
	/*
	std::string convp = ConvertPattern(ext);

	#ifdef USE_FLTK2_DEFAULT_FILECHOOSER
		fltk::FileChooser fc(".", convp.c_str(), fltk::FileChooser::SINGLE, msg);

		fc.show();
		if (fc.shown() != 0) {
			fn = fc.directory();
			return true;
		}
	#else
		#ifdef USE_FLTK1_NATIVEFILECHOOSER
			Fl_Native_File_Chooser fc;
			fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
		#else
			fltk::NativeFileChooser fc;
			fc.type(fltk::NativeFileChooser::BROWSE_SAVE_FILE);
		#endif

		fc.title(msg);
		fc.filter(convp.c_str());
		fc.preset_file(fn.c_str());

		if (fc.show() == 0) {
			fn = fc.filename();
			return true;
		}
	#endif
	*/
	return false;
}
