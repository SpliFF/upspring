//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

// All this code is included in the FLTK-generated EditorUI.h, and will be inside the EditorUI class
public:
~EditorUI();

// Menu callbacks
void menuFileLoad();
void menuFileSave();
void menuFileSaveAs();
void menuFileNew();
void menuFileExit();
void menuHelpAbout();
void menuObjectLoad();
void menuObjectSave();
void menuObjectReplace();
void menuObjectMerge();
void menuObjectLoad3DS();
void menuObjectSave3DS();
void menuObjectApproxOffset();
void menuObjectResetPos();
void menuObjectResetTransform();
void menuObjectResetScaleRot();
void menuObjectFlipPolygons();
void menuObjectRecalcNormals();
void menuObjectShowAnimWindows();
void menuObjectGenCSurf();
void menuEditOptimizeAll();
void menuEditOptimizeSelected();
void menuEditUndo();
void menuEditRedo();
void menuEditShowUndoBufferViewer();
void menuMappingImportUV();
void menuMappingExportUV();
void menuMappingShow();
void menuMappingShowTexBuilder();
void menuSettingsShowArchiveList();
void menuSettingsTextureGroups();
void menuSettingsRestoreViews();
void SaveSettings();
void menuSettingsSetBgColor();
void menuSetSpringDir();
void menuScriptLoad();

// Function callbacks for the UI components
void uiAddObject();
void uiCloneObject();
void uiCopy();
void uiCut();
void uiPaste();
void uiSwapObjects();
void uiDeleteSelection();
void uiApplyTransform();
void uiUniformScale();
void uiCalculateMidPos ();
void uiCalculateRadius ();
void uiSetRenderMethod (RenderMethod rm);

void uiRotate3DOTex (); // rotate tool button

#define DegreesToRadians (3.141593f / 180.0f)
void uiObjectStateChanged(Vector3 MdlObject::*state, float Vector3::*axis,fltk::Input *o, float scale=1.0f);
void uiModelStateChanged(float *prop, fltk::Input *o);
void uiObjectRotationChanged(int axis, fltk::Input* o);
void uiObjectPositionChanged(int axis, fltk::Input* o);
void browserSetObjectName(MdlObject *obj);
void uiAddUnitTextures();

void Show(bool initial);
void Update ();
void Initialize ();
void UpdateTitle ();
bool Save ();
bool Load (const std::string& fn);
void SetModel (Model *mdl);
void SetTool (Tool *t);
void RenderScene (IView *view);
void SetMapping(int mapping);
void BrowseForTexture (int index);
void SetModelTexture (int index, Texture *tex);
void ReloadTexture (int index);
void ConvertToS3O();
MdlObject *GetSingleSel ();
void SelectionUpdated();
// Texture groups
void UpdateTextureGroups();
void SelectTextureGroup(fltk::Widget *w, void *d);
void InitTexBrowser();
TextureGroup* GetCurrentTexGroup();
fltk::Color SetTeamColor();
void LoadSettings();
void LoadToolWindowSettings();
void SerializeConfig(CfgList& cfg, bool store);

fltk::Color teamColor;
std::string filename, windowTitle;
Model *model;
ModelDrawer* modelDrawer;
Tool *currentTool;

Tools tools;

class ObjectView;
ObjectView* objectViewer;

class EditorCB : public IEditor { 
public:
	EditorUI *ui;

	void RedrawViews () { ui->Update(); }
	std::vector<EditorViewWindow *> GetViews ();
	void SelectionUpdated() {ui->SelectionUpdated();}
	Model* GetMdl() { return ui->model; }
	Tool* GetTool();
	void RenderScene (IView *view) { ui->RenderScene(view); }
	TextureHandler* GetTextureHandler() { return ui->textureHandler; }
	void SetTextureSelectCallback (void (*cb)(Texture*,void*),void *d) { 
		ui->texBrowser->SetSelectCallback( cb, d );
	}
	float GetTime () { return ui->uiTimeline->GetTime(); }
	void MergeView(EditorViewWindow *own, EditorViewWindow *other);
	void AddView(EditorViewWindow *v);
	void SetModel(Model *mdl) { ui->SetModel(mdl); }
};

EditorCB callback;
CopyBuffer copyBuffer;
ArchiveList archives;

TextureHandler *textureHandler;
TextureGroupHandler *textureGroupHandler;

BackupViewerUI *uiBackupViewer;
RotatorUI *uiRotator;
IK_UI *uiIK;
TimelineUI *uiTimeline;
MappingUI* uiMapping;
TexBuilderUI* uiTexBuilder;
AnimTrackEditorUI* uiAnimTrackEditor;
bool optimizeOnLoad;

lua_State *luaState;
std::vector<ScriptedMenuItem*> scripts;
