//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"
#include "Util.h"
#include "Texture.h"
#include "Model.h"

#include "c3o_fmt.h"

struct C3O_IO
{
	vector <C3O_Chunk> chunks;
	FILE *file;
	map <int, TextureBinding> textures;
	string filename;

	~C3O_IO ();
	int AddChunk (ushort id, ushort version);

	int SaveObject(MdlObject* obj);
	int SaveVertices(const vector<Vertex>& vrt);
	int SavePolygons(const vector<Poly*>& pl);
	int SaveIKinfo(IKinfo &ik);
	int SaveTexture(char type, const string& name);
	
	vector<Vertex> LoadVertices (int chunk);
	vector<Poly*> LoadPolygons (int chunk);
	MdlObject* LoadObject(int chunk);
	void LoadIKinfo(int chunk, IKinfo& ik);
	void LoadTexture(Model* mdl, int chunk);
};

C3O_IO::~C3O_IO()
{
	if (file) 
		fclose(file);
}

int C3O_IO::AddChunk (ushort id, ushort version)
{
	chunks.push_back (C3O_Chunk());

	chunks.back().id = id;
	chunks.back().version = version;
	chunks.back().offset = ftell(file);

	return chunks.size()-1;
}

int C3O_IO::SaveIKinfo (IKinfo& ik)
{
	if (ik.jointType == IKJT_Hinge)
	{
		C3O_HingeJoint hj;
		((HingeJoint*)ik.joint)->axis.copy (hj.axis);

		int ch=AddChunk (C3O_HINGEJT, 0);
		fwrite (&hj, sizeof(C3O_HingeJoint), 1, file);
		return ch;
	}
	else if (ik.jointType == IKJT_Universal)
	{
		C3O_UniversalJoint uj;
		((UniversalJoint *)ik.joint)->axis [0].copy (uj.axis1);
		((UniversalJoint *)ik.joint)->axis [1].copy (uj.axis2);

		int ch=AddChunk (C3O_UNIVERSALJT, 0);
		fwrite (&uj, sizeof(C3O_UniversalJoint), 1, file);
		return ch;
	}
	return -1;
}

void C3O_IO::LoadIKinfo (int chunkID, IKinfo& ik)
{
	C3O_Chunk& ch = chunks[chunkID];

	fseek (file, ch.offset, SEEK_SET);

	if (ch.id == C3O_HINGEJT) {
		C3O_HingeJoint j;
		if (fread (&j, sizeof(C3O_HingeJoint), 1, file)) {}

		ik.jointType = IKJT_Hinge;
		HingeJoint *jt = new HingeJoint;
		jt->axis.set (j.axis);
	}

	if (ch.id == C3O_UNIVERSALJT) {
		C3O_UniversalJoint j;
		if (fread (&j, sizeof(C3O_UniversalJoint), 1, file)) {}

		ik.jointType = IKJT_Universal;
		UniversalJoint *jt = new UniversalJoint;
		jt->axis [0].set (j.axis1);
		jt->axis [1].set (j.axis2);
	}
}

int C3O_IO::SavePolygons (const vector<Poly*>& pl)
{
	// write polygon chunk
	int chunk = AddChunk (C3O_POLYGONTBL, 2);
	int npl = pl.size();
	fwrite (&npl, sizeof(int), 1, file);

	for (unsigned int a=0;a<pl.size();a++)
	{
		const Poly* p = pl[a];
		fputc (p->verts.size(), file);

		for (unsigned int v=0;v<p->verts.size();v++)  {
			ushort index = p->verts [v];
			fwrite (&index, sizeof(ushort), 1, file);
		}
	}

	return chunk;
}

vector<Poly*> C3O_IO::LoadPolygons (int chunk)
{
	vector<Poly*> polygons;
	C3O_Chunk& ch = chunks[chunk];

	if (ch.id != C3O_POLYGONTBL || ch.version != 2)
		throw std::runtime_error ("Wrong polygon chunk type or incorrect polygon chunk version.");

	int npl;
	fseek (file, ch.offset, SEEK_SET);
	if (fread (&npl, sizeof(int), 1, file)) {}

	polygons.resize (npl);
	for (int a=0;a<npl;a++) {
		Poly *pl = new Poly;
		polygons[a] = pl;

		int vc = fgetc (file);
		pl->verts.resize (vc);
		for (int v=0;v<vc;v++) {
			ushort index;
			if (fread (&index, 2,1,file)) {}
			pl->verts [v] = index;
		}
	}

	return polygons;
}

