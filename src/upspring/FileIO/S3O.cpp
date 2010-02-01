//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"
#include "Util.h"
#include "Model.h"
#include "Texture.h"

#pragma pack(push, 4)
#include "S3O.h"
#pragma pack(pop)

#define S3O_ID "Spring unit"

static void MirrorX(MdlObject *o)
{
	PolyMesh* pm = o->GetPolyMesh();
	if (pm)
	{
		for (unsigned int a=0;a<pm->verts.size();a++) {
			pm->verts[a].pos.x *= -1.0f;
			pm->verts[a].normal.x *= -1.0f;
		}

		for (unsigned int a=0;a<pm->poly.size();a++)
			pm->poly[a]->Flip();
	}

	o->position.x *= -1.0f;
	for (unsigned int a=0;a<o->childs.size();a++)
		MirrorX(o->childs[a]);
}

static MdlObject *S3O_LoadObject (FILE *f, ulong offset)
{
	size_t read_result;
	int oldofs = ftell(f);
	MdlObject *obj = new MdlObject;
	PolyMesh* pm;
	obj->geometry = pm = new PolyMesh;

	// Read piece header
	S3OPiece piece;
	fseek (f, offset, SEEK_SET);
	read_result = fread (&piece, sizeof(S3OPiece), 1, f);
	if (read_result != 1) throw std::runtime_error ("Couldn't read piece header.");

	// Read name
	obj->name = ReadString (piece.name, f);
	obj->position.set(piece.xoffset,piece.yoffset,piece.zoffset);

	// Read child objects
	fseek (f, piece.childs, SEEK_SET);
	for (unsigned int a=0;a<piece.numChilds;a++) {
		ulong chOffset;
		read_result = fread (&chOffset, sizeof(ulong), 1, f);
		if (read_result != 1) throw std::runtime_error ("Couldn't read child object.");
		MdlObject *child = S3O_LoadObject (f, chOffset);
		if (child) {
			child->parent = obj;
			obj->childs.push_back (child);
		}
	}

	// Read vertices
	pm->verts.resize (piece.numVertices);
	fseek (f, piece.vertices, SEEK_SET);
	for (unsigned int a=0;a<piece.numVertices;a++) {
		S3OVertex sv;
		read_result = fread (&sv, sizeof(S3OVertex), 1, f);
		if (read_result != 1) throw std::runtime_error ("Couldn't read vertex.");
		pm->verts [a].normal.set (sv.xnormal, sv.ynormal, sv.znormal);
		pm->verts [a].pos.set (sv.xpos, sv.ypos, sv.zpos);
		pm->verts [a].tc[0] = Vector2(sv.texu, sv.texv);
	}

	// Read primitives - 0=triangles,1 triangle strips,2=quads
	fseek (f, piece.vertexTable, SEEK_SET);
	switch (piece.primitiveType) { 
		case 0: { // triangles
			for (unsigned int i=0;i<piece.vertexTableSize;i+=3) {
				ulong index;
                Poly *pl = new Poly;
				pl->verts.resize(3);
				for (int a=0;a<3;a++) {
					read_result = fread (&index,4,1,f);
					if (read_result != 1) throw std::runtime_error ("Couldn't read primitive.");
					pl->verts [a] = index;
				}
				pm->poly.push_back (pl);
			}
			break;}
		case 1: { // tristrips
			ulong *data=new ulong[piece.vertexTableSize];
			read_result = fread (data,4,piece.vertexTableSize, f);
			if (read_result != piece.vertexTableSize) throw std::runtime_error ("Couldn't read tristrip.");
			for (unsigned int i=0;i<piece.vertexTableSize;) {
				// find out how long this strip is
				unsigned int first=i;
				while (i<piece.vertexTableSize && data[i]!=-1) 
					i++;
				// create triangles from it
				for (unsigned int a=2;a<i-first;a++) {
					Poly *pl = new Poly;
					pl->verts.resize(3);
					for (int x=0;x<3;x++)
						pl->verts[(a&1)?x:2-x]=data[first+a+x-2];
					pm->poly.push_back(pl);
				}
			}
			delete[] data;
			break;}
		case 2: { // quads
			for (unsigned int i=0;i<piece.vertexTableSize;i+=4) {
				ulong index;
                Poly *pl = new Poly;
				pl->verts.resize(4);
				for (int a=0;a<4;a++) {
					read_result = fread (&index,4,1,f);
					if (read_result != 1) throw std::runtime_error ("Couldn't read quad.");
					pl->verts [a] = index;
				}
				pm->poly.push_back (pl);
			}
			break;}
	}

//	fltk::message("object %s has %d polygon and %d vertices", obj->name.c_str(), obj->poly.size(),pm->verts.size());

	fseek (f, oldofs, SEEK_SET);
	return obj;
}


