//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef VIEW_WINDOW_H
#define VIEW_WINDOW_H

#include <fltk/GlWindow.h>
#include "IView.h"

class CfgList;

struct Point { 
	Point () { x=y=0; }
	Point (int _x,int _y) { x=_x; y = _y; }
	int x,y; 
};


class ViewWindow : public fltk::GlWindow
{
public:
	ViewWindow (int X,int Y,int W,int H,const char *l=0);
	~ViewWindow ();

	void SetRasterMatrix ();

	// Fl_Widget interface
	void draw ();
	int handle (int msg);

	// Interface for subclasses
	virtual void DrawScene ()=0;
	virtual void Draw2D () {}
	virtual void GetViewMatrix (Matrix &m)=0;
	virtual void SetupProjectionMatrix () = 0;
	virtual float GetCamDistance (const Vector3& pos);

	void Serialize (CfgList& cfg, bool store);

	Vector3 backgroundColor;
	IEditor *editor;
protected:
	Vector3 FindSelCoords (int sx, int sy);
	void resize (int x,int y,int w, int h);
	void InitGL ();
	void Select (float sx, float sy, int w, int h, bool box);
	void DeSelect ();
	void PushSelector (ViewSelector *s);
	void PopSelector ();
	void Ortho ();	// an ortho compatible with GL_SELECT mode

	int selector_count;
	bool bSelecting, bUnSelecting;
	ViewSelector** selector_list;

	Point last;
	Point click;
	bool bBoxSelect;

	static inline void setvalue(fltk::Widget *w, bool v) {
		fltk::Item *i = (fltk::Item *)w;
		i->type(fltk::Item::TOGGLE);
		SET_WIDGET_VALUE(i, v);
	}
};

class EditorViewWindow;

struct Camera
{
	Camera ();
	void Reset ();
	void MouseEvent (EditorViewWindow *view, int msg, Point move);

	void MouseEventRotatingMode (EditorViewWindow *view, int msg, Point move);
	void MouseEventFPSMode (EditorViewWindow *view, int msg, Point move);
	void MouseEvent2DView (EditorViewWindow *view, int msg, Point move);

	void GetMatrix (Matrix& vm);
	Vector3 GetOrigin ();

	enum CtlMode {
		Rotating, FPS
	} ctlmode;

	void SetCtlMode (CtlMode m) { ctlmode=m; }

	// FPS state
	float yaw, pitch;
	Vector3 pos, strafe;
	Matrix mat;
	float zoom;
};


class EditorViewWindow : public ViewWindow, public IView
{
public:
	EditorViewWindow (int X,int Y,int W,int H,IEditor *cb);

	// IView interface
	void PushSelector (ViewSelector *s);
	void PopSelector ();
	int GetRenderMode () { return rendermode; }
	Vector3 GetCameraPos();
	TextureHandler* GetTextureHandler ();
	float GetConfig (int cfg);
	int GetMode ();
	bool IsSelecting () { return bSelecting || bUnSelecting; }

	int handle (int event);

	void ClickBorder (int x,int y);

	void ShowPopupMenu();
	void ShowViewModifyMenu(int x,int y);

	void SetupProjectionMatrix ();
	void DrawScene ();
	void Draw2D ();
	void DrawLight();
	void GetViewMatrix (Matrix &m);
	void DrawGrid ();

	void SetMode (int m);
	void Serialize (CfgList& config, bool store);

	int mode;
	int rendermode;
	Camera cam;
	bool bCullFaces;
	bool bLighting;
	bool bPolySelect;
	bool bHideGrid;
	bool bDrawRadius;
	bool bDrawCenters;
	bool bDrawMeshSmoothing;
	bool bDrawVertexNormals;
	bool bLockLightToCam;
	Vector3 lightPos;
};

class UVViewWindow : public ViewWindow
{
public:
	UVViewWindow (int X,int Y,int W,int H,const char *l=0);

	void SetupProjectionMatrix ();
	void DrawScene  ();
	void GetViewMatrix (Matrix &m);

	void ShowPopupMenu ();
	int handle (int);
	bool SetupChannelMask ();
	void DisableChannelMask ();

	int textureIndex;
	int channel;
	bool hasTexCombine;

};

#endif
