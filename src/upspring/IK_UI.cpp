//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "CfgParser.h"
#include "AnimationUI.h"

#include <GL/glew.h>
#include <GL/gl.h>


// ------------------------------------------------------------------------------------------------
// IK_UI
// ------------------------------------------------------------------------------------------------

IK_UI::IK_UI(IEditor *callback) : callback(callback)
{
	multipleTypes = 0;

	CreateUI ();
}

IK_UI::~IK_UI()
{
	delete window;
}


void IK_UI::Show ()
{
	window->set_non_modal();
	window->show ();
}

void IK_UI::AnimateToPos ()
{
	
}

void IK_UI::JointType (IKJointType jt)
{
	Model *mdl = callback->GetMdl ();
	vector<MdlObject*> sel = mdl->GetSelectedObjects ();

	for (uint a=0;a<sel.size();a++) {
		IKinfo& ik = sel[a]->ikInfo;

		if (ik.jointType == jt)
			continue;

		SAFE_DELETE(ik.joint);
		if (jt == IKJT_Hinge) 
			ik.joint = new HingeJoint;
		else if (jt == IKJT_Universal)
			ik.joint = new UniversalJoint;

		ik.jointType = jt;
	}
}


void IK_UI::Update ()
{
	if (!window->visible())
		return;

	Model *mdl = callback->GetMdl ();
	vector<MdlObject*> sel = mdl->GetSelectedObjects ();

	const char *mt = "<multiple types>";
	selJointType->remove(mt);

	if (!sel.empty()) {
		selJointType->activate ();

		int jt = sel.front()->ikInfo.jointType;
		for (uint a=1;a<sel.size();a++) {
			if (sel[a]->ikInfo.jointType != jt) {
				jt = -1;
				break;
			}
		}

		if (jt < 0) {
			multipleTypes = selJointType->add (mt);
			int index = ((fltk::Group*)selJointType)->find (multipleTypes);
			selJointType->value (index);
		} else
			selJointType->value (jt);
	} else
		selJointType->deactivate ();
	window->redraw ();
}

