//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"

//#ifdef USE_IK

#include <IL/il.h>
#include <IL/ilut.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <fltk/run.h>
#include <fltk/file_chooser.h>
#include <fltk/ColorChooser.h>

#include "EditorDef.h"

#include "EditorUI.h"
#include "Util.h"
#include "ModelDrawer.h"
#include "CurvedSurface.h"
#include "ObjectView.h"
#include "Texture.h"
#include "CfgParser.h"
#include "BackupManager.h"

#include <fstream>
#include "creg/Serializer.h"

#include "AnimationUI.h"
#include "FileSearch.h"
#include "MeshIterators.h"

extern "C"{
#include "lualib.h"
}

#include "swig/ScriptInterface.h"


const char* ViewSettingsFile="data/views.cfg";
const char* ArchiveListFile="data/archives.cfg";
const char* TextureGroupConfig="data/texgroups.cfg";

string applicationPath;

/*
 * 	"All Supported (*.{bmp,gif,jpg,png})"
	"All Files (*)\0"
 */

const char* FileChooserPattern=
	"Spring model (S3O)\0s3o\0"
	"Total Annihilation model (3DO)\0 3do\0"
	"3D Studio (3DS)\0 3ds\0"
	"Wavefront OBJ\0obj\0"
	;

// ------------------------------------------------------------------------------------------------
// ArchiveList
// ------------------------------------------------------------------------------------------------

bool ArchiveList::Load () {
	string fn=applicationPath + ArchiveListFile;
	CfgList *cfg=CfgValue::LoadFile ( fn.c_str() );

	if (!cfg)
		return false;

	CfgList *archs = dynamic_cast<CfgList*>(cfg->GetValue("Archives"));
	if (archs) {
		for (list<CfgListElem>::iterator i=archs->childs.begin();i!=archs->childs.end();++i) {
			CfgLiteral *lit = dynamic_cast<CfgLiteral*>(i->value);
			if (lit) archives.insert (lit->value);
		}
	}
	delete cfg;
	return true;
}

bool ArchiveList::Save () {
	string fn=applicationPath + ArchiveListFile;
	CfgWriter writer (fn.c_str());
	if (writer.IsFailed())
		return false;

	CfgList cfg;
	CfgList *archs=new CfgList;
	cfg.AddValue ("Archives", archs);
	int index=0;
	for (set<string>::iterator s=archives.begin();s!=archives.end();++s, index++) {
		char tmp[20];
		sprintf (tmp, "arch%d", index);
		archs->AddLiteral (tmp,s->c_str());
	}
	cfg.Write (writer,true);
	return true;
}
// ------------------------------------------------------------------------------------------------
// Editor Callback
// ------------------------------------------------------------------------------------------------


vector<EditorViewWindow *> EditorUI::EditorCB::GetViews ()
{
	vector<EditorViewWindow *> vl;
	for (int a=0;a<ui->viewsGroup->children();++a)
		vl.push_back( (EditorViewWindow *)ui->viewsGroup->child(a));
	return vl;
}

void EditorUI::EditorCB::MergeView (EditorViewWindow *own, EditorViewWindow *other)
{
	/*int w = other->w ();*/
	if (own->x () > other->x()) own->set_x(other->x());
	if (own->y () > other->y()) own->set_y(other->y());
	if (own->r () < other->r()) own->set_r(other->r());
	if (own->b () < other->b()) own->set_b(other->b());
	delete other;
	ui->Update();
}

void EditorUI::EditorCB::AddView (EditorViewWindow *v)
{
	v->editor=this;
	ui->viewsGroup->add (v);
	ui->Update();
}

Tool *EditorUI::EditorCB::GetTool() 
{
	if (ui->currentTool) ui->currentTool->editor=this; 
	return ui->currentTool;
}

// ------------------------------------------------------------------------------------------------
// EditorUI
// ------------------------------------------------------------------------------------------------

static void EditorUIProgressCallback (float part, void *data)
{
	EditorUI *ui = (EditorUI *)data;
	ui->progress->position (part);
	ui->progress->redraw();
}

static void LogCallbackProc(LogNotifyLevel level, const char *str, void* /*user_data*/)
{
	if (level == NL_Warn || level == NL_Error)
		fltk::message (str);
}

