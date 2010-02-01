//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef EDITOR_DEF_H
#define EDITOR_DEF_H

#include <algorithm>

#define SAFE_DELETE_ARRAY(c) if(c){delete[] (c);(c)=0;}
#define SAFE_DELETE(c) if(c){delete (c);(c)=0;}


#ifdef _MSC_VER
#define for if(1)for                // MSVC 6 ruuless!
#pragma warning(disable: 4244 4018) // signed/unsigned and loss of precision...
#pragma warning(disable: 4312 4311)
#endif 

using namespace std;

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;


// Most of this file should be skipped if SWIG is being executed,
#ifndef SWIG


/*
 * UPS_HASH_SET defines a hash map template in a portable way.
 * Use #include UPS_HASH_MAP_H to include the header
 */
#ifdef _MSC_VER
	#define UPS_HASH_SET stdext::hash_set
	#define UPS_HASH_SET_H <hash_set>

	#define UPS_HASH_MAP stdext::hash_map
	#define UPS_HASH_MAP_H <hash_map>
#elif defined(__GNUC__)
	#define UPS_HASH_SET __gnu_cxx::hash_set
	#define UPS_HASH_SET_H <ext/hash_set>

	#define UPS_HASH_MAP __gnu_cxx::hash_map
	#define UPS_HASH_MAP_H <ext/hash_map>

	// This is needed as gnu doesn't offer specialization for other pointer types other than char*
	// (look in ext/hash_fun.h for the types supported out of the box)

	#if (__GNUC__ == 4 && __GNUC_MINOR__ > 2)
	#include <backward/hash_fun.h>
	#else
	#include <ext/hash_fun.h>
	#endif

	namespace __gnu_cxx {
		template<> struct hash<void*> {
			size_t operator() (const void* __s) const { return (size_t)(__s); }
		};

// 		template<> struct hash<String> {
// 			size_t operator() (const String& __s) const { return hash<const char*>()(__s.c_str()); }
// 		};
		template<> struct hash<std::string> {
			size_t operator() (const std::string& __s) const { return hash<const char*>()(__s.c_str()); }
		};
	}
#else
	#error Unsupported compiler
#endif

/*
 * Portable definition for SNPRINTF, VSNPRINTF, STRCASECMP and STRNCASECMP
 */
#ifdef _MSC_VER
	#if _MSC_VER > 1310
		#define SNPRINTF _snprintf_s
		#define VSNPRINTF _vsnprintf_s
	#else
		#define SNPRINTF _snprintf
		#define VSNPRINTF _vsnprintf
	#endif
	#define STRCASECMP _stricmp
	#define STRNCASECMP _strnicmp
	#define ALLOCA(size) _alloca(size) // allocates memory on stack
#else
	#define STRCASECMP strcasecmp
	#define STRNCASECMP strncasecmp
	#define SNPRINTF snprintf
	#define VSNPRINTF vsnprintf
	#define ALLOCA(size) alloca(size)
#endif
#define ALLOCA_ARRAY(T, N) (new(ALLOCA(sizeof(T) * N)) T[N])

#endif // !defined(SWIG)

void usDebugBreak ();
void usAssertFailed (const char *condition, const char *file, int line);

class Tool;
struct Model;
class IView;
class Texture;
class TextureHandler;
class TextureGroup;
class TextureGroupHandler;
class EditorViewWindow;
class IK_UI;
class TimelineUI;
class ScriptedMenuItem;
class AnimTrackEditorUI;
class Timer;
class BackupViewerUI;

#include "IEditor.h"

struct ArchiveList {
	bool Load();
	bool Save();

	set<string> archives;
};

class content_error: public std::exception {
	public:
		content_error(const string& s): errMsg(s) {}
		// KLOOTNOTE: g++ demands null-bodies
		~content_error() throw() {};

	string errMsg;
};

inline void stringlwr(string& str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}


bool FileSaveDlg(const char *msg, const char *pattern, string& fn);
bool FileOpenDlg(const char *msg, const char *pattern, string& fn);
bool SelectDirectory(const char *msg, std::string& dir);

extern string applicationPath;


template<typename InputIterator, typename EqualityComparable>
int element_index(InputIterator first, InputIterator last, const EqualityComparable& value) {
	// KLOOTNOTE: g++ craps out on   <type1> i = <val1>, <type2> j = <val2>   style loop-init
	InputIterator i;

	for (int index = 0, i = first; i != last; ++i, ++index)
		// KLOOTNOTE: i is not a pointer so invalid type for *
		// if (*i == value) return index;
		if (i == value) return index;
	return -1;
}

template<typename InputIterator>
InputIterator element_at(InputIterator first, InputIterator last, int index) {
	while (first != last) {
		if (!index--) break;
		++first;
	}
	return first;
}

// Used to mark declarations that should be ignored by SWIG
#define NO_SCRIPT

#define SET_WIDGET_VALUE(w, v) (w)->set_flag(fltk::STATE, v)

static const char ImageFileExt[] =
"All image files\0bmp,tga,pcx,jpg,png,dds,pnm,raw,tif,sgi\0";

#endif

