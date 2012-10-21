//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "Util.h"

#include "Animation.h"

//-----------------------------------------------------------------------
// Animation controllers for Quaternion,Vector3 and float
//-----------------------------------------------------------------------


class FloatController : public AnimController
{
public:
	int GetSize () { return sizeof(float); }
	void LinearInterp (void *a, void *b, float x,void *out)
	{
		*(float*)out = *(float*)a * (1.0f-x) + *(float*)b * x;
	}
	void Copy (void *src, void *dst) {
		*(float*)dst = *(float*)src;
	}
	bool CanConvertToFloat() { return true; }
	float ToFloat(void *v) { return *(float*)v; }
	AnimKeyType GetType() { return ANIMKEY_Float; }
};

AnimController *AnimController::GetFloatController()
{
	static FloatController fc;
	return &fc;
}


class QuatController : public AnimController
{
public:
	int GetSize () { return sizeof(Quaternion); }
	void LinearInterp (void *a, void *b, float x, void *out) {
		Quaternion* qa = (Quaternion*)a;
		Quaternion* qb = (Quaternion*)b;

		qa->slerp (qb, x, (Quaternion*)out, Quaternion::qshort);
	}
	void Copy (void *src, void *dst) {
		*(Quaternion*)dst = *(Quaternion*)src;
	}

	// Fake euler angles
	template<int Axis>
	class QuatToEulerController : public FloatController
	{
	public:
		float ToFloat(void *v) { 
			Quaternion *q = (Quaternion*)v;
			Matrix m;
			q->makematrix(&m);
			Vector3 euler = m.calcEulerYXZ();
			return euler.v[Axis];
		}
	};

	int GetNumMembers () { return 3; }
	std::pair<AnimController*,void*> GetMemberCtl (int m, void *inst) 
	{ 
		static QuatToEulerController<0> q2e0;
		static QuatToEulerController<1> q2e1;
		static QuatToEulerController<2> q2e2;
		AnimController* actl[] = { &q2e0, &q2e1, &q2e2 };

		return std::pair<AnimController*,void*> (actl[m],inst); 
	}
	const char* GetMemberName(int m)
	{
		const char *axis[] = { "x", "y", "z" };
		return axis[m];
	}
	AnimKeyType GetType() { return ANIMKEY_Quat; }
};

AnimController *AnimController::GetQuaternionController()
{
	static QuatController qc;
	return &qc;
}

class EulerAngleController : public FloatController
{
public:
	void LinearInterp (void *A, void *B, float x, void *out)
	{
		float& a=*(float*)A;
		float& b=*(float*)B;
		float& o=*(float*)out;

		if (a > 2 * M_PI) a -= 2 * M_PI;
		if (a < 0.0f) a += 2 * M_PI;
		if (b > 2 * M_PI) b -= 2 * M_PI;
		if (b < 0.0f) b += 2 * M_PI;

		float v = b-a;
		if (fabsf(v) > M_PI)
		{
			if (v > 0) v -= 2*M_PI;
			else v += 2*M_PI;
		}

		o = a + v * x;
	}
};

AnimController *AnimController::GetEulerAngleController ()
{
	static EulerAngleController eac;
	return &eac;
}

struct StructAnimController : public AnimController
{
	AnimController *subctl;
	creg::Class *class_;

	StructAnimController (AnimController *subctl, creg::Class *class_) :
		subctl(subctl), class_(class_) {}

	int GetSize () { return class_->binder->size; }
	void Copy (void *src, void *dst) { memcpy (dst, src, class_->binder->size); }
	void LinearInterp (void *a, void *b, float x, void *out) {
		for (unsigned int m = 0; m < class_->members.size(); m ++)
		{
			int offset = class_->members[m]->offset;
			subctl->LinearInterp ((char*)a + offset, (char*)b + offset, x, (char*)out + offset);
		}
	}
	int GetNumMembers() { return (int)class_->members.size(); }
	std::pair<AnimController*,void*> GetMemberCtl(int m, void *inst) {
		return std::pair<AnimController*,void*>(subctl, (char*)inst + class_->members[m]->offset);
	}
	const char* GetMemberName (int m) { return class_->members[m]->name; }
	AnimKeyType GetType() { return class_ == Vector3::StaticClass() ? ANIMKEY_Vector3 : ANIMKEY_Other; }
};

AnimController *AnimController::GetStructController (AnimController *subctl, creg::Class *class_)
{
	static vector <StructAnimController> ctls;
	for (unsigned int a=0;a<ctls.size();a++) {
		if (ctls[a].class_ == class_ && ctls[a].subctl == subctl)
			return &ctls[a];
	}
	ctls.push_back (StructAnimController (subctl,class_));
	return &ctls.back();
}

//-----------------------------------------------------------------------
// AnimProperty - holds animation info for an object property
//-----------------------------------------------------------------------

CR_BIND(AnimProperty, ());

CR_REG_METADATA(AnimProperty, (
				CR_MEMBER(keyData),
				CR_MEMBER(name)
				));

AnimProperty::AnimProperty(AnimController *ctl, const std::string& name, int offset) 
	: offset (offset), name (name), controller(ctl)
{
	elemSize = sizeof (float) + controller->GetSize();
}

AnimProperty::AnimProperty ()
{
	controller = 0;
	elemSize = 0;
}

AnimProperty::~AnimProperty ()
{}