void EditorUI::Initialize ()
{
	optimizeOnLoad=true;

	archives.Load ();

	ilInit ();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);

	callback.ui = this;
	new BackupManager(&callback);
	
	objectViewer = new ObjectView (this, objectTree);
	uiIK = new IK_UI (&callback);
	uiTimeline = new TimelineUI (&callback);
	uiAnimTrackEditor = new AnimTrackEditorUI (&callback, uiTimeline);
	uiRotator = new RotatorUI;
	uiRotator->CreateUI(&callback);
	uiBackupViewer = new BackupViewerUI(&callback);

	tools.SetEditor (&callback);
	tools.camera->button = selectCameraTool;
	tools.move->button = selectMoveTool;
	tools.rotate->button = selectRotateTool;
	tools.scale->button = selectScaleTool;
	tools.texmap->button = selectTextureTool;
	tools.color->button = selectColorTool;
	tools.flip->button = selectFlipTool;
	tools.originMove->button = selectOriginMoveTool;
	tools.rotateTex->button = selectRotateTexTool;
	tools.toggleCurvedPoly->button = selectCurvedPolyTool;
	tools.LoadImages();

	currentTool = tools.GetDefaultTool ();
	modelDrawer = new ModelDrawer;
	model = 0;
	SetModel (new Model);
	UpdateTitle();

	textureHandler = new TextureHandler ();

	for (set<string>::iterator arch=archives.archives.begin();arch!=archives.archives.end();++arch)
		textureHandler->Load (arch->c_str());

	textureGroupHandler = new TextureGroupHandler (textureHandler);
	textureGroupHandler->Load ((applicationPath + TextureGroupConfig).c_str());
	
	UpdateTextureGroups();
	InitTexBrowser();

	uiMapping = new MappingUI (&callback);
	uiTexBuilder = new TexBuilderUI (0,0);

	LoadSettings();

	// create 4 views if no views were specified in the config (ie: there was no config)
	if (!viewsGroup->children()) {
		viewsGroup->begin();

		EditorViewWindow *views[4];
		int vw = viewsGroup->w ();
		int vh = viewsGroup->h ();

		for (int a=0;a<4;a++)
		{
			int Xofs=(a&1)*vw/2;
			int Yofs=(a&2)*vh/4;
			int W=vw/2;
			int H=vh/2;
			if (Xofs) W = vw-Xofs;
			if (Yofs) H = vh-Yofs;
			views[a]=new EditorViewWindow (Xofs, Yofs, W,H, &callback);
			views[a]->SetMode(a);
			views[a]->bDrawRadius = false;
		}
		views[3]->rendermode = M3D_TEX;

		viewsGroup->end();
	}

	BACKUP_POINT("New model");
}

void EditorUI::Show(bool initial)
{
	if (initial) viewsGroup->suppressRescale=true;
	window->show();
	viewsGroup->suppressRescale=false;
	Update();
}


EditorUI::~EditorUI()
{
	SAFE_DELETE(uiMapping);
	SAFE_DELETE(uiIK);
	SAFE_DELETE(uiTimeline);
	SAFE_DELETE(uiTexBuilder);
	SAFE_DELETE(uiAnimTrackEditor);
	SAFE_DELETE(uiRotator);
	SAFE_DELETE(uiBackupViewer);

	if (textureGroupHandler) {
		textureGroupHandler->Save((applicationPath+TextureGroupConfig).c_str());
		delete textureGroupHandler;
		textureGroupHandler=0;
	}

	SAFE_DELETE(textureHandler);

	SAFE_DELETE(objectViewer);
	SAFE_DELETE(modelDrawer);
	SAFE_DELETE(model);

	delete &BackupManager::Get();

	ilShutDown ();
}

fltk::Color EditorUI::SetTeamColor () {
	fltk::Color r;
	if (fltk::color_chooser("Set team color:",r)) {
		teamColor=r;
		return r;
	}
	return teamColor;
}

void EditorUI::uiSetRenderMethod(RenderMethod rm)
{
	modelDrawer->SetRenderMethod(rm);
	viewsGroup->redraw();
}

static void CollectTextures(MdlObject *o, set<Texture*>& textures) {
	PolyMesh* pm = o->GetPolyMesh();
	if (pm) {
		for (unsigned int a=0;a<pm->poly.size();a++) {
			if (pm->poly[a]->texture) textures.insert(pm->poly[a]->texture.Get());
		}
	}
	for (unsigned int a=0;a<o->childs.size();a++)
		CollectTextures(o->childs[a],textures);
}

void EditorUI::uiAddUnitTextures()
{
	// go through all the textures used by the unit
	TextureGroup *cur = GetCurrentTexGroup();
	if (cur && model->root) {
		CollectTextures(model->root, cur->textures);
		InitTexBrowser();
	}
}

void EditorUI::uiCut ()
{
	copyBuffer.Cut (model);
	BACKUP_POINT("Cut selected objects");
	Update();
}

void EditorUI::uiCopy ()
{
	copyBuffer.Copy (model);
	Update();
}

void EditorUI::uiPaste ()
{
	vector<MdlObject*> sel = model->GetSelectedObjects();
	MdlObject *where=0;
	if (!sel.empty()) {
		if (sel.size()==1)
			where=sel.front();
		else {
			fltk::message("You can't select multiple objects to paste in.");
			return;
		}
	}

	copyBuffer.Paste (model, where);
	BACKUP_POINT("Pasted copy buffer into model");
	Update();
}

void EditorUI::uiAddObject ()
{
	const char *r = fltk::input("Add empty object", 0);
	if (r) {
		MdlObject *obj=new MdlObject;
		obj->name = r;

		if (model->root) {
			MdlObject *parent=GetSingleSel ();
			if (!parent) 
				delete obj;
			else {
				obj->parent = parent;
				parent->childs.push_back (obj);
				parent->isOpen=true;
			}
		} else 	model->root=obj;

		BACKUP_POINT("Added empty object");
		Update ();
	}
}

