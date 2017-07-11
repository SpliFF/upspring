//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "Util.h"
#include "Model.h"
#include "View.h"
#include "Texture.h"

#include "creg/Serializer.h"
#include "creg/VarTypes.h"
#include <fstream>
#include <stdexcept>

#include "CurvedSurface.h"

#include "MeshIterators.h"

// ------------------------------------------------------------------------------------------------
// Register model types
// ------------------------------------------------------------------------------------------------

// Simple structures
CR_BIND(Vertex,());
CR_REG_METADATA(Vertex, 
(
	CR_MEMBER(pos), 
	CR_MEMBER(normal), 
	CR_MEMBER(tc))
);

CR_BIND(Triangle,());
CR_REG_METADATA(Triangle, (CR_MEMBER(vrt)));

// Joints
CR_BIND(BaseJoint,());

CR_BIND_DERIVED(HingeJoint, BaseJoint,());
CR_REG_METADATA(HingeJoint, CR_MEMBER(axis));

CR_BIND_DERIVED(UniversalJoint, BaseJoint,());
CR_REG_METADATA(UniversalJoint, CR_MEMBER(axis));

// IK info
CR_BIND(IKinfo,());
CR_REG_METADATA(IKinfo, (CR_MEMBER(joint), CR_ENUM_MEMBER(jointType)));

// Poly
CR_BIND(Poly,());
CR_REG_METADATA(Poly, (
	CR_MEMBER(verts), 
	CR_MEMBER(texname), 
	CR_MEMBER(color), 
	CR_MEMBER(taColor),
	CR_MEMBER(texture),
	CR_MEMBER_SETFLAG(texture, CM_NoSerialize), // the texture shouldn't be serialized
	CR_MEMBER(isSelected))
);

CR_BIND(Rotator,());
CR_REG_METADATA(Rotator, 
(
	CR_MEMBER(euler),
	CR_MEMBER(eulerInterp))
);

CR_BIND_INTERFACE(Geometry);

CR_BIND_DERIVED(PolyMesh, Geometry, ())
CR_REG_METADATA(PolyMesh,
(
	CR_MEMBER(poly),
	CR_MEMBER(verts))
);


// MdlObject
CR_BIND(MdlObject, ());
CR_REG_METADATA(MdlObject,
(
	CR_MEMBER(childs),
	CR_MEMBER(geometry),

	// Orientation
	CR_MEMBER(position),
	CR_MEMBER(scale),
	CR_MEMBER(rotation),

	CR_MEMBER(name),
	CR_MEMBER(isSelected),
	CR_MEMBER(isOpen),
	CR_MEMBER(parent),
	CR_MEMBER(ikInfo),

	CR_MEMBER(animInfo)
))

// Model
CR_BIND(Model, ());
CR_REG_METADATA(Model,
(
	CR_MEMBER(radius),
	CR_MEMBER(height),
	CR_MEMBER(mid),

	CR_MEMBER(mapping),
	CR_MEMBER(texBindings),
	CR_MEMBER(root),
	CR_POSTLOAD(PostLoad)
))

CR_BIND(TextureBinding, ())
CR_REG_METADATA(TextureBinding, (
	CR_MEMBER(name),
	CR_MEMBER(texture),
	CR_MEMBER_SETFLAG(texture,CM_NoSerialize)  // only texture names are stored
	))

// ------------------------------------------------------------------------------------------------
// Model
// ------------------------------------------------------------------------------------------------

Model::Model ()
{
	root=0;
	mapping=MAPPING_S3O;
	height=radius=0.0f;
}

Model::~Model()
{
	if (root) 
		delete root;
}

void Model::SetTextureName(uint index, const char *name)
{
	if (texBindings.size () <= index)
		texBindings.resize (index+1);

	texBindings[index].name = name;
}

void Model::SetTexture(uint index, Texture *tex)
{
	if (texBindings.size () <= index)
		texBindings.resize (index+1);

	texBindings[index].texture = tex;
	texBindings[index].name = tex ? tex->name : "";
}


void Model::InsertModel (MdlObject *obj, Model *sub)
{
	if(!sub->root)
		return;

	obj->childs.push_back (sub->root);
	sub->root->parent = obj;
	sub->root = 0;
}


