//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"

#include "Model.h"
#include "EditorUI.h"
#include "ObjectView.h"
#include "Util.h"


EditorUI::ObjectView::ObjectView(EditorUI *editor, fltk::Browser *tree) :
	editor(editor), tree(tree) 
{
	browserList.editor = editor;
	tree->list(&browserList);
}

EditorUI::ObjectView::~ObjectView()
{}


void EditorUI::ObjectView::Update()
{
	tree->redraw();
	tree->relayout();
}

// ------------------------------------------------------------------------------------------------
// List
// ------------------------------------------------------------------------------------------------

EditorUI::ObjectView::List::List() {
	rootObject = new MdlObject;
	editor = 0;
}
EditorUI::ObjectView::List::~List() {
	rootObject->childs.clear();
	delete rootObject;
}

MdlObject* EditorUI::ObjectView::List::root_obj() {
	rootObject->childs.resize(1);
	rootObject->childs[0] = editor->model->root;
	return editor->model->root ? rootObject : 0;
}

int EditorUI::ObjectView::List::children (const fltk::Menu *, const int *indexes, int level) {
	MdlObject *obj = root_obj();
	if (!obj) return -1;
	while (level--) {
		int i = *indexes++;
		//if (i < 0 || i >= group->children()) return -1;
		MdlObject *ch = obj->childs[i];
		if (ch->childs.empty()) return -1;
		obj = ch;
	}
	return obj->childs.size();
}


fltk::Widget *EditorUI::ObjectView::List::child (const fltk::Menu *, const int *indexes, int level)
{
	MdlObject *obj = root_obj();
	if (!obj) return 0;
	for (;;) {
		int i = *indexes++;
		if (i < 0 || i >= obj->childs.size()) return 0;
		MdlObject *ch = obj->childs[i];// Widget* widget = group->child(i);
		if (!level--) {
			obj = ch;
			break;
		}
		if (ch->childs.empty()) return 0;
		obj = ch;
	}

	// init widget
	item.label(obj->name.c_str());
	item.w(0);
	item.user_data(obj);

	// set selection
	item.set_flag (fltk::SELECTED, obj->isSelected);

	// set open/closed state
	item.set_flag (fltk::OPENED, obj->isOpen && obj->childs.size());

	return &item;
}

void EditorUI::ObjectView::List::flags_changed (const fltk::Menu *, fltk::Widget *w)
{
	MdlObject *obj = (MdlObject *)w->user_data();
	obj->isOpen = w->flags() & fltk::STATE;
	obj->isSelected = w->flags() & fltk::SELECTED;
	editor->SelectionUpdated();
}