void EditorUI::uiDeleteSelection()
{
	if (currentTool->needsPolySelect ()) {
		vector <MdlObject *> objects=model->GetObjectList ();
		for (unsigned int a=0;a<objects.size();a++) {
			PolyMesh *pm = objects[a]->GetPolyMesh();

			if (pm)
			{
				vector <Poly *> polygons;
				for (unsigned int b=0;b<pm->poly.size();b++) {
					Poly *pl = pm->poly[b];
					if (!pl->isSelected) polygons.push_back (pl);
					else delete pl;
				}
				pm->poly = polygons;
			}
		}
		BACKUP_POINT("Deleted selected polygons");
	} else {
		vector <MdlObject *> objects=model->GetSelectedObjects ();
		vector <MdlObject *> filtered;
		for (unsigned int a=0;a<objects.size();a++) {
			if (objects[a]->HasSelectedParent ())
				continue;
			filtered.push_back(objects[a]);
		}
		for (unsigned int a=0;a<filtered.size();a++)
			model->DeleteObject (filtered[a]);

		BACKUP_POINT("Deleted selected objects");
	}
	Update();
}

void EditorUI::uiApplyTransform()
{
	// apply the transformation of each object onto itself, and remove the orientation+offset
	vector<MdlObject*>sel = model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++)
	{
		MdlObject *o=sel[a];
		o->ApplyTransform(true,true,true);
	}
	BACKUP_POINT("Transformation applied");
	Update();
}

void EditorUI::uiUniformScale()
{
	vector<MdlObject*> sel=model->GetSelectedObjects();
	if (sel.empty()) return;

	const char *scalestr = fltk::input("Scale selected objects with this factor", "1");
	if (scalestr) {

		float scale=atof(scalestr);

		for (unsigned int a=0;a<sel.size();a++) {
			if (sel[a]->HasSelectedParent ())
				continue;

			sel[a]->scale *= scale;
		}
		BACKUP_POINT("Uniform scale applied");
		Update();
	}
}

void EditorUI::uiRotate3DOTex ()
{
	if (!currentTool->needsPolySelect())
		fltk::message ("Use the polygon selection tool first to select the polygons");
	else {
		vector<MdlObject*> obj = model->GetObjectList ();
		for(vector<MdlObject*>::iterator o=obj.begin();o!=obj.end();++o) {
			PolyMesh* pm = (*o)->GetPolyMesh();

			if(pm) 
				for (unsigned int a=0;a<pm->poly.size();a++) {
					Poly *pl = pm->poly[a];
					if (pl->isSelected)
						pl->RotateVerts();
				}
		}
		BACKUP_POINT("3DO texture rotated");
		Update();
	}
}

void EditorUI::uiSwapObjects()
{
	vector<MdlObject*> sel = model->GetSelectedObjects ();
	if (sel.size ()!=2) {
		fltk::message ("2 objects have to be selected for object swapping");
		return;
	}

	model->SwapObjects(sel.front(), sel.back());
	sel.front()->isOpen = true;
	sel.back()->isOpen = true;
	BACKUP_POINT("Objects swapped");
	Update();
}

void EditorUI::uiObjectPositionChanged(int axis, fltk::Input *o)
{
	MdlObject *sel = GetSingleSel();

	if (sel) {
		float val=atof(o->value());

		if (chkOnlyMoveOrigin->value())
		{
			Vector3 newPos = sel->position;
			newPos[axis] = val;

			sel->MoveOrigin(newPos - sel->position);
		} else
			sel->position[axis] = val;
		BACKUP_POINT("Object position change");
		Update();
	}
}


void EditorUI::uiObjectStateChanged(Vector3 MdlObject::*state, float Vector3::*axis, fltk::Input *o, float scale)
{
	MdlObject *sel = GetSingleSel();

	if (sel) {
		float val=atof(o->value());
		(sel->*state).*axis = val * scale;
		BACKUP_POINT("Object property change");
		Update();
	}
}

void EditorUI::uiObjectRotationChanged(int axis, fltk::Input *o)
{
	MdlObject *sel = GetSingleSel();

	if (sel) {
		Vector3 euler = sel->rotation.GetEuler();
		euler.v[axis] = atof(o->value()) * DegreesToRadians;
		sel->rotation.SetEuler(euler);
		BACKUP_POINT("Object rotation change");
		Update();
	}
}

void EditorUI::uiModelStateChanged(float *prop, fltk::Input *o)
{
	*prop = atof (o->value ());
	BACKUP_POINT("Model property change");
	Update ();
}

void EditorUI::uiCalculateMidPos ()
{
	model->EstimateMidPosition ();
	BACKUP_POINT("Midpoint estimation");
	Update ();
}

void EditorUI::uiCalculateRadius ()
{
	model->CalculateRadius();
	BACKUP_POINT("Radius calculation");
	Update ();
}

void EditorUI::menuObjectApproxOffset()
{
	vector<MdlObject*> sel = model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++)
		sel[a]->ApproximateOffset ();
	BACKUP_POINT("Offset approximation of selected objects");
	Update();
}

void EditorUI::browserSetObjectName (MdlObject *obj)
{
	if (!obj) return;

	const char *r = fltk::input("Set object name", obj->name.c_str());
	if (r) {
		string prev = obj->name;
		obj->name = r;
		BACKUP_POINT( string("Object name change ('" + prev + "' => '" + obj->name + "')").c_str() );
		objectTree->redraw();
	}
}