// TODO: Abstract file formats
Model* Model::Load(const string& _fn, bool /*Optimize*/, IProgressCtl& progctl) {
	const char *fn = _fn.c_str();
	const char *ext=fltk::filename_ext(fn);
	Model *mdl = 0;

	try {
		if (!STRCASECMP(ext, ".opk"))
			mdl = Model::LoadOPK(fn, progctl);
		else {
			bool r;
			mdl = new Model;

			if ( !STRCASECMP(ext, ".3do"))
				r = mdl->Load3DO(fn, progctl);
			else if( !STRCASECMP(ext, ".s3o"))
				r = mdl->LoadS3O(fn, progctl);
			else if(!STRCASECMP(ext, ".3ds"))
				r = (mdl->root = Load3DSObject(fn, progctl)) != 0;
			else if (!STRCASECMP(ext, ".obj"))
				r = (mdl->root = LoadWavefrontObject(fn, progctl)) != 0;
			else if (!STRCASECMP(ext, ".c3o"))
				r = mdl->LoadC3O(fn, progctl);
			else {
				fltk::message ("Unknown extension %s\n", fltk::filename_ext(fn));
				delete mdl;
				return nullptr;
			}
			if (!r) {
				delete mdl;
				mdl = 0;
			}
		}
	}
	catch (std::runtime_error err)
	{
		fltk::message (err.what());
		return nullptr;
	}
	if (mdl)
		return mdl;
	else {
		fltk::message ("Failed to read file %s:\n",fn);
		return nullptr;
	}
}

bool Model::Save(Model* mdl, const string& _fn, IProgressCtl& progctl)
{
	bool r = false;
	const char *fn = _fn.c_str();
	const char *ext=fltk::filename_ext(fn);

	if (!mdl->root) {
		fltk::message ("No objects");
		return false;
	}

	assert (fn&&fn[0]);
	if ( !STRCASECMP(ext, ".3do") )
		r = mdl->Save3DO(fn, progctl);
	else if( !STRCASECMP(ext, ".s3o" )) {
		r = mdl->SaveS3O(fn, progctl);
	} else if( !STRCASECMP(ext, ".3ds"))
		r = Save3DSObject(fn, mdl->root,progctl);
	else if( !STRCASECMP(ext, ".obj"))
		r = SaveWavefrontObject(fn, mdl->root, progctl);
	else if( !STRCASECMP(ext, ".c3o"))
		r = mdl->SaveC3O(fn, progctl);
	else if (!STRCASECMP(ext, ".opk"))
		r = Model::SaveOPK(mdl, fn, progctl);
	else
		fltk::message ("Unknown extension %s\n", fltk::filename_ext(fn));
	if (!r) {
		fltk::message ("Failed to save file %s\n", fn);
	}
	return r;
}

static void GetSelectedObjectsHelper( MdlObject *obj, vector<MdlObject*>& sel)
{
	if (obj->isSelected)
		sel.push_back (obj);

	for (unsigned int a=0;a<obj->childs.size();a++)
		GetSelectedObjectsHelper(obj->childs[a],sel);
}

vector <MdlObject*> Model::GetSelectedObjects ()
{
	vector<MdlObject*> sel;
	if (root)
		GetSelectedObjectsHelper (root,sel);
	return sel;
}

void Model::DeleteObject(MdlObject *obj)
{
	if (obj->parent) {
		vector<MdlObject*>::iterator i=find (obj->parent->childs.begin(),obj->parent->childs.end(), obj);
		if (i != obj->parent->childs.end()) obj->parent->childs.erase (i);
	} else {
		assert (obj == root);
		root = 0;
	}

	delete obj;
}

void Model::SwapObjects (MdlObject *a, MdlObject *b)
{
	MdlObject *ap = a->parent, *bp = b->parent;
	
	// Unlink both objects from their parents 
	// (this will also fix problems if b is a child of a or vice versa)
	a->UnlinkFromParent();
	b->UnlinkFromParent();

	// Swap childs
	swap(a->childs,b->childs);
	// assign parents again on the swapped childs
	for (vector<MdlObject*>::iterator ci=a->childs.begin();ci!=a->childs.end();ci++)
		(*ci)->parent = a;
	for (vector<MdlObject*>::iterator ci=b->childs.begin();ci!=b->childs.end();ci++)
		(*ci)->parent = b;

	if (b == ap) { // was b originally the parent of a?
		b->LinkToParent(a);
		if (bp) a->LinkToParent(bp);
	} else if(a == bp) { // or was a originally the parent of b
		a->LinkToParent(b);
		if (ap) b->LinkToParent(ap);
	} else { // no parents of eachother at all
		if (bp) a->LinkToParent(bp);
		if (ap) b->LinkToParent(ap);
	}

	if (a == root) root = b;
	else if (b == root) root = a;
}

