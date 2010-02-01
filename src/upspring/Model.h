//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#ifndef JC_MODEL_H
#define JC_MODEL_H
#include "IView.h"
#include "Animation.h"
#include "VertexBuffer.h"
#include "Referenced.h"
#include "Texture.h"

#define MAPPING_S3O 0
#define MAPPING_3DO 1

namespace csurf { 
	class Face; 
	class Object;
};

using namespace std;

class Texture;

struct Poly;
struct Vertex;
struct MdlObject;
struct Model;
struct IKinfo;
class PolyMesh;

struct Triangle
{
	CR_DECLARE_STRUCT(Triangle);

	Triangle(){vrt[0]=vrt[1]=vrt[2]=0;}
	int vrt[3];
};


struct Vertex
{
	CR_DECLARE_STRUCT(Vertex);

	Vector3 pos, normal;
	Vector2 tc[1];
};

struct Poly
{
	CR_DECLARE(Poly);

	Poly ();
	~Poly ();

	Plane CalcPlane(const vector<Vertex>& verts);
	void Flip();
	Poly* Clone();
	void RotateVerts();

	vector <int> verts;
	string texname;
	Vector3 color;
	int taColor; // TA indexed color
	RefPtr<Texture> texture;

	bool isCurved; // this polygon should get a curved surface at the next csurf update
	bool isSelected;

#ifndef SWIG
	struct Selector : ViewSelector {
		Selector(Poly *poly) : poly(poly),mesh(0) {}
		float Score(Vector3 &pos, float camdis);
		void Toggle (Vector3 &pos, bool bSel);
		bool IsSelected ();
		Poly *poly;
		Matrix transform;
		PolyMesh *mesh;
	};

	Selector *selector;
#endif
};

// Inverse Kinematics joint types - these use the same naming as ODE
enum IKJointType
{
	IKJT_Fixed=0,
	IKJT_Hinge=1,  // rotation around an axis
	IKJT_Universal=2,  // 2 axis
};

struct BaseJoint 
{
	CR_DECLARE(BaseJoint);

	virtual ~BaseJoint() {}
};

struct HingeJoint : public BaseJoint
{
	CR_DECLARE(HingeJoint);

	Vector3 axis;
};

struct UniversalJoint : public BaseJoint
{
	CR_DECLARE(UniversalJoint);

	Vector3 axis[2];
};

struct IKinfo
{
	CR_DECLARE(IKinfo);

	IKinfo();
	~IKinfo();

	IKJointType jointType;
	BaseJoint* joint;
};

struct IRenderData
{
	virtual ~IRenderData() {}
	virtual void Invalidate () = 0;
};

class Rotator
{
public:
	CR_DECLARE(Rotator);

	Rotator();
	void AddEulerAbsolute(const Vector3& rot);
	void AddEulerRelative(const Vector3& rot);
	Vector3 GetEuler();
	void SetEuler(Vector3 euler);
	void ToMatrix(Matrix& o);
	void FromMatrix(const Matrix& r);
	Quaternion GetQuat();
	void SetQuat(Quaternion q);

	Vector3 euler;
//	Quaternion q;
	bool eulerInterp; // Use euler angles to interpolate
};

class PolyMesh;
class ModelDrawer;

class Geometry
{
public:
	CR_DECLARE(Geometry);

	virtual ~Geometry() {}
	
	virtual void Draw(ModelDrawer *drawer, Model* mdl, MdlObject* o) = 0;
	virtual Geometry* Clone() = 0;
	virtual void Transform(const Matrix& transform) = 0;
	virtual PolyMesh* ToPolyMesh() = 0;
	virtual void InvalidateRenderData() {}

	virtual void CalculateRadius(float& radius, const Matrix &tr, const Vector3& mid) = 0;
};

class PolyMesh : public Geometry
{
public:
	CR_DECLARE(PolyMesh);

	~PolyMesh();