void EditorUI::SelectionUpdated()
{
	vector<MdlObject*> sel =model->GetSelectedObjects ();
	char label[128];
	if (sel.size () == 1) {
		MdlObject *f = sel.front();
		PolyMesh *pm = f->GetPolyMesh();
		sprintf (label, "MdlObject %s selected: position=(%4.1f,%4.1f,%4.1f) scale=(%3.2f,%3.2f,%3.2f) polycount=%lu vertexcount=%lu", f->name.c_str(),
			f->position.x, f->position.y, f->position.z, f->scale.x, f->scale.y, f->scale.z, pm? pm->poly.size():0, pm?pm->verts.size():0);
	} else if(sel.size() > 1) {
		int plcount = 0, vcount = 0;
		for (unsigned int a=0;a<sel.size();a++) {
			PolyMesh *pm = sel[a]->GetPolyMesh();
			if (pm) {
				plcount += pm->poly.size();
				vcount += pm->verts.size();
			}
		}
		sprintf (label, "Multiple objects selected. Total polycount: %d, Total vertexcount: %d", plcount, vcount);
	} else label[0]=0;
	status->value (label);
	status->redraw ();

	Update ();
}

void EditorUI::Update ()
{
	objectViewer->Update ();
	viewsGroup->redraw();
	uiIK->Update ();
	uiTimeline->Update ();
	uiAnimTrackEditor->Update ();
	uiRotator->Update();
	uiBackupViewer->Update();

	// update object inputs
	vector<MdlObject*> sel = model->GetSelectedObjects();
	fltk::Input *inputs[] = {
		inputPosX,inputPosY, inputPosZ,
			inputScaleX,inputScaleY,inputScaleZ,
			inputRotX,inputRotY,inputRotZ 
	};
	if (sel.size()==1) {
		MdlObject *s=sel.front();
		for (unsigned int a=0;a<sizeof(inputs)/sizeof(fltk::Input*);a++) inputs[a]->activate();
		inputPosX->value(s->position.x);
		inputPosY->value(s->position.y);
		inputPosZ->value(s->position.z);
		inputScaleX->value(s->scale.x);
		inputScaleY->value(s->scale.y);
		inputScaleZ->value(s->scale.z);
		Vector3 euler = s->rotation.GetEuler();
		inputRotX->value(euler.x / DegreesToRadians);
		inputRotY->value(euler.y / DegreesToRadians);
		inputRotZ->value(euler.z / DegreesToRadians);
	} else {
		for (unsigned int a=0;a<sizeof(inputs)/sizeof(fltk::Input*);a++) inputs[a]->deactivate();
	}
	// model inputs
	inputRadius->value(model->radius);
	inputHeight->value(model->height);
	inputCenterX->value(model->mid.x);
	inputCenterY->value(model->mid.y);
	inputCenterZ->value(model->mid.z);
}

void EditorUI::RenderScene (IView *view)
{
	if (model) {
		const float m=1/255.0f;
		Vector3 teamc((teamColor & 0xff000000) >> 24, (teamColor & 0xff0000)>>16, (teamColor&0xff00)>>8);
		modelDrawer->Render (model, view, teamc*m);
	}
}

// get a single selected object and abort when multiple are selected
MdlObject* EditorUI::GetSingleSel ()
{
	vector <MdlObject*> sel = model->GetSelectedObjects ();

	if (sel.size () != 1) {
		fltk::message ("Select a single object");
		return 0;
	}
	return sel.front();
}

void EditorUI::SetMapping (int mapping)
{
	model->mapping = mapping;
	viewsGroup->redraw();
	
	if (mapping == MAPPING_S3O) {
		texGroupS3O->activate ();
		texGroup3DO->deactivate ();
	} else {
		texGroupS3O->deactivate ();
		texGroup3DO->activate ();
	}

	mappingChooser->value(mapping);
	mappingChooser->redraw();
}

void EditorUI::SetModel (Model *mdl)
{
	SAFE_DELETE(model);
	model = mdl;

	objectViewer->Update();
	viewsGroup->redraw ();
	mappingChooser->value (mdl->mapping);
	SetMapping (mdl->mapping);

	inputTexture1->value(0);
	inputTexture2->value(0);

	for (unsigned int a=0;a<mdl->texBindings.size();a++)
		SetModelTexture (a, mdl->texBindings[a].texture.Get());

	Update ();
}

void EditorUI::SetTool (Tool *t)
{
	if (t) {
		t->editor = &callback;
		if (t->isToggle) {
			currentTool = t;
			tools.Disable();
			t->toggle(true);
		} else
			t->click();
		viewsGroup->redraw();
	}
}

void EditorUI::SetModelTexture (int index, Texture *tex)
{
	model->SetTexture(index,tex);

	if (tex && !tex->glIdent) {
		((EditorViewWindow *)viewsGroup->child(0))->make_current();
		tex->VideoInit ();
	}

	if (index < 2) {
		fltk::FileInput *input = index ? inputTexture2 : inputTexture1;
		input->value (tex ? tex->name.c_str() : "");
	}
}

void EditorUI::BrowseForTexture(int textureIndex)
{
	static std::string fn;
	if (FileOpenDlg ("Select texture:", ImageFileExt, fn))
	{
		Texture *tex = new Texture (fn);
		if (!tex->IsLoaded ()) 
			delete tex;
		else
			SetModelTexture (textureIndex, tex);
		BACKUP_POINT("Texture change");
		Update();
	}
}