bool Model::LoadS3O(const char *filename, IProgressCtl& /*progctl*/) {
	S3OHeader header;
	size_t read_result;
	FILE *file = fopen (filename, "rb");
	if (!file)
		return false;

	read_result = fread (&header, sizeof(S3OHeader), 1, file);
	if (read_result != (size_t)1) throw std::runtime_error ("Couldn't read S3O header.");

	logger.Trace (NL_Error,"Wrong version. Only version 1 is supported");

	if (memcmp (header.magic, S3O_ID, 12)) {
		logger.Trace (NL_Error, "S3O model %s has wrong identification", filename);
		fclose (file);
		return false;
	}

	if (header.version != 0) {
		logger.Trace (NL_Error, "S3O model %s has wrong version (%d, wanted: %d)", filename, header.version, 0);
		fclose (file);
		return false;
	}

	radius = header.radius;
	mid.set (-header.midx, header.midy, header.midz);
	height = header.height;

	root = S3O_LoadObject (file, header.rootPiece);
	MirrorX(root);

	string mdlPath = GetFilePath (filename);

	// load textures
	for (int tex=0;tex<2;tex++) {
		if ( !(tex ? header.texture2 : header.texture1))
			continue;

		texBindings.push_back(TextureBinding());
		TextureBinding &tb = texBindings.back ();

		tb.name = ReadString (tex ? header.texture2 : header.texture1, file);
		tb.texture = new Texture (tb.name, mdlPath);
		if (!tb.texture->IsLoaded ())
			tb.texture = 0;
	}

	mapping = MAPPING_S3O;

	fclose (file);
	return true;
}

static void S3O_WritePrimitives(S3OPiece *p, FILE *f, PolyMesh* pm)
{
	bool allQuads=true;
	size_t write_result;
	unsigned int a=0;
	for (;a<pm->poly.size();a++) {
		if (pm->poly[a]->verts.size()!=4)
			allQuads=false;
	}

	p->vertexTable = ftell(f);
	if (allQuads) {
		for (unsigned int a=0;a<pm->poly.size();a++) {
			Poly *pl = pm->poly[a];
			for (int b=0;b<4;b++) {
				uint i=pl->verts[b];
				write_result = fwrite (&i, 4,1,f);
				if (write_result != (size_t)1) throw std::runtime_error ("Couldn't write poly.");
			}
		}
		p->vertexTableSize = 4 * (uint)pm->poly.size();
		p->primitiveType=2;
	} else {
		vector<Triangle> tris = pm->MakeTris ();
		for (unsigned int a=0;a<tris.size();a++)
		{
			for (int b=0;b<3;b++) {
				uint i=tris[a].vrt[b];
				write_result = fwrite (&i,4,1,f);
				if (write_result != (size_t)1) throw std::runtime_error ("Couldn't write tri.");

			}
		}
		p->vertexTableSize = 3 * (uint)tris.size();
		p->primitiveType=0;
	}
}

