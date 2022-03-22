//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"
#include "Model.h"
#include "Util.h"

#include <lib3ds.h>

static inline void removeTransform(MdlObject *obj) 
{
	obj->ApplyTransform(true,true,true);
}

static Lib3dsMesh *ConvertObjTo3DS (MdlObject *obj)
{
	Lib3dsMesh *mesh = lib3ds_mesh_new(obj->name.c_str());

	PolyMesh *pm = obj->GetPolyMesh ();
	if (pm && !pm->verts.empty()) {
		lib3ds_mesh_resize_vertices(mesh, pm->verts.size(), 1, 1);
		for (std::size_t v = 0, max = pm->verts.size(); v != max; ++v)
		{
			// Copy Vertexes
			for(uint axis=0;axis<3;axis++)
				mesh->vertices[v][axis] = pm->verts[v].pos[axis];

			mesh->texcos[v][0] = pm->verts[v].tc[0].x;
			mesh->texcos[v][1] = pm->verts[v].tc[0].y;
		}

		uint numFaces=0;
		for (std::size_t p = 0, max = pm->poly.size(); p != max; ++p)
			numFaces+=pm->poly[p]->verts.size()-2;

		lib3ds_mesh_resize_faces(mesh, numFaces);
		uint curFace=0;
		for (std::size_t p = 0, max = pm->poly.size(); p != max; ++p)
		{
			Poly *pl = pm->poly [p];
			for(uint v=2;v<pl->verts.size();v++)
			{
				mesh->faces[p].index[0] = pl->verts[0];
				mesh->faces[p].index[1] = pl->verts[v-1];
				mesh->faces[p].index[2] = pl->verts[v];
				curFace++;
			}
		}
	}
	return mesh;
}

bool Save3DSObject(const char *fn, MdlObject *obj, IProgressCtl& /*progctl*/)
{
	MdlObject *cl = obj->Clone ();
	IterateObjects (cl, removeTransform);

	Lib3dsFile *file = lib3ds_file_new();

	int choice = 0;
	if (!obj->childs.empty ()) {
		choice = fltk::choice("3DS can't store a tree of objects, so what should be done?:\n"
			"1) Just save all objects without hierarchy\n"
			"2) Don't save childs\n"
			"3) Merge all childs",
			"1","2", "3");
	}
	
	std::vector <MdlObject *> objList;
	if (choice == 0) 
		objList = cl->GetChildObjects();
	else if (choice == 2)
		cl->FullMerge();
	objList.push_back (cl);

	for (std::size_t i = 0, max = objList.size(); i != max; ++i)
	{
		Lib3dsMesh *mesh = ConvertObjTo3DS (objList[i]);
		lib3ds_file_insert_mesh (file, mesh, i);
	}

	bool r = lib3ds_file_save (file, fn);

	// cleanup
	delete cl;
	lib3ds_file_free(file);

	return !!r;
}

static MdlObject *Convert3DSToObj (Lib3dsMesh *mesh)
{
	MdlObject *obj = new MdlObject();
	PolyMesh *pm = obj->GetOrCreatePolyMesh ();

	printf("Name: %s\n", mesh->name);

	if (!mesh->faces)
		return 0;
	
	std::vector <Vertex> orgvrt(mesh->nvertices);
	for (uint i = 0; i < mesh->nvertices; i++) {
		orgvrt [i].tc[0].x = mesh->texcos[i][0];
		orgvrt [i].tc[0].y = mesh->texcos[i][1];

		for (uint x = 0; x < 3; x++)
			orgvrt[i].pos[x] = mesh->vertices[i][x];
	}

	printf("Name: %s\nVertices: %d\nFaces: %d\n", mesh->name,
	 mesh->vertices, mesh->nfaces);

	pm->verts.resize (mesh->nfaces * 3);
	pm->poly.resize (mesh->nfaces);

	float face_normals[mesh->nfaces][3];
	lib3ds_mesh_calculate_face_normals(mesh, face_normals);

	for (uint i = 0; i < mesh->nfaces; i++) {
		Poly* pl = new Poly();
		pl->verts.resize(3);

		Lib3dsFace *face = &mesh->faces[i];
		for (uint f = 0; f < 3; f++) {
			pl->verts[f] = i * 3 + f;
			Vertex& vrt = pm->verts[pl->verts[f]];
			
			vrt = orgvrt [face->index[f]];
			for (uint axis=0;axis<3;axis++)
				vrt.normal[axis] = face_normals[i*3+f][axis];
		}

		pm->poly[i] = pl;
	}

	delete face_normals;

	obj->name = mesh->name;

	return obj;
}

MdlObject *Load3DSObject(const char *fn, IProgressCtl& /*progctl*/)
{
	Lib3dsFile *file = lib3ds_file_open(fn);
	if (!file)
		return 0;

	int method = 2;

	int meshCount = file->meshes_size;
	if (meshCount > 1)
	{
		method = fltk::choice("The file contains more than 1 mesh. What should be done?:\n"
			"1) Put all geometry in one object\n"
			"2) Load all objects as childs of an empty root object\n"
			"3) Only load the first mesh\n",
			"1", "2", "3"
			);
	}
	else if (meshCount == 0)
	{
		fltk::message ("3DS file contains no meshes");
		return 0;
	}

	std::vector<MdlObject *> objects;

	for (int i = 0; i<file->meshes_size; i++) {
		MdlObject *o = Convert3DSToObj(file->meshes[i]);
		if (o) objects.push_back (o);
	}

	lib3ds_file_free(file);

	if (method == 1 || method == 0)
	{
		MdlObject *obj = new MdlObject;
		obj->geometry = new PolyMesh;
		obj->name = fltk::filename_name(fn);

		for (uint x=0;x<objects.size();x++)
			obj->AddChild (objects[x]);

		if (method == 0) // merge all
			obj->FullMerge ();
		return obj;
	}

	// method 2: objects contains 1 object
	return objects[0];
}