void EditorUI::ReloadTexture (int index)
{
	//fltk::FileInput *input = index ? inputTexture2 : inputTexture1;

	if (model->texBindings.size () <= (uint)index)
		model->texBindings.resize (index+1);

	TextureBinding& tb = model->texBindings[index];
	RefPtr<Texture> tex = new Texture(tb.name);
	SetModelTexture (index, tex->IsLoaded () ? tex.Get() : 0);

	BACKUP_POINT("Texture reload");
	Update();
}

void EditorUI::ConvertToS3O()
{
	static int texW = 256;
	static int texH = 256;
	
	texW = atoi (fltk::input ("Enter texture width: ", SPrintf("%d", texW).c_str()));
	texH = atoi (fltk::input ("Enter texture height: ", SPrintf("%d", texH).c_str()));
	std::string name_ext = fltk::filename_name (filename.c_str());
	std::string name (name_ext.c_str(), fltk::filename_ext (name_ext.c_str()));
	if (model->ConvertToS3O(GetFilePath(filename) + "/" + name + "_tex.bmp", texW, texH)) {
		// Update the UI
		SetModelTexture (0, model->texBindings[0].texture.Get());
		SetMapping (MAPPING_S3O);
		BACKUP_POINT("Converted to S3O texturing");
	} else
		BackupManager::Get().ReloadLast ();
	Update();
}

// Update window title
void EditorUI::UpdateTitle ()
{
	const char *baseTitle="Upspring Model Editor - ";

	if (filename.empty()) windowTitle=baseTitle+string("unnamed model");
	else windowTitle = baseTitle + filename;

	window->label (windowTitle.c_str());
}

void EditorUI::UpdateTextureGroups()
{
	textureGroupMenu->clear();

	for (unsigned int a=0;a<textureGroupHandler->groups.size();a++) {
		TextureGroup *tg = textureGroupHandler->groups[a];
		textureGroupMenu->add (tg->name.c_str(),0,0,tg);
	}
	textureGroupMenu->redraw();
}

void EditorUI::SelectTextureGroup (fltk::Widget* /*w*/,void* /*d*/) 
{
	InitTexBrowser ();
}

TextureGroup* EditorUI::GetCurrentTexGroup()
{
	assert (textureGroupHandler->groups.size()==(uint)textureGroupMenu->children());
	if (textureGroupHandler->groups.empty()) 
		return 0;

	fltk::Widget *s = textureGroupMenu->child (textureGroupMenu->value());
	if (s) {
		TextureGroup *tg = (TextureGroup *)s->user_data();
		return tg;
	} 
	return 0;
}

void EditorUI::InitTexBrowser()
{
	texBrowser->clear();

	TextureGroup *tg = GetCurrentTexGroup();
	if(!tg) return;
	for (set<Texture*>::iterator t=tg->textures.begin();t!=tg->textures.end();++t)
		texBrowser->AddTexture(*t);
	texBrowser->UpdatePositions();
	texBrowser->redraw();
}


bool EditorUI::Load (const string& fn)
{
	Model *mdl = Model::Load(fn, optimizeOnLoad);

	if (mdl) { 
		filename = fn;
		SetModel (mdl);
		BACKUP_POINT("Load model");
		Update();
	} else {
		delete mdl;
	}
	return mdl!=0;
}


void EditorUI::menuObjectLoad()
{
	MdlObject* obj = GetSingleSel();
	if (!obj) return;

	static std::string fn;
	if (FileOpenDlg("Load object:", FileChooserPattern, fn)) {
		Model* submdl = Model::Load(fn);
		if (submdl)
			model->InsertModel (obj, submdl);

		delete submdl;
		BACKUP_POINT("Load object");
		Update();
	}
}

void EditorUI::menuObjectSave()
{
	MdlObject* sel = GetSingleSel();
	if (!sel) return;
	static std::string fn;
	char fnl [256];
	strncpy (fnl, fn.c_str(),sizeof(fnl));
	strncpy (fltk::filename_name (fnl), sel->name.c_str(), sizeof(fnl));
	fn = fnl;
	if (FileSaveDlg("Save object:", FileChooserPattern, fn))
	{
		Model *submdl = new Model;
		submdl->root = sel;
		Model::Save (submdl,fn);
		submdl->root = 0;
		delete submdl;
	}
}

void EditorUI::menuObjectReplace()
{
	MdlObject* old = GetSingleSel();
	if (!old) return;
	char buf[128];
	sprintf (buf, "Select model to replace \'%s\' ", old->name.c_str());
	static std::string fn;
	if (FileOpenDlg(buf, FileChooserPattern, fn)) {
		Model *submdl = Model::Load(fn);
		if (submdl) {
			model->ReplaceObject (old, submdl->root);
			submdl->root = 0;

			BACKUP_POINT("Replace object");
			Update ();
		}
		delete submdl;
	}
}

void EditorUI::menuObjectMerge()
{
	vector <MdlObject*> sel = model->GetSelectedObjects ();

	for (unsigned int a=0;a<sel.size();a++) {
		MdlObject *parent = sel [a]->parent;
		if (parent) parent->MergeChild (sel[a]);
	}
	BACKUP_POINT("Selected objects merged");
	Update ();
}

