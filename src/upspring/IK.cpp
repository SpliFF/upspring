//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"

#include "Util.h"
#include "Model.h"


IKinfo::IKinfo()
{
	jointType = IKJT_Fixed;
	joint = 0;
}

IKinfo::~IKinfo()
{
	SAFE_DELETE(joint);
}

