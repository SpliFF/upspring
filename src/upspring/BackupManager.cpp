
#include "EditorIncl.h"
#include "EditorDef.h"
#include "BackupManager.h"
#include "Model.h"

static BackupManager* backupManager;

BackupManager& BackupManager::Get()
{
	return *backupManager;
}

BackupManager::BackupManager(IEditor *_editor)
{
	editor=_editor;
	backupManager=this;
	position = backups.end();
	numBackups = 40;
}

BackupManager::~BackupManager()
{}

BackupManager::Backup::Backup()
{ model = 0; }
BackupManager::Backup::~Backup()
{ delete model; }

BackupManager::Backup* BackupManager::LastBackup()
{
	if (!backups.empty()) 
		return &*position;

	return 0;
}

void BackupManager::AddBackup(const char *name, OperationType ot, ulong cmpDat)
{
	Model *srcmdl = editor->GetMdl();
	Model *copy = srcmdl->Clone();

	backups.push_back(Backup());
	backups.back().model = copy;
	backups.back().optype = ot;
	backups.back().compareData = cmpDat;
	backups.back().actionName = name;
	position = backups.end();
	position--;

	SetNumBackups(numBackups); // enforce backup limit
}

void BackupManager::AddMergeableOperation (const char *name, OperationType ot, BackupManager::ApplyOperationCB cb, void *userdata)
{
	Backup *lbk = LastBackup();

	ulong hash = editor->GetMdl()->ObjectSelectionHash ();
	if (ot != OT_Atomic && lbk && lbk->optype == ot && lbk->compareData == hash)
	{
		// mergeable, so apply the operation to the editors version and to the end of the backuplist
		RemoveRedoBackups();
		cb (lbk->model, userdata);
		cb (editor->GetMdl(), userdata);
	}
	else
	{
		// non-mergeable apply to editor version and clone
		cb (editor->GetMdl(), userdata);
		RemoveRedoBackups();
		AddBackup(name, ot, hash);
	}
}

void BackupManager::AddBackupPoint(const char *name)
{
	if (numBackups <= 1)
		return;

	RemoveRedoBackups();
	AddBackup(name, OT_Atomic, 0);
}

void BackupManager::RemoveRedoBackups()
{
	if (!backups.empty()) 
	{
		BackupList::iterator next = position;
		next++;

		if (next != backups.end()) {
			// "redo" operations will be dropped now
			backups.erase (next, backups.end());
		}
	}
}

void BackupManager::Undo()
{
	if (HasUndo()) {
		position--;
		Model *mdl = position->model->Clone();

		editor->SetModel (mdl);
	}
}

void BackupManager::ReloadLast()
{
	if (position != backups.end())
		editor->SetModel (position->model->Clone());
}

void BackupManager::Redo()
{
	if (HasRedo()) {
		position ++;
		editor->SetModel (position->model->Clone());
	}
}

void BackupManager::Goto(int n)
{
	if (backups.empty())
		return;

	// find the right position
	BackupList::iterator i = backups.begin();
	int index = 0;
	for (; i != backups.end(); i++, index++)
		if (index == n) break;

	if (i != backups.end()) {
		position = i;
		editor->SetModel(position->model->Clone());
	}
}

bool BackupManager::HasRedo()
{
	BackupList::iterator next = position;
	next++;
	return !backups.empty() && next != backups.end();
}

bool BackupManager::HasUndo()
{
	return !backups.empty() && position != backups.begin();
}


void BackupManager::SetNumBackups(int n)
{
	numBackups = n;

	size_t cs = backups.size();
	while (cs-- > numBackups) {
		if (position == backups.begin())
			position ++;
		backups.pop_front();
	}
	if (backups.empty())
		position = backups.end();
}
