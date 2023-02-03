//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#ifndef VIEW_INTERFACE_H
#define VIEW_INTERFACE_H

#define MAP_XZ 0
#define MAP_XY 1
#define MAP_YZ 2
#define MAP_3D 3

// view modes
#define M3D_WIRE 0
#define M3D_SOLID 1
#define M3D_TEX 2

struct Model;
class ViewSelector;

class IView
{
public:
	virtual void PushSelector (ViewSelector *s)=0;
	virtual void PopSelector ()=0;
	virtual int GetMode () = 0;
	virtual int GetRenderMode () = 0;
	virtual Vector3 GetCameraPos () = 0;
	virtual float GetConfig (int cfg) = 0;
	virtual TextureHandler* GetTextureHandler () =0 ;
	virtual bool IsSelecting () = 0; // currently in a selection render?
};

#define CFG_OBJCENTERS 1 // render object centers?
#define CFG_POLYSELECT 2 // select polygons?
#define CFG_DRAWRADIUS 3 // draw Model::radius
#define CFG_DRAWHEIGHT 4 // draw Model::height
#define CFG_MESHSMOOTH 5
#define CFG_VRTNORMALS 6

class ViewSelector
{
public:
	ViewSelector () { link=0; }
	virtual float Score (Vector3 &pos, float camdis) = 0; // Is pos contained by this object?
	virtual void Toggle (Vector3 &pos, bool bSel) = 0;
	virtual bool IsSelected () = 0;

	void Link (ViewSelector* &list) { link = list; list = this; }
protected:
	ViewSelector *link;
};


void Circle(float x,float y,float rad, const Vector3& col, float acc=0.3f);

#endif
