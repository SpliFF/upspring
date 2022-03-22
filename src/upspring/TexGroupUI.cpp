//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "EditorUI.h"
#include "CfgParser.h"

TexGroupUI::TexGroupUI(TextureGroupHandler *tgh, TextureHandler *th) 
	: texGroupHandler(tgh), textureHandler(th)
{
	CreateUI ();

	for (map<string, TextureHandler::TexRef>::iterator ti=th->textures.begin();ti != th->textures.end(); ++ti)
		texBrowser->AddTexture (ti->second.texture.Get());

	texBrowser->UpdatePositions();
	current=0;
	UpdateGroupList();

	window->exec ();
}

TexGroupUI::~TexGroupUI()
{
	delete window;
}

void TexGroupUI::SelectGroup()
{
	int index = groups->value ();
	if (!groups->children()) return;

	fltk::Widget *w=groups->child(index);
	current = (TextureGroup*)w->user_data();
	InitGroupTexBrowser();
}

void TexGroupUI::InitGroupTexBrowser()
{
	groupTexBrowser->clear();
	if (!current) return;
	for (set<Texture*>::iterator i=current->textures.begin();i!=current->textures.end();++i)
		groupTexBrowser->AddTexture(*i);
	groupTexBrowser->UpdatePositions();
	groupTexBrowser->redraw();
}

void TexGroupUI::RemoveFromGroup()
{
	vector<Texture*> sel=groupTexBrowser->GetSelection();

	for (uint a=0;a<sel.size();a++) {
		groupTexBrowser->RemoveTexture(sel[a]);
		current->textures.erase (sel[a]);
	}
	groupTexBrowser->UpdatePositions();
}

void TexGroupUI::SetGroupName()
{
	if (!current) return;

	const char *str = fltk::input("Give name for new texture group:", 0);
	if (str) {
		current->name=str;
		UpdateGroupList();
	}
}

void TexGroupUI::RemoveGroup()
{
	if (current && groups->children()) {
		texGroupHandler->groups.erase(find (texGroupHandler->groups.begin(),texGroupHandler->groups.end(), current));
		current=0;
		UpdateGroupList ();
	}
}

void TexGroupUI::AddGroup()
{
	const char *str = fltk::input("Give name for new texture group:", 0);
	if (str)  {
		TextureGroup *tg = new TextureGroup;
		tg->name = str;

		texGroupHandler->groups.push_back(tg);
		UpdateGroupList();
	}
}


void TexGroupUI::AddToGroup()
{
	if (!current)
		return;

	vector <Texture *> sel = texBrowser->GetSelection();
	for (uint a=0;a<sel.size();a++) {
		if (current->textures.find(sel[a])==current->textures.end()) {
			current->textures.insert(sel[a]);
			groupTexBrowser->AddTexture (sel[a]);
		}
	}

	if (!sel.empty())
		groupTexBrowser->UpdatePositions();
}

void TexGroupUI::UpdateGroupList()
{
	groups->clear ();
	int curval=-1;

	if (!current && !texGroupHandler->groups.empty())
		current = texGroupHandler->groups.front();

	for (uint a=0;a<texGroupHandler->groups.size();a++) {
		TextureGroup *gr = texGroupHandler->groups[a];
	}

	if (curval>=0) groups->value(curval);
	InitGroupTexBrowser();
	groups->redraw();
}

const char *GroupConfigPattern="Group config file(*.gcf)\0gcf\0";

void TexGroupUI::SaveGroup()
{
	static std::string fn;
	if (!current)
		return;

	if (FileSaveDlg ("Save file:", GroupConfigPattern,fn)) {
		CfgList *cfg=texGroupHandler->MakeConfig (current);
		CfgWriter writer(fn.c_str());
		if (writer.IsFailed()) {
			fltk::message ("Failed to write %s\n", fn.c_str());
			return;
		}

		cfg->Write(writer,true);
	}
}

void TexGroupUI::LoadGroup()
{
	static std::string fn;
	if (!current)
		return;

	if (FileOpenDlg ("Load file:", GroupConfigPattern, fn)) {
		CfgList* cfg=CfgValue::LoadFile (fn.c_str());
		if (!cfg) {
			fltk::message ("Failed to read %s\n", fn.c_str());
			return;
		}
		TextureGroup *tg = texGroupHandler->LoadGroup (cfg);
		if (tg) {
			tg->name = fltk::filename_name(fn.c_str());
			UpdateGroupList ();
		}
	}
}

