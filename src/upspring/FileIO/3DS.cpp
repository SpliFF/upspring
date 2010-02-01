//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"
#include "Model.h"
#include "Util.h"

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>

static inline void removeTransform(MdlObject *obj) 
{
	obj->ApplyTransform(true,true,true);
}

static Lib3dsMesh *ConvertObjTo3DS (MdlObject *obj)
{
	Lib3dsMesh *mesh = lib3ds_mesh_new(obj->name.c_str());

	PolyMesh *pm = obj->GetPolyMesh ();
	if (pm && !pm->verts.empty()) {
		lib3ds_mesh_new_point_list(mesh, pm->verts.size());
		lib3ds_mesh_new_texel_list(mesh, pm->verts.size());
		for(uint v=0;v<pm->verts.size();v++)
		{
			for(uint axis=0;axis<3;axis++)
				mesh->pointL[v].pos[axis] = pm->verts[v].pos[axis];
			mesh->texelL[v][0] = pm->verts[v].tc[0].x;
			mesh->texelL[v][1] = pm->verts[v].tc[0].y;
		}

		Lib3dsDword numFaces=0;
		for(uint p=0;p<pm->poly.size();p++)
			numFaces+=pm->poly[p]->verts.size()-2;

		lib3ds_mesh_new_face_list(mesh, numFaces);
		Lib3dsDword curFace=0;
		for(uint p=0;p<pm->poly.size();p++)
		{
			Poly *pl = pm->poly [p];
			for(uint v=2;v<pl->verts.size();v++)
			{
				mesh->faceL[curFace].points[0] = pl->verts[0];
				mesh->faceL[curFace].points[1] = pl->verts[v-1];
				mesh->faceL[curFace].points[2] = pl->verts[v];
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

	for (uint i = 0; i < objList.size(); i++)
	{
		Lib3dsMesh *mesh = ConvertObjTo3DS (objList[i]);
		lib3ds_file_insert_mesh (file, mesh);
	}

	Lib3dsBool r = lib3ds_file_save (file, fn);

	// cleanup
	delete cl;
	lib3ds_file_free(file);

	return !!r;
}

static int count_meshes(Lib3dsMesh *meshList)
{
	int c=0;
	while (meshList) {
		meshList = meshList->next;
		c ++;
	}
	return c;
}

static MdlObject *Convert3DSToObj (Lib3dsMesh *mesh)
{
	MdlObject *obj = new MdlObject();
	PolyMesh *pm = obj->GetOrCreatePolyMesh ();

	if (!mesh->faces)
		return 0;
	
	std::vector <Vertex> orgvrt;
	orgvrt.resize(mesh->points);
	for (uint i = 0; i < mesh->points; i++) {
		if (mesh->texels == mesh->points) {
			orgvrt [i].tc[0].x = mesh->texelL[i][0];
			orgvrt [i].tc[0].y = mesh->texelL[i][1];
		}
		for (uint x = 0; x < 3; x++)
			orgvrt [i].pos [x] = mesh->pointL [i].pos[x];
	}

	pm->verts.resize (mesh->faces * 3);
	pm->poly.resize (mesh->faces);

	Lib3dsVector *normals = new Lib3dsVector [mesh->faces * 3];
	lib3ds_mesh_calculate_normals(mesh, normals);

	for (uint i = 0; i < mesh->faces; i++) {
		Poly* pl = new Poly();
		pl->verts.resize(3);

		Lib3dsFace *face = &mesh->faceL [i];
		for (uint f = 0; f < 3; f++) {
			pl->verts[f] = i * 3 + f;
			Vertex& vrt = pm->verts[pl->verts[f]];
			
			vrt = orgvrt [face->points[f]];
			for (uint axis=0;axis<3;axis++)
				vrt.normal[axis] = normals[i*3+f][axis];
		}

		pm->poly[i] = pl;
	}

	delete[] normals;
	obj->name = mesh->name;

	return obj;
}

MdlObject *Load3DSObject(const char *fn, IProgressCtl& /*progctl*/)
{
	Lib3dsFile *file = lib3ds_file_load(fn);
	if (!file)
		return 0;

	int method = 2;

	int meshCount = count_meshes(file->meshes);
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

	for (Lib3dsMesh *mesh = file->meshes; mesh; mesh = mesh->next)
	{
		// abort if method 2 is used
		if (method == 2 && mesh != file->meshes)
			break;

		MdlObject *o = Convert3DSToObj (mesh);
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