int C3O_IO::SaveVertices (const vector<Vertex>& vertices)
{
	int chunk = AddChunk (C3O_VERTEXTBL, 0);

	int nvrt = vertices.size();
	fwrite (&nvrt, sizeof(int), 1, file);

	for(unsigned int a=0;a<vertices.size();a++) {
		const Vertex& v = vertices[a];

		C3O_Vertex wv;
		for (int x=0;x<3;x++) {
			wv.pos [x] = v.pos.v[x];
			wv.normal [x] = v.normal.v[x];
		}
		wv.texcoord [0] = v.tc[0].x;
		wv.texcoord [1] = v.tc[0].y;

		fwrite (&wv, sizeof(C3O_Vertex), 1, file);
	}

	return chunk;
}

vector<Vertex> C3O_IO::LoadVertices (int chunk)
{
	vector<Vertex> vertices;
	C3O_Chunk& ch = chunks[chunk];

	if (ch.id != C3O_VERTEXTBL || ch.version != 0)
		throw std::runtime_error ("Wrong vertex chunk type or incorrect vertex chunk version.");

	int nv;
	assert (sizeof(int)==4);
	fseek (file, ch.offset, SEEK_SET);
	if (fread (&nv, sizeof(int), 1, file)) {}

	vertices.resize (nv);
	for (int a=0;a<nv;a++) {
		C3O_Vertex vrt;
		Vertex& v = vertices[a];

		if (fread (&vrt, sizeof(C3O_Vertex), 1, file)) {}

		for (int x=0;x<3;x++) {
			v.normal.v [x] = vrt.normal [x];
			v.pos.v [x] = vrt.pos [x];
		}
		v.tc[0].x = vrt.texcoord[0];
		v.tc[0].y = vrt.texcoord[1];
	}
	return vertices;
}

int C3O_IO::SaveObject (MdlObject *obj)
{
	int chunk = AddChunk (C3O_OBJECT, 0);
	int startpos = ftell(file);
	fseek (file, sizeof(C3O_Object), SEEK_CUR);

	C3O_Object o;

	// write name
	o.nameOfs = ftell(file);
	WriteZStr (file, obj->name);

	// write child objects
	vector<int> childOfs;
	for (unsigned int a=0;a<obj->childs.size();a++)
		childOfs.push_back (SaveObject (obj->childs [a]));

	o.childOfs = ftell(file);
	o.numChilds = childOfs.size();
	if (o.numChilds) fwrite (&childOfs[0], sizeof(int), childOfs.size(), file);

	PolyMesh *pm = obj->ToPolyMesh();

	if (pm) {
		// write vertex chunk
		o.vertexTbl = SaveVertices (pm->verts);

		// write polygon chunk
		o.polyTbl = SavePolygons (pm->poly);
		delete pm;
	} else  {
		o.vertexTbl = o.polyTbl = 0;
	}

	Vector3 rotation = obj->rotation.GetEuler();
	for (int a=0;a<3;a++) {
		o.rotation [a] = rotation.v [a];
		o.scale [a] = obj->scale.v [a];
		o.position [a] = obj->position.v [a];
	}

	o.jointInfoChunk = SaveIKinfo (obj->ikInfo);

	int lastpos = ftell(file);
	fseek (file, startpos, SEEK_SET);
	fwrite (&o, sizeof(C3O_Object), 1, file);
	fseek (file, lastpos, SEEK_SET);

	return chunk;
}

MdlObject* C3O_IO::LoadObject (int chunk)
{
	C3O_Chunk & ch = chunks[chunk];
	if (ch.version != 0)
		throw std::runtime_error ("Incorrect version of object");

	MdlObject *obj = new MdlObject;

	C3O_Object o;
	fseek (file, ch.offset, SEEK_SET);
	if (fread (&o, sizeof(C3O_Object), 1, file)) {}

	// read childs
	fseek (file, o.childOfs, SEEK_SET);
	vector <int> childs;
	if(o.numChilds>0) {
		childs.resize(o.numChilds);
		if (fread (&childs[0], sizeof(int), o.numChilds, file)) {}
		for (int a=0;a<o.numChilds;a++) {
			obj->childs.push_back (LoadObject (childs[a]));
			obj->childs.back()->parent = obj;
		}
	}

	// read name
	fseek (file, o.nameOfs, SEEK_SET);
	obj->name = ReadZStr (file);

	PolyMesh *pm = new PolyMesh;
	obj->geometry = pm;

	pm->verts = LoadVertices (o.vertexTbl);
	pm->poly = LoadPolygons (o.polyTbl);

	Vector3 rotation;
	for (int a=0;a<3;a++) {
		obj->position.v [a] = o.position [a];
		rotation.v[a] = o.rotation [a];
		obj->scale.v [a] = o.scale [a];
	}
	obj->rotation.AddEulerAbsolute(rotation);

	if (o.jointInfoChunk >= 0)
		LoadIKinfo (o.jointInfoChunk, obj->ikInfo);

	return obj;
}