void EditorUI::menuObjectFlipPolygons()
{
	vector <MdlObject*> sel=model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++) {
		for (PolyIterator pi(sel[a]); !pi.End(); pi.Next()) 
			pi->Flip();
		
		sel[a]->InvalidateRenderData ();
	}
	BACKUP_POINT("Flip polygons of selected objects");
	Update ();
}

void EditorUI::menuObjectRecalcNormals()
{
	vector <MdlObject*> sel=model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++) {
		PolyMesh *pm = sel[a]->GetPolyMesh();
		if (pm) pm->CalculateNormals();

		sel[a]->InvalidateRenderData ();
	}
	BACKUP_POINT("Normal recalculation");
	Update();
}

void EditorUI::menuObjectResetScaleRot()
{
	vector<MdlObject*> sel=model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++) 	{
        sel[a]->rotation=Rotator();
		sel[a]->scale.set(1,1,1);
	}
	BACKUP_POINT("Reset scale+rotation of selected objects");
	Update();
}

void EditorUI::menuObjectResetTransform()
{
	vector<MdlObject*> sel=model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++) 	{
        sel[a]->position=Vector3();
        sel[a]->rotation=Rotator();
		sel[a]->scale.set(1,1,1);
	}
	BACKUP_POINT("Reset transform of selected objects");
	Update();
}

void EditorUI::menuObjectResetPos()
{
	vector<MdlObject*> sel=model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++)
        sel[a]->position=Vector3();
	BACKUP_POINT("Reset position of selected objects");
	Update();
}

void EditorUI::menuObjectShowAnimWindows()
{
	uiIK->Show ();
	uiTimeline->Show ();
}

void EditorUI::menuObjectGenCSurf()
{
	vector<MdlObject*> sel=model->GetSelectedObjects();
	for (unsigned int a=0;a<sel.size();a++) {
		delete sel[a]->csurfobj;

		sel[a]->csurfobj = new csurf::Object;
		sel[a]->csurfobj->GenerateFromPolyMesh(sel[a]->GetPolyMesh());
	}
	BACKUP_POINT("Curved surface object created");
}

void EditorUI::menuEditOptimizeAll()
{
	vector<MdlObject *> objects = model->GetObjectList();
	for (uint i=0;i<objects.size();i++)
		if (objects[i]->GetPolyMesh ())
			objects[i]->GetPolyMesh()->Optimize(PolyMesh::IsEqualVertexTCNormal);
	BACKUP_POINT("All objects optimized");
}

void EditorUI::menuEditOptimizeSelected()
{
	vector<MdlObject *> objects = model->GetObjectList();
	for (uint i=0;i<objects.size();i++)
		if (objects[i]->isSelected && objects[i]->GetPolyMesh())
			objects[i]->GetPolyMesh()->Optimize(PolyMesh::IsEqualVertexTCNormal);
	BACKUP_POINT("Selected objects optimized");
}

void EditorUI::menuEditUndo()
{
	BackupManager::Get().Undo();
}

void EditorUI::menuEditRedo()
{
	BackupManager::Get().Redo();
}

void EditorUI::menuEditShowUndoBufferViewer()
{
	uiBackupViewer->Show();
}


void EditorUI::menuFileSaveAs() 
{
	static std::string fn;
	if (FileSaveDlg("Save model file:", FileChooserPattern, fn))
	{
		filename=fn;
		Model::Save(model,fn);
		UpdateTitle();
	}
}

void EditorUI::menuFileNew()
{
	SetModel (new Model);
	BACKUP_POINT("New model");
}

// this will also be called by the main window callback (on exit)
void EditorUI::menuFileExit()
{
	uiIK->Hide();
	uiMapping->Hide();
	uiTimeline->Hide();
	uiAnimTrackEditor->Hide();
	uiRotator->Hide();
	uiBackupViewer->Hide();
	window->hide ();
}

void EditorUI::menuFileLoad()
{
	static std::string fn;
	const bool r = FileOpenDlg("Load model file:", FileChooserPattern, fn);

	if (r) {
		Load(fn);
		UpdateTitle();
	}
}

void EditorUI::menuFileSave()
{
	if(filename.empty()) {
		menuFileSaveAs();
		return;
	}

	Model::Save (model,filename.c_str());
}


void EditorUI::menuSettingsShowArchiveList()
{
	ArchiveListUI sui(archives);
	if (sui.window->exec()) {
		archives=sui.settings;
		archives.Save ();

		for (set<string>::iterator arch=archives.archives.begin();arch!=archives.archives.end();++arch)
			textureHandler->Load (arch->c_str());
	}
}

// Show texture group window
void EditorUI::menuSettingsTextureGroups()
{
	TexGroupUI texGroupUI (textureGroupHandler, textureHandler);
	textureGroupHandler->Save ((applicationPath+TextureGroupConfig).c_str());

	UpdateTextureGroups();
	InitTexBrowser();
}