void Model::ReplaceObject (MdlObject *old, MdlObject *_new)
{
	// Insert the old childs into the new object
	_new->childs.insert (_new->childs.end(), old->childs.begin(),old->childs.end());
	old->childs.clear();
	
	// Delete the old object
	MdlObject *parent = old->parent;
	DeleteObject (old);

	// Insert the new object
	if (parent) parent->childs.push_back (_new);
	else root = _new;
}

static void AddPositions (MdlObject *o, Vector3& p,int &count) {
	Matrix transform;
	o->GetTransform(transform);

	for (VertexIterator v(o);!v.End();v.Next()) {
		Vector3 temp;
		transform.apply (&v->pos, &temp);
		p += temp;
		count ++;
	}
	for (unsigned int a=0;a<o->childs.size();a++)
		AddPositions (o->childs[a], p, count);
}


void Model::EstimateMidPosition ()
{
	Vector3 total;
	int count=0;
	if (root) 
		AddPositions (root, total, count);

	if (count) {
		mid = total / count;
	} else mid=Vector3();
}

void Model::CalculateRadius ()
{
	vector<MdlObject*> objs = GetObjectList();
	radius=0.0f;
	for (unsigned int o=0;o<objs.size();o++) {
		MdlObject *obj = objs[o];
		/*PolyMesh *pm = obj->GetPolyMesh();*/
		Matrix objTransform;
		obj->GetFullTransform(objTransform);
		if (obj->geometry)
			obj->geometry->CalculateRadius(radius, objTransform, mid);
	}
}


bool Model::ExportUVMesh (const char *fn)
{
	// create a cloned model
	MdlObject *cloned = root->Clone ();
	Model mdl;
	mdl.root = cloned;
	mdl.texBindings = texBindings;

	// merge all objects in the clone
	while (!mdl.root->childs.empty()) {
		vector<MdlObject*> childs=mdl.root->childs;

		for (vector<MdlObject*>::iterator ci=childs.begin(); ci!=childs.end(); ++ci)
			mdl.root->MergeChild(*ci);
	}

	return Model::Save(&mdl, fn);
}

bool Model::ImportUVMesh(const char *fn, IProgressCtl& progctl) {
	Model *mdl;
	mdl = Model::Load(fn, false);// an unoptimized mesh so the vertices are not merged
		return false;

	if (!ImportUVCoords(mdl, progctl)) {
		delete mdl;
		return false;
	}
	delete mdl;
	return true;
}


int MatchPolygon (MdlObject *root, vector<Vector3>& pverts, int& startVertex)
{
	int index = 0;
	for (PolyIterator pi(root); !pi.End();pi.Next(),index++) {
		Poly *pl = *pi;

		if (pl->verts.size() != pverts.size() || !pi.verts())
			continue;

		// An early out plane comparision, will also make sure that "double-sided" polgyon pairs
		// are handled correctly
		Plane plane = pl->CalcPlane (*pi.verts());
		Plane tplane;
		tplane.MakePlane (pverts[0],pverts[1],pverts[2]);
		
		if (!plane.EpsilonCompare(tplane, EPSILON))
			continue;

		// in case the polygon vertices have been reordered, 
		// this takes care of finding "the first" vertex again
		unsigned int startv = 0;
		for (;startv < pverts.size();startv++) {
			if (( (*pi.verts())[pl->verts [0]].pos-pverts[startv]).length () < EPSILON)
				break;
		}
		// no start vertex has been found
		if (startv == pverts.size())
			continue;

		// compare the polygon vertices with eachother... 
		unsigned int v = 0;
		for (;v<pverts.size();v++) {
			if (( (*pi.verts())[pl->verts[v]].pos - pverts[(v+startv)%pverts.size()]).length () >= EPSILON)
				break;
		}
		if (v==pverts.size()) {
			startVertex=startv;
			return index;
		}
	}
	return -1;
}