static void S3O_SaveObject (FILE *f, MdlObject *obj)
{
	int startpos = ftell(f);
	S3OPiece piece;
	size_t write_result;
	memset(&piece, 0, sizeof(piece));

	fseek (f, sizeof(S3OPiece), SEEK_CUR);
	piece.name = ftell(f);
	WriteZStr(f, obj->name);
	
	piece.xoffset = obj->position.x;
	piece.yoffset = obj->position.y;
	piece.zoffset = obj->position.z;
	piece.collisionData = 0;
	piece.vertexType = 0;

	PolyMesh* pm = obj->geometry ? obj->geometry->ToPolyMesh() : 0;
	if (pm) 
	{
		S3O_WritePrimitives (&piece, f, pm);

		piece.numVertices = (uint) pm->verts.size();
		piece.vertices = ftell(f);
		for (unsigned int a=0;a<pm->verts.size();a++)
		{
			Vertex *myVert=&pm->verts[a];
			S3OVertex v;
			v.texu=myVert->tc[0].x;
			v.texv=myVert->tc[0].y;
			v.xnormal=myVert->normal.x;
			v.ynormal=myVert->normal.y;
			v.znormal=myVert->normal.z;
			v.xpos=myVert->pos.x;
			v.ypos=myVert->pos.y;
			v.zpos=myVert->pos.z;
			write_result = fwrite(&v,sizeof(S3OVertex),1,f);
			if (write_result != (size_t)1) throw std::runtime_error ("Couldn't write vertex.");
		}
		delete pm;
	}

	piece.numChilds = (uint)obj->childs.size();
	ulong *childpos=new ulong[piece.numChilds];
	for (unsigned int a=0;a<obj->childs.size();a++)
	{
		childpos[a] = ftell(f);
		S3O_SaveObject (f,obj->childs[a]);
	}
	piece.childs=ftell(f);
	write_result = fwrite (childpos,4,piece.numChilds,f);
	if (write_result != (size_t)(piece.numChilds)) throw std::runtime_error ("Couldn't write child.");
	delete[] childpos;

	int endpos=ftell(f);
	fseek (f,startpos,SEEK_SET);
	write_result = fwrite (&piece,sizeof(S3OPiece),1,f);
	if (write_result != (size_t)1) throw std::runtime_error ("Couldn't write piece.");
	fseek (f,endpos,SEEK_SET);
}


// S3O supports position saving, but no rotation or scaling
static inline void ApplyOrientationAndScaling (MdlObject *o) 
{
	o->ApplyTransform(true,true,false);
}


bool Model::SaveS3O(const char *filename, IProgressCtl& /*progctl*/) {
	S3OHeader header;
	size_t write_result;
	memset (&header,0,sizeof(S3OHeader));
	memcpy (header.magic, S3O_ID, 12);

	if (!root)
		return false;
	
	FILE *f = fopen (filename, "wb");
	if (!f) 
		return false;

	fseek (f, sizeof(S3OHeader), SEEK_SET);
	header.rootPiece = ftell(f);

	if (root) {
		MdlObject *cloned = root->Clone();
		IterateObjects (cloned, ApplyOrientationAndScaling);
		MirrorX(cloned);
		S3O_SaveObject (f, cloned);
		delete cloned;
	}

	for (uint tex=0;tex<texBindings.size();tex++) {
		if (tex >= texBindings.size ())
			break;

		TextureBinding &tb = texBindings[tex];
		if (!tb.name.empty()) {
			if (tex==0) header.texture1 = ftell(f);
			if (tex==1) header.texture2 = ftell(f);
			WriteZStr (f, tb.name);
		}
	}

	header.radius = radius;
	header.height = height;
	header.midx = -mid.x;
	header.midy = mid.y;
	header.midz = mid.z;

	fseek (f, 0, SEEK_SET);
	write_result = fwrite (&header, sizeof(S3OHeader), 1, f);
	if (write_result != (size_t)1) throw std::runtime_error ("Couldn't write S3O header.");
	fclose (f);

	return true;
}