void EditorUI::menuSettingsSetBgColor()
{
	Vector3 col;
	if (fltk::color_chooser ("Select view background color:", col.x,col.y,col.z))
	{
		for (int a=0;a<viewsGroup->children();++a)
			((EditorViewWindow *)viewsGroup->child(a))->backgroundColor = col;
		viewsGroup->redraw();
	}
}

void EditorUI::menuSetSpringDir()
{
	SelectDirectory (
		"Select S3O texture loading directory\n"
		"(for example c:\\games\\spring\\unittextures)", 
		Texture::textureLoadDir);
}

void EditorUI::menuSettingsRestoreViews()
{
	LoadSettings();
}

void EditorUI::SaveSettings()
{
	string path=applicationPath + ViewSettingsFile;
	CfgWriter writer(path.c_str());
	if (writer.IsFailed ()) {
		fltk::message ("Failed to open %s for writing view settings", path.c_str());
		return;
	}
	CfgList cfg;
	SerializeConfig (cfg, true);
	cfg.Write(writer,true);
}

void EditorUI::LoadSettings()
{
	string path=applicationPath + ViewSettingsFile;
	CfgList *cfg = CfgValue::LoadFile (path.c_str());
	if (cfg) {
		SerializeConfig (*cfg, false);
		delete cfg;
	}
}

void EditorUI::LoadToolWindowSettings()
{
	string path=applicationPath + ViewSettingsFile;
	CfgList *cfg = CfgValue::LoadFile (path.c_str());
	if (cfg) {
		// tool window visibility
		bool showTimeline,showTrackEdit,showIK;
		CFG_LOAD(*cfg,showTimeline);
		CFG_LOAD(*cfg,showTrackEdit);
		CFG_LOAD(*cfg,showIK);
		if (showTrackEdit) uiAnimTrackEditor->Show();
		if (showIK) uiIK->Show();
		if (showTimeline) uiTimeline->Show();

		delete cfg;
	}
}

void EditorUI::SerializeConfig (CfgList& cfg, bool store)
{
	// Serialize editor window properties
	int x=window->x(), y=window->y(), width=window->w(), height=window->h();
	string& springTexDir = Texture::textureLoadDir;
	if (store) {
		CFG_STOREN(cfg,x);
		CFG_STOREN(cfg,y);
		CFG_STOREN(cfg,width);
		CFG_STOREN(cfg,height);
		CFG_STORE(cfg,springTexDir);
		CFG_STORE(cfg,optimizeOnLoad);
		bool showTimeline=uiTimeline->window->visible();
		CFG_STORE(cfg,showTimeline);
		bool showTrackEdit=uiAnimTrackEditor->window->visible();
		CFG_STORE(cfg,showTrackEdit);
		bool showIK=uiIK->window->visible();
		CFG_STORE(cfg,showIK);
	} else {
		CFG_LOADN(cfg,x);
		CFG_LOADN(cfg,y);
		CFG_LOADN(cfg,width);
		CFG_LOADN(cfg,height);
		CFG_LOAD(cfg,springTexDir);
		CFG_LOAD(cfg,optimizeOnLoad);

		window->resize(x,y,width,height);
	}
	// Serialize views: first match the number of views specified in the config
	int numViews = viewsGroup->children();
	if (store) {
		CFG_STOREN(cfg,numViews);
	} else {
		CFG_LOADN(cfg,numViews);
		int nv=numViews-viewsGroup->children();
		if (nv > 0) {
			viewsGroup->begin();
			for (int a=0;a<nv;a++) {
				EditorViewWindow *wnd = new EditorViewWindow (0,0,0,0,&callback);
				wnd->SetMode(a%4);
			}
			viewsGroup->end();
		} else {
			for (int a=viewsGroup->children()-1;a>=numViews;a--) {
				EditorViewWindow *wnd = (EditorViewWindow *)viewsGroup->child(a);
				viewsGroup->remove (wnd);
				delete wnd;
			}
		}
	}
	// then serialize view settings for each view in the config
	int viewIndex = 0;
	char viewName[16];
	for (int a=0;a<viewsGroup->children();++a) {
		sprintf (viewName, "View%d", viewIndex++);
		CfgList *viewcfg;
		if (store) {
			viewcfg = new CfgList;
			cfg.AddValue (viewName, viewcfg);
		}else viewcfg=dynamic_cast<CfgList*>(cfg.GetValue(viewName));
		if (viewcfg) 
			((EditorViewWindow *)viewsGroup->child(a))->Serialize (*viewcfg,store);
	}
}


void EditorUI::menuMappingImportUV() {
	static std::string fn;
	if (FileOpenDlg("Select model to copy UV coordinates from:", FileChooserPattern, fn))
	{
		IProgressCtl progctl;

		progctl.cb = EditorUIProgressCallback;
		progctl.data = this;

		progress->range (0.0f, 1.0f, 0.01f);
		progress->position (0.0f);
		progress->show ();
		model->ImportUVMesh(fn.c_str(), progctl);
		progress->hide ();
		BACKUP_POINT("UV mapping imported");
	}
}

void EditorUI::menuMappingExportUV ()
{
	static std::string fn;
	if (FileSaveDlg("Save merged model for UV mapping:", FileChooserPattern, fn))
		model->ExportUVMesh (fn.c_str());
}

void EditorUI::menuMappingShow ()
{
	uiMapping->Show ();
}