bool Model::ImportUVCoords(Model* other, IProgressCtl &progctl) {
	vector <MdlObject*> objects=GetObjectList ();
	MdlObject *srcobj = other->root;

	vector <Vector3> pverts;

	int numPl = 0, curPl=0;
	for (unsigned int a=0;a<objects.size();a++) {
		PolyMesh *pm = objects[a]->GetPolyMesh();
		if (pm)
			numPl += pm->poly.size();
	}
	
	for (unsigned int a=0;a<objects.size();a++) {
		MdlObject *obj = objects[a];
		Matrix objTransform;
		obj->GetFullTransform(objTransform);
		PolyMesh *pm = obj->GetPolyMesh();

		// give each polygon an independent set of vertices, this will be optimized back to normal later
		vector <Vertex> nverts;
		for (PolyIterator pi(obj);!pi.End();pi.Next()) {
			for (unsigned int v=0;v<pi->verts.size();v++) {
				nverts.push_back (pm->verts[pi->verts[v]]);
				pi->verts[v]=nverts.size()-1;
			}
		}
		pm->verts=nverts;

		// match our polygons with the ones of the other model
		for (PolyIterator pi(obj);!pi.End();pi.Next()) {
			pverts.clear();
			for (unsigned int pv=0;pv<pi->verts.size();pv++) {
				Vector3 tpos;
				objTransform.apply (&pm->verts [pi->verts[pv]].pos, &tpos);
				pverts.push_back (tpos);
			}

			int startVertex;
			int bestpl = MatchPolygon (srcobj,pverts,startVertex);
			PolyMesh *srcpm = srcobj->GetPolyMesh();
			if (bestpl >= 0) {
				// copy texture coordinates from rt->poly[bestpl] to pl
				Poly *src = srcpm->poly [bestpl];
				for (unsigned int v=0;v<src->verts.size();v++) {
					Vertex &dstvrt = pm->verts[pi->verts[(v + startVertex)%pi->verts.size()]];
					dstvrt.tc[0] = srcpm->verts[src->verts[v]].tc[0];
				}
			}

			progctl.Update ((float)(curPl++) / numPl);
		}
		obj->InvalidateRenderData();
	}

	return true;
}

static void GetObjectListHelper (MdlObject *obj, vector<MdlObject*>& list)
{
	list.push_back (obj);
	for (unsigned int a=0;a<obj->childs.size();a++)
		GetObjectListHelper (obj->childs [a], list);
}

vector<MdlObject*> Model::GetObjectList ()
{
	vector<MdlObject*> objlist;
	if (root)
		GetObjectListHelper (root, objlist);
	return objlist;
}

vector<PolyMesh*> Model::GetPolyMeshList ()
{
	vector<MdlObject*> objlist = GetObjectList();
	vector<PolyMesh*> pmlist;
	for (uint a=0;a<objlist.size();a++)
		if (objlist[a]->GetPolyMesh ())
			pmlist.push_back (objlist[a]->GetPolyMesh());
	return pmlist;
}

Model* Model::LoadOPK(const char *filename, IProgressCtl& /*progctl*/) {
	creg::CInputStreamSerializer s;
	Model *mdl = 0;
	creg::Class *cls = 0;

	std::ifstream f (filename, ios::in|ios::binary);
	if (!f.is_open ())
		return 0;

	void *root = 0;
	s.LoadPackage (&f, root, cls);

	mdl = (Model *)root;
	if (cls != Model::StaticClass())
	{
		cls->DeleteInstance (mdl);
		return 0;
	}

    return mdl;
}

bool Model::SaveOPK(Model* mdl, const char *filename, IProgressCtl& /*progctl*/)
{
	creg::COutputStreamSerializer s;

	std::ofstream f (filename, ios::out|ios::binary);
	if (!f.is_open ())
		return false;

	s.SavePackage (&f, mdl, mdl->GetClass());
	return true;
}

// loads textures after a creg read serialization
void Model::PostLoad()
{
	for (uint t=0;t<texBindings.size();t++)
	{
		if(texBindings[t].name.empty())
			continue;

		texBindings[t].texture = new Texture (texBindings[t].name);
		if (!texBindings[t].texture->IsLoaded ())
			texBindings[t].texture = 0;
	}
}


Model* Model::Clone()
{
	Model *cpy = new Model();

	cpy->texBindings=texBindings;
	cpy->radius=radius;
	cpy->height=height;
	cpy->mid=mid;
	cpy->mapping=mapping;
	
	if (root)
		cpy->root=root->Clone();

	return cpy;
}



