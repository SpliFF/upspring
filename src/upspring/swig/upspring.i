%module upspring

#pragma SWIG nowarn=509

%{
#undef SWIG_fail_arg
#define SWIG_fail_arg(func_name,argnum,type) \
  {lua_Debug ar;\
  lua_getstack(L, 1, &ar);\
  lua_getinfo(L, "nSl", &ar);\
  lua_pushfstring(L,"Error (%s:%d) in %s (arg %d), expected '%s' got '%s'",\
  ar.source,ar.currentline,func_name,argnum,type,SWIG_Lua_typename(L,argnum));\
  goto fail;}
%}


%{
#include "EditorIncl.h"
#include "EditorDef.h"
#include "ScriptInterface.h"
#include "../Model.h"
#include "DebugTrace.h"
#include "Texture.h"

#include <iostream>
%}

%ignore CR_DECLARE;
%ignore CR_DECLARE_STRUCT;
%ignore NO_SCRIPT;

%include "std_vector.i"
%rename(cppstring) string;
%include "std_string.i"
%include "list.i"
namespace std{ 
	%template(IntArray) vector<int>; 
	%template(FloatArray) vector<float>;
	%template(CharArray) vector<char>;
	%template(ShortArray) vector<short>;
}

%include "../DebugTrace.h"
%include "../math/Mathlib.h"
%include "../Model.h"
%include "../Animation.h"
%include "../IEditor.h"
%include "ScriptInterface.h"
//%include "../Fltk.h"

%feature("immutable") MdlObject::parent;
%feature("immutable") MdlObject::childs;
%feature("immutable") MdlObject::poly;

%newobject MdlObject::Clone();
%newobject Poly::Clone();

namespace std{
	%template(PolyRefArray) vector<Poly*>;
	%template(VertexArray) vector<Vertex>;
	%template(TriArray) vector<Triangle>;
	%template(ObjectRefArray) vector<MdlObject*>;
	%template(AnimationInfoRefArray) vector<AnimationInfo*>;
	%template(AnimationInfoList) list<AnimationInfo>;
	%template(AnimInfoListIt)  list_iterator<AnimationInfo>;
	%template(AnimInfoListRevIt)  list_reverse_iterator<AnimationInfo>;
	%template(AnimPropertyList) list<AnimProperty>;
	%template(AnimPropListIt) list_iterator<AnimProperty>;
	%template(AnimPropListRevIt) list_reverse_iterator<AnimProperty>;
	%template(AnimPropertyRefArray) vector<AnimProperty*>;
}

namespace fltk{
void message(const char *fmt, ...);
const char* input(const char *label, const char *def);
}

// ---------------------------------------------------------------
// Script util functions
// ---------------------------------------------------------------

%extend Model {
	void SetRoot(MdlObject *o) {
		self->root = o;
	}
}

%extend MdlObject {
	void NewPolyMesh() {
		delete self->geometry;
		self->geometry = new PolyMesh;
	}
}

%inline %{
Model* upsGetModel() { return upsGetEditor()->GetMdl(); }
MdlObject* upsGetRootObj() { return upsGetEditor()->GetMdl()->root; }
void upsUpdateViews() { upsGetEditor()->Update(); }
bool _upsFileSaveDlg (const char *msg, const char *pattern, string& fn) { return FileSaveDlg(msg, pattern, fn); }
bool _upsFileOpenDlg (const char *msg, const char *pattern, string& fn) { return FileOpenDlg(msg, pattern, fn); }
%}

inline %{
#include "../Model.h"

namespace UpsScript {
	TextureHandler *textureHandler = new TextureHandler();


	void LoadArchives() {
		ArchiveList archives;
		archives.Load();

		for (auto it = archives.archives.begin(); it !=archives.archives.end(); ++it) {
			std::cout << "Loading archive: " << it->c_str() << std::endl;
			textureHandler->Load (it->c_str());
		}
	};

	void TexturesToModel(Model *pModel) {
		pModel->root->Load3DOTextures(textureHandler);
	}
}
%}

namespace UpsScript {
	void LoadArchives() {};
	void TexturesToModel(Model *pModel) {};
}