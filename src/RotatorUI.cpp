//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "EditorUI.h"
#include "CfgParser.h"

RotatorUI::~RotatorUI()
{
	delete window;
}

void RotatorUI::ApplyRotation(bool absolute, int axis, fltk::Input* o)
{
	std::vector<MdlObject*> selection = editorCallback->GetMdl()->GetSelectedObjects();

	if(selection.size()!=1)
		return;

	MdlObject *obj = selection.front();

	Vector3 rot;
	rot.v[axis] = atof(o->value()) * DegreesToRadians;

	if (absolute)
		obj->rotation.AddEulerAbsolute(rot);
	else
		obj->rotation.AddEulerRelative(rot);

	o->value("0");
	editorCallback->Update();
}

void RotatorUI::Update()
{
	std::vector<MdlObject*> selection = editorCallback->GetMdl()->GetSelectedObjects();

	fltk::Input* inputs[] = { inputAbsX, inputAbsY, inputAbsZ, inputRelX, inputRelY, inputRelZ, 0 };
	if(selection.size()!=1) {
		// disable all inputs
		for (int a=0;inputs[a];a++)
			inputs[a]->deactivate();
	}
	else {
		for (int a=0;inputs[a];a++) {
			inputs[a]->value("0");
			inputs[a]->activate();
		}
	}
}

void RotatorUI::Show()
{
	Update();

	window->set_non_modal();
	window->show();
}

void RotatorUI::Hide()
{
	window->hide();
}

void RotatorUI::ResetRotation()
{
	std::vector<MdlObject*> selection = editorCallback->GetMdl()->GetSelectedObjects();

	if(selection.size()!=1)
		return;

	MdlObject *obj = selection.front();
	obj->rotation = Rotator();
	editorCallback->Update();
}


void RotatorUI::ApplyRotationToGeom()
{
	std::vector<MdlObject*> selection = editorCallback->GetMdl()->GetSelectedObjects();

	if(selection.size()!=1) return;
	MdlObject *obj = selection.front();
	obj->ApplyTransform(true,false,false);
	editorCallback->Update();
}