void EditorUI::menuMappingShowTexBuilder ()
{
	uiTexBuilder->Show ();
}

void EditorUI::menuScriptLoad()
{
	const char *pattern ="Lua script (LUA)\0*.lua\0";

	static std::string lastLuaScript;
	if (FileOpenDlg("Load script:", pattern, lastLuaScript))
	{
		if (luaL_dofile(luaState, lastLuaScript.c_str()) != 0)
		{
			const char *err = lua_tostring(luaState, -1);
			fltk::message("Error while executing %s: %s", lastLuaScript.c_str(), err);
		}
	}
}

static EditorUI* editorUI = 0;

static void scriptClickCB(fltk::Widget* /*w*/, void *d)
{
	ScriptedMenuItem *s= (ScriptedMenuItem*)d;

	char buf[64];
	SNPRINTF(buf, sizeof(buf), "%s()", s->funcName.c_str());
	if (luaL_dostring(editorUI->luaState, buf) != 0)
	{
		const char *err = lua_tostring(editorUI->luaState, -1);
		fltk::message("Error while executing %s: %s",s->name.c_str(), err);
	}

	editorUI->Update();
}

void upsAddMenuItem(const char *name, const char *funcName)
{
	ScriptedMenuItem *s = new ScriptedMenuItem;
	s->funcName = funcName;
	editorUI->scripts.push_back(s);
	
	fltk::Widget* w = editorUI->menu->add(name, 0, scriptClickCB, s);
	s->name = w->label();
}

IEditor* upsGetEditor()
{
	return &editorUI->callback;
}

void EditorUI::menuHelpAbout ()
{
	fltk::message("Upspring, created by Jelmer Cnossen\n"
		"Model editor for the Spring RTS project: spring.clan-sy.com\n"
		"Using FLTK, lib3ds, GLEW, zlib, and DevIL/OpenIL");
}



// ------------------------------------------------------------------------------------------------
// Application Entry Point
// ------------------------------------------------------------------------------------------------


/*#ifdef WIN32

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	EditorUI editor;
	editor.Show();
	return fltk::run ();
}

#else*/

void PrintCmdLine ()
{
	printf (
		"Upspring command line:\n"
		"-run luafile\t\tRuns given lua script and exits.\n"
		);
}

string ReadTextFile (const char *name)
{
	FILE *f = fopen(name, "rb");
	if (!f) {
		logger.Trace (NL_Error, "Can't open file %s\n", name);
		return string();
	}
	int l;
	fseek (f, 0, SEEK_END);
	l = ftell(f);
	fseek (f, 0, SEEK_SET);
	string r;
	r.resize (l);
	if (fread (&r[0], l, 1, f)) {}
	fclose (f);

	return r;
}

bool ParseCmdLine(int argc, char *argv[], int& /*r*/)
{
	for (int a=1;a<argc;a++) {
		if (!STRCASECMP(argv[a], "-run")) {
			if (a == argc-1)  {
				PrintCmdLine ();
				return false;
			}

			/*
			const char *scriptFile = argv[a+1];
			r = RunScript (binder, scriptFile);
			*/
			return false;
		}
	}

	return true;
}

extern void math_test();

extern "C" int luaopen_upspring(lua_State *L);

int main (int argc, char *argv[])
{
#ifdef WIN32
	if (__argv[0]) {
		applicationPath = __argv[0];
		applicationPath.erase (applicationPath.find_last_of ('\\')+1, applicationPath.size());
	}
#else
	if (argv[0]) {
		applicationPath = argv[0];
		applicationPath.erase (applicationPath.find_last_of ('/')+1, applicationPath.size());
	}
#endif

	// Setup logger callback, so error messages are reported with fltk::message
	logger.AddCallback (LogCallbackProc, 0);
#ifdef _DEBUG
	logger.SetDebugMode (true);
#endif

//	math_test();
	// fltk::message( "path: %s", applicationPath.c_str() );

	// Initialize the class system
	creg::System::InitializeClasses ();

	int r = 0;
	if (ParseCmdLine(argc, argv, r))
	{
		// Bring up the main editor dialog
		EditorUI editor;
		editorUI = &editor;
		editor.Show(true);
		editor.LoadToolWindowSettings();

		// Initialise Lua
		lua_State *L = lua_open();
		luaL_openlibs(L);
		luaopen_upspring(L);

		editor.luaState = L;
		
		if (luaL_dofile(L, "scripts/init.lua") != 0) {
			const char *err = lua_tostring(L, -1);
			fltk::message("Error while executing init.lua: %s", err);
		}

		// NOTE: the "scripts/plugins" literal was changed to "scripts/plugins/"
		std::list<std::string>* luaFiles = FindFiles("*.lua", false,
#ifdef WIN32
			"scripts\\plugins");
#else
			"scripts/plugins/");
#endif

		for (std::list<std::string>::iterator i = luaFiles->begin(); i != luaFiles->end(); ++i) {
			if (luaL_dofile(L, i->c_str()) != 0) {
				const char *err = lua_tostring(L, -1);
				fltk::message("Error while executing \'%s\': %s", i->c_str(), err);
			}
		}

		fltk::run();
	}

	// Shutdown scripting system and class system
	creg::System::FreeClasses ();

	return r;
}

//#endif


