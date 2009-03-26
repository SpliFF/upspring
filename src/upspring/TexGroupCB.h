//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

public:
    TexGroupUI (TextureGroupHandler *tgh, TextureHandler *th);
	~TexGroupUI ();

private:
	// UI callbacks
	void SelectGroup();
	void SetGroupName();
	void RemoveGroup();
	void RemoveFromGroup();
	void AddGroup();
	void AddToGroup();
	void UpdateGroupList();
	void InitGroupTexBrowser();
	void SaveGroup();
	void LoadGroup();
public:

	TextureGroup *current;
	TextureGroupHandler* texGroupHandler;
	TextureHandler *textureHandler;

