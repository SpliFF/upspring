%module upspring
%{
#include "EditorIncl.h"
#include "EditorDef.h"

#include "ScriptInterface.h"
#include "../Model.h"
#include "DebugTrace.h"
//#include "../Fltk.h"
%}

%ignore CR_DECLARE;
%ignore CR_DECLARE_STRUCT;
%ignore NO_SCRIPT;

%include "vector.i"
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

// ---------------------------------------------------------------
// Animation reading helpers
// ---------------------------------------------------------------

%inline %{
#define ANIMTYPE_FLOAT 0
#define ANIMTYPE_VECTOR3 1
#define ANIMTYPE_ROTATION 2
#define ANIMTYPE_OTHER 3

int upsAnimGetType(AnimProperty& prop) 
{
	switch(prop.controller->GetType()) {
	case AnimController::ANIMKEY_Float: return ANIMTYPE_FLOAT;
	case AnimController::ANIMKEY_Vector3: return ANIMTYPE_VECTOR3;
	case AnimController::ANIMKEY_Quat: return ANIMTYPE_ROTATION;
	case AnimController::ANIMKEY_Other: return ANIMTYPE_OTHER;
	}
	return -1;
}

int upsAnimGetKeyIndex(AnimProperty& prop, float time) 
{
	return prop.GetKeyIndex(time);
}

int upsAnimGetNumKeys(AnimProperty& prop) 
{
	return prop.NumKeys();
}

float upsAnimGetKeyTime(AnimProperty& prop, int key) 
{
	if(key<0) key=0;
	if(key>=prop.NumKeys()) key=prop.NumKeys()-1;
	return prop.GetKeyTime(key);
}

float upsAnimGetFloatKey(AnimProperty& prop, int key) 
{
	if(key<0) key=0;
	if(key>=prop.NumKeys()) key=prop.NumKeys()-1;
	if (prop.controller->GetType() == AnimController::ANIMKEY_Float)
		return *(float*)prop.GetKeyData(key);
	return 0.0f;
}

Vector3 upsAnimGetVector3Key(AnimProperty& prop, int key) 
{
	if(key<0) key=0;
	if(key>=prop.NumKeys()) key=prop.NumKeys()-1;
	if (prop.controller->GetType() == AnimController::ANIMKEY_Vector3)
		return *(Vector3*)prop.GetKeyData(key);
	return Vector3();
}

Vector3 upsAnimGetRotationKey(AnimProperty& prop, int key) 
{
	if(key<0) key=0;
	if(key>=prop.NumKeys()) key=prop.NumKeys()-1;
	if (prop.controller->GetType() == AnimController::ANIMKEY_Quat) {
		Quaternion q = *(Quaternion*)prop.GetKeyData(key);
		Rotator rot;
		rot.SetQuat(q);
		return rot.GetEuler();
	} else if(prop.controller->GetType() == AnimController::ANIMKEY_Vector3) {
		Vector3 v = *(Vector3*)prop.GetKeyData(key);
		return v;
	}
	return Vector3();
}


void upsAnimInsertVectorKey(AnimProperty& prop, float time, Vector3 v) 
{
	switch(prop.controller->GetType()) {
	case AnimController::ANIMKEY_Vector3:
		prop.InsertKey(&v, time);
		break;
	case AnimController::ANIMKEY_Quat: {
		Rotator rot;
		rot.SetEuler(v);
		Quaternion q = rot.GetQuat();
		prop.InsertKey(&q, time);
		break; }
	}
}

void upsAnimInsertRotatorKey(AnimProperty& prop, float time, Rotator r)
{
	if (prop.controller->GetType() == AnimController::ANIMKEY_Quat) {
		Quaternion q = r.GetQuat();
		prop.InsertKey(&q, time);
	}
	else if (prop.controller->GetType() == AnimController::ANIMKEY_Vector3) {
		Vector3 v = r.GetEuler();
		prop.InsertKey(&v, time);
	}
}

void upsAnimInsertFloatKey(AnimProperty& prop, float time, float val) 
{
	if (prop.controller->GetType() == AnimController::ANIMKEY_Float)
		prop.InsertKey(&val, time);
}



%}

