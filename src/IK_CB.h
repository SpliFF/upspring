//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
protected:
	IEditor *callback;
	fltk::Widget *multipleTypes;

	void JointType(IKJointType jc);
public:
	IK_UI (IEditor *editor);
	~IK_UI ();

	void Show ();
	void Hide () { window->hide(); }
	void AnimateToPos ();
	void Update ();
