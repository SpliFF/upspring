#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <list>

enum OperationType
{
	OT_Atomic, // non-mergable
	OT_Move,
	OT_Scale,
	OT_Rotate,
	OT_OriginMove
};

class IEditor;
struct Model;

// singleton
class BackupManager
{
public:
	BackupManager(IEditor *editor);
	~BackupManager();

	static BackupManager& Get();

	typedef void (*ApplyOperationCB)(Model* mdl, void *param);

	// Will apply the operation on both the mergeable backup and the active model in the editor
	void AddMergeableOperation(const char *name, OperationType op, ApplyOperationCB cb, void *userdata);
	void AddBackupPoint(const char *name);

	void Undo();
	void Redo();
	void Goto(int i); // move to backup #i
	void ReloadLast();

	bool HasUndo();
	bool HasRedo();

	unsigned int GetNumBackups() { return numBackups; }
	void SetNumBackups(int num);


	struct Backup
	{
		Backup(); 
		~Backup();

		Model *model;
		OperationType optype;
		ulong compareData;
		std::string actionName;
	};

	typedef std::list<Backup> BackupList;
	BackupList::iterator BeginIterator() { return backups.begin(); }
	BackupList::iterator EndIterator() { return backups.end(); }
	BackupList::iterator GetPosition() { return position; }

protected:
	void AddBackup(const char *name, OperationType ot, ulong cmpDat);
	void RemoveRedoBackups();

	unsigned int numBackups;
	IEditor *editor;

	Backup* LastBackup();

	BackupList::iterator position;
	BackupList backups;
};


#define BACKUP_MERGEABLE_OP(name, op, func, udata) (BackupManager::Get().AddMergeableOperation(name, op, func, udata))
#define BACKUP_POINT(name) (BackupManager::Get().AddBackupPoint(name))

#endif