int AnimProperty::GetKeyIndex (float time, int *lastkey)
{
	int nk=NumKeys ();
	int a=0;
	if (lastkey && *lastkey>=0) a=*lastkey;
	for (;a<nk;a++) {
		if (GetKeyTime(a) > time) 
			return a-1;
	}
	return NumKeys()-1;
}

void AnimProperty::Evaluate (float time, void *value, int* lastkey)
{
	if (keyData.empty ())
		return;

	int index = GetKeyIndex (time, lastkey);
	if(lastkey) *lastkey=index;

	if (index < 0)
		controller->Copy (GetKeyData(0), value);
	else if (index+1 < NumKeys()) {
		float timeA = GetKeyTime(index);
		float timeB = GetKeyTime(index+1);

		float x = (time - timeA) / (timeB - timeA);
		controller->LinearInterp (GetKeyData(index), GetKeyData(index+1), x, value);
	} else {
		// return the value of a single key
		controller->Copy (GetKeyData(index), value);
	}
}

void AnimProperty::InsertKey (void *data, float time)
{
	int index = GetKeyIndex (time);

	// create a new key or modify an existing one?
	if (keyData.empty() || !(GetKeyTime(index) > time - EPSILON && GetKeyTime(index) < time + EPSILON))
	{
		assert (!keyData.empty() || index==-1);
		keyData.insert(keyData.begin() + elemSize * (index+1), elemSize, 0);

		// set keyframe
		GetKeyTime(index+1) = time;
		controller->Copy (data, GetKeyData (index+1));
	} else
		controller->Copy (data, GetKeyData (index));
}

void AnimProperty::ChopAnimation (float endTime)
{
	for (int k = 0; k < NumKeys (); k++) 
		if (GetKeyTime (k) > endTime) {
			keyData.erase (keyData.begin() + elemSize * k, keyData.end());
			break;
		}
}

AnimProperty* AnimProperty::Clone()
{
	AnimProperty* cp = new AnimProperty();
	
	cp->keyData = keyData;
	cp->controller = controller;
	cp->name = name;
	cp->offset = offset;
	cp->elemSize = elemSize;

	return cp;
}

//-----------------------------------------------------------------------
// AnimationInfo
//-----------------------------------------------------------------------

CR_BIND(AnimationInfo, ());

CR_REG_METADATA(AnimationInfo, CR_SERIALIZER(Serialize));

AnimationInfo::~AnimationInfo()
{
	for (vector<AnimProperty*>::iterator i=properties.begin();i!=properties.end();++i)
		delete *i;
}

// Because properties are only registered by the user object,
// AnimationInfo is not compatible with the automated serialization, 
// so it needs to be serialized manually. 
// The anim properties are saved as if they are embedded in the animation info object
void AnimationInfo::Serialize (creg::ISerializer& s)
{
	if (s.IsWriting ())
	{
		uint size = properties.size();
		s.Serialize (&size,4);

		for (vector<AnimProperty*>::iterator i = properties.begin(); i != properties.end(); ++i)
			s.SerializeObjectInstance (*i, AnimProperty::StaticClass());
	}
	else
	{
		uint size;
		s.Serialize (&size,4);

		for (unsigned int a=0;a<size;a++) {
			AnimProperty prop;
			s.SerializeObjectInstance (&prop, AnimProperty::StaticClass());

			AnimProperty *match = 0;

			// rebind the animation properties to the current set of properties using the property name
			for (vector<AnimProperty*>::iterator i = properties.begin(); i != properties.end(); ++i)
			{
				if ((*i)->name == prop.name)
				{
					match = *i;
					break;
				}
			}
			if (match)
				match->keyData = prop.keyData;
		}
	}
}

void AnimationInfo::AddProperty (AnimController *ctl, const char *name, int offset)
{
	properties.push_back (new AnimProperty(ctl, name, offset));
}

void AnimationInfo::InsertKeyFrames (void *obj, float time)
{
	for (vector<AnimProperty*>::iterator ppi = properties.begin(); ppi != properties.end(); ++ppi)
	{
		bool edited=false;
		AnimProperty *pi = *ppi;
		int size = pi->controller->GetSize();
		char *currentValue = ((char*)obj) + pi->offset;

		if (pi->keyData.empty() || pi->GetKeyTime (0) > time || pi->GetKeyTime (pi->NumKeys()-1) < time)
			edited=true;
		else {
			char *value = new char[size];

			pi->Evaluate (time, value);
			if (memcmp (value, currentValue, size))
			{
				// value is not the same as calculated value, so it is edited and a new key
				// should be added for it
				edited=true;
			}

			delete[] value;
		}

		if (edited)
			pi->InsertKey (currentValue, time);
	}
}

void AnimationInfo::Evaluate (void *obj, float time)
{
	for (vector<AnimProperty*>::iterator pi = properties.begin(); pi != properties.end(); ++pi)
		(*pi)->Evaluate (time, (char*)obj + (*pi)->offset);
}

void AnimationInfo::ClearAnimData()
{
	for (vector<AnimProperty*>::iterator pi = properties.begin(); pi != properties.end(); ++pi)
		(*pi)->Clear();
}


void AnimationInfo::CopyTo(AnimationInfo& animInfo)
{
	// free anim data of the destination object
	for (vector<AnimProperty*>::iterator i=animInfo.properties.begin();i!=animInfo.properties.end();++i)
		delete *i;

	animInfo.properties.resize (properties.size());
	for (uint i = 0; i < properties.size(); i++)
		animInfo.properties [i] = properties[i]->Clone();
}