ulong Model::ObjectSelectionHash()
{
	ulong ch = 0;
	ulong a = 63689;
	vector<MdlObject*> sel = GetSelectedObjects();
	
	for (uint x=0;x<sel.size();x++) {
		ch = ch * a + (ulong)sel[x];
		a *= 378551;
	}

	return ch;
}


uint Vector3ToRGB(Vector3 v)
{
	return (uint(v.x * 255.0f) << 16) | (uint(v.y * 255.0f) << 8) | (uint(v.z * 255.0f) << 0);
}

bool Model::ConvertToS3O(std::string textureName, int texw, int texh)
{
	// collect all textures used by the model
	std::set<Texture *> textures;
	std::map<uint, RefPtr<Texture> > coltextures;

	vector<PolyMesh*> pmlist = GetPolyMeshList();
	vector<Poly*> polygons = GetElementList(&PolyMesh::poly, pmlist.begin(), pmlist.end());

	for (unsigned int a=0;a<polygons.size();a++)
	{
		if (polygons[a]->texture)
			textures.insert (polygons[a]->texture.Get());
		else { // create a new color texture
			uint color = Vector3ToRGB (polygons[a]->color);
			RefPtr<Texture>& cur = coltextures [color];

			if (!cur) {
				Texture *ct = new Texture ();
				Image *cimg = new Image(1,1, ImgFormat(ImgFormat::RGB));
				cimg->SetPixel (0, 0, color << 8); // RGB to RGBA
				ct->SetImage (cimg);
				ct->name = SPrintf("Color%d", color);
				cur = ct;
			}
			polygons[a]->texture = cur;
			textures.insert (cur.Get());
		}
	}

	TextureBinTree tree;
	ImgFormat fmt (ImgFormat::RGB);
	tree.Init (texw,texh,&fmt);

	std::map <Texture*, TextureBinTree::Node *> texToNode;

	for (std::set<Texture*>::iterator ti = textures.begin(); ti != textures.end(); ++ti)
	{
		Image conv;
		conv.format = ImgFormat(ImgFormat::RGB);
		(*ti)->image->Convert (&conv);
		//conv.Save ( (textureLocation + (*ti)->name + "_test.bmp").c_str());
		TextureBinTree::Node *node = tree.AddNode (&conv);

		if (!node) {
			fltk::message ("Not enough texture space for all 3DO textures.");
			return false;
		}

		texToNode [*ti] = node;
	}
	
	Image *res = tree.GetResult();
	res->Save ( textureName.c_str() );

	mapping = MAPPING_S3O;
	Texture *nt = new Texture();
	nt->SetImage (res->Clone());
	nt->name = textureName;

	SetTexture(0, nt);

	// now set new texture coordinates. 
	// Vertices might need to be split, so vertices are calculated per frame and then optimized again

	for (vector<PolyMesh*>::iterator pmi = pmlist.begin(); pmi != pmlist.end(); ++pmi)
	{
		PolyMesh *pm = *pmi;
		std::vector<Vertex> vertices;

		for (uint a=0;a<pm->poly.size();a++)
		{
			Poly *pl = pm->poly[a];
			TextureBinTree::Node *tnode = texToNode[pl->texture.Get()];

			if (pl->verts.size() <= 4) {
				const float tc[] = { 0.0f,1.0f,  1.0f, 1.0f,   1.0f,0.0f, 0.0f,0.0f};

				for (uint v=0;v<pl->verts.size();v++) {
					vertices.push_back (pm->verts [pl->verts[v]]);
					Vertex& vrt = vertices.back();
					// convert to texturebintree UV coords:
					vrt.tc[0].x = tree.GetU (tnode, tc[v*2+0]);
					vrt.tc[0].y = tree.GetV (tnode, tc[v*2+1]);

					pl->verts[v] = vertices.size()-1;
				}
			} else {
				for (uint v=0;v<pl->verts.size();v++)
				{
					vertices.push_back (pm->verts [pl->verts[v]]);
					Vertex& vrt = vertices.back();
					vrt.tc[0].x = vrt.tc[0].y = 0.0f;
					pl->verts[v] = vertices.size()-1;
				}
			}
		}

		pm->verts = vertices;
		pm->Optimize (&PolyMesh::IsEqualVertexTC);
	}
	return true;
}


