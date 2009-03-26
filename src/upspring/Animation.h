
#ifndef SWIG

class AnimController
{
public:
	virtual void LinearInterp (void *a, void *b, float x, void *out) = 0;
	virtual int GetSize () = 0;
	virtual void Copy (void *src, void *dst) = 0;
	virtual float ToFloat(void *v) {return 0.0f; }
	virtual bool CanConvertToFloat() { return false; }

	virtual int GetNumMembers () { return 0; }
	virtual std::pair<AnimController*,void*> GetMemberCtl (int m, void *inst) { return std::pair<AnimController*,void*> (0,0); }
	virtual const char* GetMemberName(int m) { return 0; }
	
	// upsprings lua script has support for a fixed set of types
	enum AnimKeyType {
		ANIMKEY_Float,
		ANIMKEY_Vector3,
		ANIMKEY_Quat,
		ANIMKEY_Other // any other struct
	};
	virtual AnimKeyType GetType() = 0;

	static AnimController *GetQuaternionController();
	static AnimController *GetFloatController();
	static AnimController *GetEulerAngleController();
	// Create a struct controller that applies subctl on all it's members
	// All struct members need to be compatible with the given subctl for this
	static AnimController *GetStructController(AnimController* subctl, creg::Class *def);
};
#endif

class AnimProperty
{
public:
	CR_DECLARE(AnimProperty);

	AnimProperty();
	AnimProperty(AnimController *ctl, const std::string& name, int offset);
	~AnimProperty();

	int GetKeyIndex (float time, int *lastkey=0);
	void Evaluate (float time, void *value, int *lastkey=0);
	void InsertKey (void *data, float time);
	void ChopAnimation (float endTime);// chop off all animation past endTime

	float* GetKeyData (int index) { return (float*)&keyData[elemSize * index + sizeof(float)]; }
	int NumKeys () { return (int)keyData.size()/elemSize; }
	float& GetKeyTime (int index) { return *(float*)&keyData[elemSize * index]; }
	void SetKeyTime(int index, float t) { GetKeyTime(index) = t; }
	const char *GetName() { return name.c_str(); }
	void Clear() { keyData.clear(); }
	AnimProperty* Clone();

	std::vector <char> keyData;
	int offset, elemSize;
	std::string name;
#ifndef SWIG
	AnimController *controller;
#endif
};

// Contains animation data(keyframes) for a particular object
class AnimationInfo
{
public:
	CR_DECLARE(AnimationInfo);
	
	~AnimationInfo();
	void Serialize(creg::ISerializer& s);

	void AddProperty (AnimController *controller, const char *name, int offset);

	// Calculate a state of the properties at a specific time
	void Evaluate (void *obj, float time);

	// Insert keyframes for modified properties
	void InsertKeyFrames (void *obj, float time);

	void ClearAnimData();
	void CopyTo(AnimationInfo& dst);

/**
 * every object with AnimationInfo can have a number of AnimProperty's 
 * attached to it's own object properties.
 * The AnimProperty contains keyframes for a real object property
 */
	std::vector<AnimProperty*> properties;
};

// An animation sequence contains all animation data for a set of objects
struct AnimationSequence
{
	std::string name;
	std::vector <AnimationInfo *> objects;
};
