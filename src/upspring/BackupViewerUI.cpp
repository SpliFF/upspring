#include "EditorIncl.h"
#include "EditorDef.h"
#include "BackupManager.h"
#include "EditorUI.h"

#include <sstream>

BackupViewerUI::BackupViewerUI(IEditor *cb)
{
	callback = cb;
	CreateUI();
}


BackupViewerUI::~BackupViewerUI()
{}

void BackupViewerUI::Show()
{
	window->set_non_modal();
	window->show();
}

void BackupViewerUI::Hide()
{
	window->hide();
}

void BackupViewerUI::BufferViewerCallback(fltk::Widget *w, void *d)
{
	long index = (long)d;

	BackupManager& bm = BackupManager::Get();
	bm.Goto (index);
}

void BackupViewerUI::Update()
{
	bufferView->clear();

	BackupManager& bm = BackupManager::Get();
	BackupManager::BackupList::iterator i = bm.BeginIterator();
	BackupManager::BackupList::iterator pos = bm.GetPosition();
	long index=0, selindex=0;
	for (; i != bm.EndIterator(); ++i, index++) {
		std::string name = i->actionName;
		if (i == pos) {
			name = "<" + name + ">";
			selindex=index;
		}
		fltk::Widget *w = bufferView->add(name.c_str(), 0, BufferViewerCallback, (void*)index);
	}
	bufferView->select( (int)selindex);
	bufferView->redraw();
}

void BackupViewerUI::SetBufferSize()
{
	char buf[20];
	SNPRINTF(buf, 20, "%d", BackupManager::Get().GetNumBackups());

	const char *r = fltk::input ("Enter buffer size: ", buf);
	if (r) {
		BackupManager::Get().SetNumBackups(max (atoi (r),0) );
		callback->Update();
	}
}