	vector <Vertex> verts;
	vector <Poly*> poly;

	void Draw(ModelDrawer* drawer, Model* mdl, MdlObject* o);
	Geometry* Clone();
	void Transform(const Matrix& transform);
	PolyMesh* ToPolyMesh(); // clones
	vector<Triangle> MakeTris();

	static bool IsEqualVertexTC(Vertex& a, Vertex& b);
	static bool IsEqualVertexTCNormal(Vertex& a, Vertex& b);
	typedef bool (*IsEqualVertexCB)(Vertex& a, Vertex& b);

	void OptimizeVertices(IsEqualVertexCB cb);
	void Optimize(IsEqualVertexCB cb);

	void MoveGeometry(PolyMesh *dst);
	void FlipPolygons(); // flip polygons of object and child objects
	void CalculateRadius (float& radius, const Matrix &tr, const Vector3& mid);
	void CalculateNormals ();
	void CalculateNormals2 (float maxSmoothAngle);
};


struct MdlObject {
	CR_DECLARE(MdlObject);

	MdlObject ();
	virtual ~MdlObject ();

	bool IsEmpty ();
	void Dump (int r=0);
	void MergeChild (MdlObject *ch);
	void FullMerge (); // merge all childs and their subchilds
	void GetTransform (Matrix& tr); // calculates the object space -> parent space transform
	void GetFullTransform (Matrix& tr); // object space -> world space
	vector<MdlObject*> GetChildObjects (); // returns all child objects (recursively)

	void UnlinkFromParent ();
	void LinkToParent (MdlObject *p);

	template<typename Fn>
	void ApplyPolyMeshOperationR(Fn f)
	{
		PolyMesh *pm = GetPolyMesh();
		if (pm) (pm->*f)();
		for (uint a=0;a<childs.size();a++)
			childs[a]->ApplyPolyMeshOperationR(f);
	}

	void FlipPolygons();

	void Load3DOTextures(TextureHandler *th);

	bool HasSelectedParent ();
	
	void ApplyTransform (bool rotation, bool scaling, bool position);
	void ApplyParentSpaceTransform(const Matrix& transform);
	void TransformVertices (const Matrix& transform);
	void ApproximateOffset ();
	void SetPropertiesFromMatrix(Matrix& transform);
	// Apply transform to contents of object, 
	// does not touch the properties such as position/scale/rotation
	void Transform(const Matrix& matrix);

	void InvalidateRenderData ();
	void NormalizeNormals ();

	void MoveOrigin(Vector3 move);

	// Simple script util functions, useful for other code as well
	void AddChild(MdlObject *o);
	void RemoveChild(MdlObject *o);

	PolyMesh* GetPolyMesh();
	PolyMesh* ToPolyMesh() { return geometry ? geometry->ToPolyMesh() : 0; } // returns a new PolyMesh
	PolyMesh* GetOrCreatePolyMesh();

	virtual void InitAnimationInfo ();
	void UpdateAnimation (float time);

	MdlObject *Clone();

	// Orientation
    Vector3 position;
	Rotator rotation;
	Vector3 scale;

	Geometry* geometry;

	AnimationInfo animInfo;

	string name;
	bool isSelected;
	bool isOpen; // childs visible in object browser
	IKinfo ikInfo;

	MdlObject *parent;
	vector <MdlObject*> childs;

#ifndef SWIG
	csurf::Object* csurfobj;

	struct Selector : ViewSelector
	{
		Selector(MdlObject *obj) : obj(obj) {}
		// Is pos contained by this object?
		float Score (Vector3 &pos, float camdis);
		void Toggle (Vector3 &pos, bool bSel);
		bool IsSelected ();
		MdlObject *obj;
	};
	Selector *selector;
	bool bTexturesLoaded;
#endif
};