void C3O_IO::LoadTexture (Model* /*mdl*/, int chunk)
{
	C3O_Chunk& ch = chunks[chunk];

	if (ch.version != 0) {
		logger.Trace (NL_Error, "Texture chunk has wrong version.");
		return;
	}

	fseek(file, ch.offset, SEEK_SET);
	int index = fgetc (file);

	if (textures.find (index) != textures.end()) {
		fltk::message ("File error: 2 textures with the same index found");
		return;
	}

	TextureBinding binding;
	binding.name = ReadZStr (file);

	string mdlPath = GetFilePath (filename);
	binding.texture = new Texture (binding.name, mdlPath);
	
	if (binding.texture->IsLoaded ())
		textures [index] = binding;
}

int C3O_IO::SaveTexture (char type, const string& name)
{
	int ch=AddChunk (C3O_TEXTURE, 0);
	fputc(type, file);
	WriteZStr (file, name);
	return ch;
}


bool Model::SaveC3O(const char *filename, IProgressCtl& /*progctl*/) {
	C3O_IO c3o;

	if (!root) return false;

	c3o.file = fopen(filename, "wb");
	if (!c3o.file) 
		return false;

	C3O_Header h;
	h.version = C3O_VERSION;
	h.id = C3O_MAGIC_ID;
	fseek (c3o.file, sizeof(C3O_Header), SEEK_SET);

	// Write object hierarchy
	h.rootObj = c3o.SaveObject (root);

	// Write texture references
	for (uint a=0;a<texBindings.size();a++) {
		if (!texBindings[a].name.empty())
			c3o.SaveTexture (a, texBindings[a].name);
	}

	// write chunk table
	h.chunkTableOfs = ftell(c3o.file);
	h.numChunks = c3o.chunks.size();
	fwrite (&c3o.chunks[0], sizeof(C3O_Chunk), c3o.chunks.size(), c3o.file);

	// write header
	fseek (c3o.file, 0, SEEK_SET);
	fwrite (&h, sizeof(C3O_Header),1,c3o.file);

	return true;
}


bool Model::LoadC3O(const char *filename, IProgressCtl& /*progctl*/) {
	C3O_IO c3o;
	c3o.file = fopen(filename, "rb");
	if (!c3o.file)
		return false;

	C3O_Header h;
	if (fread (&h, sizeof(C3O_Header), 1, c3o.file)) {}

	if (h.id != C3O_MAGIC_ID) 
		throw std::runtime_error ("File does not have C3O identification.");
	if (h.version != C3O_VERSION)
		throw std::runtime_error ("File does not have the correct version.");

	fseek (c3o.file, h.chunkTableOfs, SEEK_SET);
	if (h.numChunks>0) {
		c3o.chunks.resize (h.numChunks);
		if (fread (&c3o.chunks [0], sizeof(C3O_Chunk), h.numChunks, c3o.file)) {}

		root = c3o.LoadObject (h.rootObj);
	} else root=0;

	for (unsigned int a=0;a<c3o.chunks.size();a++) {
		if (c3o.chunks [a].id == C3O_TEXTURE)
			c3o.LoadTexture (this, a);
	}

	if (!c3o.textures.empty())
	{
		int numbinds = 0;
		for (map<int,TextureBinding>::iterator i=c3o.textures.begin();i!=c3o.textures.end();++i)
			if (numbinds <= i->first) numbinds=i->first+1;
		
		texBindings.resize(numbinds);
		for (map<int,TextureBinding>::iterator i=c3o.textures.begin();i!=c3o.textures.end();++i)
			texBindings[i->first] = i->second;
	}

	return root != 0;
}

