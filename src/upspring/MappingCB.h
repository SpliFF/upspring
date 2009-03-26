//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
protected:
	IEditor *callback;
public:
	MappingUI (IEditor *editor);
	~MappingUI ();

	void Show ();
	void Hide () { window->hide(); }
	void flipUVs();
	void mirrorUVs();