static inline void IterateObjects(MdlObject *obj, void (*fn)(MdlObject *obj))
{
	fn (obj);
	for (unsigned int a=0;a<obj->childs.size();a++)
		IterateObjects (obj->childs[a], fn);
}

// allows a GUI component to plug in and show the progress
struct IProgressCtl {
	IProgressCtl() { data=0; cb=0; }
	virtual void Update(float v) { if (cb) cb(v, data); }
	void (*cb)(float part, void *data);
	void *data;
};

struct TextureBinding
{
	CR_DECLARE(TextureBinding);

	std::string name;
#ifndef SWIG
	RefPtr<Texture> texture;
#endif
	// swig helpers
	void SetTexture(Texture *t) { texture = t; }
	Texture* GetTexture() { return texture.Get(); }
};


// KLOOTNOTE: g++ disallows references to temporary objects so...
#define HACK_CAST (IProgressCtl&) (const IProgressCtl&)

struct Model {
	CR_DECLARE(Model);

	Model();
	~Model();

	void PostLoad();


	bool Load3DO(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());
	bool Save3DO(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());

	bool LoadS3O(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());
	bool SaveS3O(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());

	// Common 3D MdlObject (chunk based format)
	bool LoadC3O(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());
	bool SaveC3O(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());

	// Memory dump using creg
	static Model* LoadOPK(const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());
	static bool SaveOPK(Model *mdl, const char *filename, IProgressCtl& progctl = HACK_CAST IProgressCtl());


	static Model* Load(const string& fn, bool Optimize=true, IProgressCtl& progctl = HACK_CAST IProgressCtl());
	static bool Save(Model *mdl, const string& fn, IProgressCtl& progctl = HACK_CAST IProgressCtl());


	// exports merged version of the model
	bool ExportUVMesh(const char *fn);

	// copies the UV coords of the single piece exported by ExportUVMesh back to the model
	bool ImportUVMesh(const char *fn, IProgressCtl& progctl = HACK_CAST IProgressCtl());
	bool ImportUVCoords(Model* other, IProgressCtl& progctl = HACK_CAST IProgressCtl());


	void InsertModel(MdlObject *obj, Model *sub);
	vector<MdlObject*> GetSelectedObjects();
	vector<MdlObject*> GetObjectList(); // returns all objects
	vector<PolyMesh*> GetPolyMeshList();
	void DeleteObject(MdlObject *obj);
	void ReplaceObject(MdlObject *oldObj, MdlObject *newObj);
	void EstimateMidPosition();
	void CalculateRadius();
	void SwapObjects(MdlObject *a, MdlObject *b);
	Model* Clone();

	ulong ObjectSelectionHash();

	void SetTextureName(uint index, const char *name);
	void SetTexture(uint index, Texture* tex);

	bool ConvertToS3O(std::string texName, int texw, int texh);

	float radius;		//radius of collision sphere
	float height;		//height of whole object
	Vector3 mid;//these give the offset from origin(which is supposed to lay in the ground plane) to the middle of the unit collision sphere

	bool HasTex(uint i) { return i<texBindings.size() && texBindings[i].texture; }
	uint TextureID(uint i) { return texBindings[i].texture->glIdent; }
	std::string& TextureName(uint i) { return texBindings[i].name; }

	std::vector<TextureBinding> texBindings;
	int mapping;

	MdlObject *root;

private:
	Model(const Model& /*c*/) {}
	void operator=(const Model& /*c*/) {}
};



MdlObject* Load3DSObject(const char *filename, IProgressCtl& progctl);
bool Save3DSObject(const char *filename, MdlObject *obj, IProgressCtl& progctl);
MdlObject* LoadWavefrontObject(const char *fn, IProgressCtl& progctl);
bool SaveWavefrontObject(const char *fn, MdlObject *obj, IProgressCtl& progctl);

void GenerateUniqueVectors(const std::vector<Vertex>& verts,
						   std::vector<Vector3>& vertPos,
						   std::vector<int>& old2new);

#endif
