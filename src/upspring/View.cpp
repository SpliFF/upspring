//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include <fltk/gl.h>
#include <GL/glu.h>

#include "View.h"
#include "Util.h"
#include "Model.h"
#include "Tools.h"
#include "CfgParser.h"

const int PopupBoxW = 32;
const int PopupBoxH = 18;


// ------------------------------------------------------------------------------------------------
// ViewWindow - base OpenGL viewing widget
// ------------------------------------------------------------------------------------------------

// size of selection box in window coordinates
#define SELECT_W 3

ViewWindow::ViewWindow(int x,int y, int w,int h,const char *l) : fltk::GlWindow (x,y,w,h,l)
{
	selector_count=0;
	bSelecting=false;
	bUnSelecting=false;
	bBoxSelect=false;
}


ViewWindow::~ViewWindow()
{}



void ViewWindow::InitGL ()
{
	glViewport(0,0,w(), h());
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_POINT_SMOOTH);
	glFrontFace (GL_CW);

	valid(1);
}

void ViewWindow::SetRasterMatrix ()
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glScalef(2.0f / (float)w(), -2.0f / (float)h(), 1.0f);
	glTranslatef (-((float)w() / 2.0f), -((float)h() / 2.0f), 0.0f);
	glMatrixMode (GL_MODELVIEW);
}

void ViewWindow::draw ()
{
	bool use_solid = false;

	if(!valid ())
		InitGL ();

	glClearColor(backgroundColor.x,backgroundColor.y,backgroundColor.z,0.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	SetupProjectionMatrix ();

	bSelecting=false;
	selector_count=0;

	DrawScene ();
	SetRasterMatrix ();
	Draw2D ();

	if (bBoxSelect)
	{
		fltk::glsetcolor(fltk::WHITE);
		fltk::glstrokerect(click.x,click.y,last.x-click.x, last.y-click.y);
	}
}

float ViewWindow::GetCamDistance (const Vector3& pos) {
	return 1.0f;
}

/*
Find the world position of the user-clicked selection area on (x,y),
using gluUnProject
*/
Vector3 ViewWindow::FindSelCoords (int px, int py)
{
	float depthbuf[SELECT_W*SELECT_W];
	int x,y;
	
	glReadPixels (px-SELECT_W/2, py-SELECT_W/2, SELECT_W, SELECT_W, GL_DEPTH_COMPONENT, GL_FLOAT, depthbuf);
	float *d = depthbuf;
	int best_x = -1, best_y = -1;
	float best_z = 1.0f;

	for(y = 0; y < SELECT_W; y++)
		for(x = 0; x < SELECT_W; x++)
		{
			if(*d < best_z)
			{
				best_z = *d;
				best_x = x + px-SELECT_W/2;
				best_y = y + py-SELECT_W/2;
			}
			++d;
		}

	if(best_x < 0) // something not right, so just ignore
		return Vector3();

	GLdouble modelview[16];
	GLdouble projection[16];
	glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev (GL_PROJECTION_MATRIX, projection);
	
	GLint vp [4];
	glGetIntegerv (GL_VIEWPORT, vp);

	double rx,ry,rz;
	if(gluUnProject (best_x, best_y, best_z, modelview, projection, vp, &rx, &ry, &rz) == GL_TRUE)
	{
		//logger.Trace (NL_Msg, "rx: %f, ry: %f, rz: %f\n", (float)rx, (float)ry, (float)rz);
		return Vector3(rx,ry,rz);
	}

	return Vector3();
}

void ViewWindow::Ortho ()
{
	glScalef (.1f, .1f, .1f);
}


void ViewWindow::PushSelector (ViewSelector *s)
{
	if(bSelecting)
	{
		glPushName (selector_count + 1);
		selector_list [selector_count] = s;
		selector_count ++;
	}
	else if (bUnSelecting) {
		Vector3 v;
		s->Toggle(v, false);
		// KLOOTNOTE: reference to temporary object
		// s->Toggle(Vector3(), false);
	}
}

void ViewWindow::PopSelector ()
{
	if(bSelecting) {
		glPopName ();
	}
}


void ViewWindow::Select(float sx, float sy, int w, int h, bool box) {
	int bufsize = 10000;	// FIXME: Find some way to calculate this value
	uint *buffer;
	int vp[4];				// viewport

	make_current();

	buffer = ALLOCA_ARRAY(uint, bufsize);
	selector_list = ALLOCA_ARRAY(ViewSelector*, bufsize);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	sy = this->h() - sy;
	glGetIntegerv(GL_VIEWPORT, vp);
	gluPickMatrix(sx + w / 2, sy - h / 2, w, h, vp);
	SetupProjectionMatrix();

	glSelectBuffer(bufsize, (uint *)buffer);
	glRenderMode(GL_SELECT);
	glInitNames();

	bSelecting = true;
	selector_count = 1;
	glPushName(0);
	DrawScene();
	glPopName();
	bSelecting = false;

	int a, hits;
	if ((hits = glRenderMode(GL_RENDER)) != 0) {
		Vector3 pos;
		if (!box) {
			pos = FindSelCoords(sx + w / 2, sy + h / 2);
			logger.Trace(NL_Msg,  "SelPos: %d, %d, %d\n", (int) pos.x, (int) pos.y, (int) pos.z);
		}

		uint *pBuf = buffer;
		ViewSelector *best = 0;
		float bestscore;
		bool bRedraw = false;
		for (a = 0; a < hits; a++) {
			uint num = pBuf [0];
			pBuf += 3;
			
			for (uint b=0;b<num;b++)
			{
				int sl_index = *(pBuf ++) - 1;

				// only the top of the name stack
				if (b < num - 1)
					continue;

				if (sl_index >= 0 && sl_index < bufsize)
				{
					ViewSelector *sel = selector_list [sl_index];
					assert (sel);

					if (box)
					{
						sel->Toggle (pos, !sel->IsSelected ());
						bRedraw=true;
					}
					else
					{
						float score = sel->Score(pos, GetCamDistance (pos));
						if (!best || score < bestscore) {
							best=sel;
							bestscore=score;
						}
					}
				}
			}
		}
		if (best)
		{
			bool bSelect = !best->IsSelected ();
			best->Toggle (pos, bSelect);
			bRedraw=true;
		}

		if (bRedraw)
			editor->SelectionUpdated ();
	}
}


void ViewWindow::DeSelect ()
{
	bUnSelecting=true;
	draw ();
	bUnSelecting=false;

	editor->SelectionUpdated ();
}

/*
  FLTK event handler
 */
int ViewWindow::handle (int msg)
{
	int ret =  GlWindow::handle (msg);
	int x = fltk::event_x(),y = fltk::event_y(), b = fltk::event_button();

	if(msg == fltk::PUSH)
	{
		click.x = fltk::event_x ();
		click.y = fltk::event_y ();
		last = click;
		if (fltk::event_state() & fltk::SHIFT)
			bBoxSelect=true;
		return -1;
	}

	if(msg == fltk::MOVE)
	{
		last.x = fltk::event_x ();
		last.y = fltk::event_y ();
		return -1;
	}

	if (msg == fltk::DRAG)
	{
		if (bBoxSelect) 
			redraw ();

		last.x = fltk::event_x ();
		last.y = fltk::event_y ();
	}

	if(msg == fltk::RELEASE)
	{
		if(bBoxSelect)
		{
			int sx=fltk::event_x (), sy = fltk::event_y ();
			int ex=click.x, ey=click.y;

			if (sx>ex) std::swap (sx,ex);
			if (sy>ey) std::swap (sy,ey);

			Select (sx,sy, ex-sx,ey-sy, true);
			bBoxSelect=false;
			redraw ();
		}
		else
		{
			if(click.x == x && click.y == y)
			{
				if(b == 1)
					Select(click.x-SELECT_W/2, click.y-SELECT_W/2, SELECT_W, SELECT_W, false);
				else if(b == 3)
					DeSelect ();
			}
		}
		return -1;
	}

	if(msg == fltk::ENTER)
		return 1;

	if(msg == fltk::LEAVE)
	{
		if(x < this->x() || x > this->x()+w() || y<this->y() || y >this->y()+h())
			bBoxSelect=false;
	}

	return ret;
}


void ViewWindow::resize (int x,int y,int w,int h)
{
	GlWindow::resize (x,y,w,h);
}

void ViewWindow::Serialize (CfgList& cfg, bool store)
{
	int X=x(),Y=y(),Width=w(),Height=h();
	float& bgRed=backgroundColor.x;
	float& bgGreen=backgroundColor.y;
	float& bgBlue=backgroundColor.z;
	if (store) {
		// view dimensions
		CFG_STOREN(cfg,X); CFG_STOREN(cfg,Y);
		CFG_STOREN(cfg,Width); CFG_STOREN(cfg,Height);
		// store background color
		CFG_STOREN(cfg,bgRed); CFG_STOREN(cfg,bgGreen); CFG_STOREN(cfg,bgBlue);
	} else {
		CFG_LOADN(cfg,X); CFG_LOADN(cfg,Y);
		CFG_LOADN(cfg,Width); CFG_LOADN(cfg,Height);
		CFG_LOADN(cfg,bgRed); CFG_LOADN(cfg,bgGreen); CFG_LOADN(cfg,bgBlue);
		resize(X,Y,Width,Height);
	}
}


// ------------------------------------------------------------------------------------------------
// Camera for EditorViewWindow
// ------------------------------------------------------------------------------------------------

const float SpeedMod = 0.05f;
const float AngleMod = 1.0f;

Camera::Camera()
{
	Reset();
	ctlmode = Rotating;
}

void Camera::Reset ()
{
	yaw = pitch = 0.0f;
	zoom = 1.0f;
	pos = Vector3();
}

void Camera::MouseEvent(EditorViewWindow *view, int msg, Point move)
{
	if (view->GetMode () == MAP_3D) {
		switch (ctlmode) {
			case Rotating: MouseEventRotatingMode (view,msg,move); break;
			case FPS: MouseEventFPSMode (view,msg,move); break;
		}
	} else
		MouseEvent2DView (view,msg,move);
}

void Camera::MouseEvent2DView (EditorViewWindow *view, int msg, Point move)
{
	uint s = fltk::event_state();
	if((msg == fltk::DRAG || msg == fltk::MOVE) && ((s & fltk::BUTTON1) || (s & fltk::BUTTON2)))
	{
		pos += Vector3 (-move.x / zoom, move.y / zoom , 0) * 0.05f;
		view->redraw();
	}
	else if((msg == fltk::DRAG || msg == fltk::MOVE) && (s & fltk::BUTTON3))
	{
		zoom *= (1-move.x * 0.02-move.y*0.02);
		view->redraw();
	}
}

void Camera::MouseEventFPSMode (EditorViewWindow *view, int msg, Point move)
{
	uint s = fltk::event_state();
    if((msg == fltk::DRAG || msg == fltk::MOVE) && (s & fltk::BUTTON1))
	{
		Vector3 f;
		yaw += (M_PI * move.x * AngleMod / 360);
		Math::ComputeOrientation (yaw, 0.0f, 0.0f,0,0, &f);
		pos -= f * move.y * SpeedMod * 2;
		view->redraw();
	}
	else if((msg == fltk::DRAG || msg == fltk::MOVE) && (s & fltk::BUTTON3))
	{
		yaw += (M_PI * (move.x * AngleMod) / 180); 
		pitch -= (M_PI * (move.y * AngleMod) / 360);
		view->redraw();
	}
	else if((msg == fltk::DRAG || msg == fltk::MOVE) && ((s & fltk::BUTTON2) || ((s & fltk::BUTTON1) && (s & fltk::BUTTON3))))
	{
		Vector3 right;
		Math::ComputeOrientation (yaw, 0.0f, 0.0f,&right,0,0);
		pos.y -= move.y * 0.5f* SpeedMod;
		pos += right * (move.x * 0.5f * SpeedMod);
		view->redraw();
	}
}

void Camera::MouseEventRotatingMode (EditorViewWindow *view, int msg, Point move)
{
	uint s = fltk::event_state(); // event_button() only works for PUSH/RELEASE
	if (msg==fltk::DRAG && (s&fltk::BUTTON1))
	{
		yaw += M_PI * move.x * AngleMod / 360; // AngleMod deg per x
		pitch -= M_PI * move.y * AngleMod / 360;
		view->redraw();
	}
	else if(msg==fltk::DRAG && (s&fltk::BUTTON3))
	{
		zoom *= 1-move.x * 0.02-move.y*0.02;
		view->redraw();
	}
	else if(msg==fltk::DRAG && (s&fltk::BUTTON2))
	{
		// x & y are used to displace the camera sideways
		Vector3 r,u;
		Math::ComputeOrientation (yaw, pitch, 0, &r, &u, 0);

		float Xmove = -move.x * 0.5f * SpeedMod * zoom;
		float Ymove = move.y * 0.5f * SpeedMod * zoom;
		strafe += r * Xmove + u * Ymove;
		view->redraw();
	}
}

void Camera::GetMatrix(Matrix& vm)
{
	if (ctlmode == FPS)  {
		Vector3 r,u,f;
		Math::ComputeOrientation (yaw, pitch, 0, &r, &u, &f);
		vm.camera (&pos, &r, &u, &f);
	}
	else if(ctlmode == Rotating) {
        Matrix tmp;
		vm.translation(-strafe);//initial translation of the model, pos is the center of rotation
		tmp.yrotate(yaw);
		vm *= tmp;
		tmp.xrotate(pitch);
		vm *= tmp;
		vm.addtranslation(Vector3(0.0f, 0.0f, 10*zoom));
	}
}

Vector3 Camera::GetOrigin ()
{
	if (ctlmode == FPS) 
		return pos;
	else {
		Matrix inv, mat;
		GetMatrix (mat);
		if (mat.inverse (inv))
		{
			Vector3 zero, origin;
			inv.apply (&zero, &origin); 
			return origin * 0.15f;
		}
		return Vector3();
	}
}

// ------------------------------------------------------------------------------------------------
// EditorViewWindow - Implements model views in main editor window
// ------------------------------------------------------------------------------------------------

EditorViewWindow::EditorViewWindow (int X,int Y,int W,int H,IEditor *cb) 
	: ViewWindow (X,Y,W,H)
{
	mode = MAP_XY;
	rendermode = M3D_WIRE;
	bCullFaces=false;
	bLighting=false;
	bHideGrid=false;
	bDrawMeshSmoothing=false;
	bDrawVertexNormals=false;
	bLockLightToCam=true;
	bDrawRadius=false;
	bDrawCenters=true;
	editor = cb;
}

void EditorViewWindow::DrawLight()
{
	glPointSize (10);
	glColor3ub (255,255,0);
	glBegin(GL_POINTS);
	glVertex3f(lightPos.x, lightPos.y, lightPos.z);
	glEnd();
	glPointSize (1);
	glColor3ub (255,255,255);
}


void EditorViewWindow::DrawScene ()
{
	glEnable (GL_DEPTH_TEST);

	// setup the view matrix
	Matrix tr;
	GetViewMatrix (cam.mat);
	cam.mat.transpose (&tr);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf ((const float*)&tr);

	if (!bHideGrid)
		DrawGrid ();
	
	if (bLockLightToCam)
		lightPos = cam.GetOrigin();

	if(bLighting && !bLockLightToCam)
		DrawLight();

	// Clear the modelview matrix so the light is not transformed
//	glLoadIdentity ();

	if (bCullFaces) {
		if (mode==MAP_3D)
			glFrontFace (GL_CW);
		else
			glFrontFace (GL_CCW);
		glEnable (GL_CULL_FACE);
	}

	glEnable (GL_DEPTH_TEST);

	if(rendermode==M3D_WIRE)
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	if (bLighting) {
		glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		float pos[4]={lightPos.x, lightPos.y, lightPos.z,1.0f};
		glLightfv (GL_LIGHT0, GL_POSITION, pos);
		float diffuse[]={1,1,1,1};
		glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuse);
		float specular[]={0.5f,0.5f,0.5f,0.0f};
		glLightfv (GL_LIGHT0, GL_SPECULAR, specular);
		//glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
		//const float params[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		//glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, params);
		//glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		//glEnable (GL_COLOR_MATERIAL);
	} else {
		glDisable (GL_LIGHTING);
		glDisable (GL_COLOR_MATERIAL);
	}

	// Load the modelview transform again
	glLoadMatrixf ((const float*)&tr);

	glColor3ub (255,255,255);
	editor->RenderScene (this);

	glDisable (GL_LIGHT0);
	glDisable (GL_LIGHTING);
	glDisable (GL_COLOR_MATERIAL);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
}

void EditorViewWindow::Draw2D ()
{
	char * str;
	switch (mode) {
	case MAP_3D: str = "3D"; break;
	case MAP_XY: str = "XY"; break;
	case MAP_XZ: str = "XZ"; break;
	case MAP_YZ: str = "YZ"; break;
	}

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glDisable (GL_TEXTURE_2D);
	glColor3ub(255,255,255);
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	glRecti (1,1,w()-2,h()-2);
	fltk::glsetcolor(fltk::GRAY50);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glRecti (1,1,PopupBoxW,PopupBoxH);
	fltk::glsetfont(fltk::COURIER, 16);
	fltk::glsetcolor(fltk::BLUE);
	fltk::gldrawtext(str, 5,15);
}

static Vector3 MakeVisibleColor (Vector3 bg)
{
	Vector3 r;
	for (int i=0;i<3;i++) {
		if (bg.v[i] < 0.5f) r.v[i]=bg.v[i] + 0.5f;
		else r.v[i] = bg.v[i] - 0.5f;
	}
	return r;
}

void EditorViewWindow::DrawGrid()
{
	Vector3 c = MakeVisibleColor (backgroundColor);
	glColor3fv(&c.x);

	int sz=16;
	if (mode==MAP_3D) {
		int w=sz,h=sz;
		float s=8.0f;
		glScalef (s,s,s);
		glEnable (GL_DEPTH_TEST);
		// draw a small grid the 3D view
		glBegin (GL_LINES);
		for (int x=-w/2;x<=w/2;x++) {
			glVertex3i (x,0,-h/2);
			glVertex3i (x,0,h/2);
		}
		for (int y=-h/2;y<=h/2;y++) {
			glVertex3i (-w/2,0,y);
			glVertex3i (h/2,0,y);
		}
		glEnd ();
	}
	else {
		int w=sz,h=sz;//this->w()/80/cam.zoom,h=this->h()/80/cam.zoom;
		float s=8.0f;
		glScalef (s,s,s);
		// draw a small grid the 3D view
		glBegin (GL_LINES);
		int stx=-w/2,endx=w/2;
		int sty=-h/2,endy=h/2;

		for (int x=stx;x<=endx;x++) 
			switch (mode)
			{
				case MAP_XY: glVertex2i (x,sty); glVertex2i (x,endy); break;
				case MAP_XZ: glVertex3i (x,0,sty); glVertex3i (x,0,endy); break;
				case MAP_YZ: glVertex3i (0,sty,x); glVertex3i (0,endy,x); break;
			}
		for (int y=sty;y<=endy;y++) {
			switch (mode) 
			{
				case MAP_XY: glVertex2i (stx,y); glVertex2i (endx,y); break;
				case MAP_XZ: glVertex3i (stx,0,y); glVertex3i (endx,0,y); break;
				case MAP_YZ: glVertex3i (0,y,stx); glVertex3i (0,y,endx); break;
			}
		}
		glEnd ();
	}
}

void EditorViewWindow::GetViewMatrix (Matrix &tv)
{
	tv.clear();
	switch (mode)
	{
	case MAP_3D:
		cam.GetMatrix (tv);
		break;
	case MAP_XY:
		tv.identity ();
		tv.t (0) = -cam.pos.x;
		tv.t (1) = -cam.pos.y;
		break;
	case MAP_XZ:
		// x' = x, y' = z
		tv.v(0, 0) = 1.0f;
		tv.v(1, 2) = 1.0f;
		tv.v(2, 1) = 1.0f;
		tv.v(3, 3) = 1.0f;
		tv.t (0) = -cam.pos.x;
		tv.t (1) = -cam.pos.y;
		break;
	case MAP_YZ:
		// x' = z, y' = y
		tv.v(0, 2) = 1.0f;
		tv.v(1, 1) = 1.0f;
		tv.v(2, 0) = 1.0f;
		tv.v(3, 3) = 1.0f;
		tv.t (0) = -cam.pos.x;
		tv.t (1) = -cam.pos.y;
		break;
	}

	if(mode != MAP_3D) {
		// apply zoom
		Matrix zoomscale;
		zoomscale.scale (cam.zoom, cam.zoom, 1.0f);
		tv *= zoomscale;

		tv.v (2, 0) = tv.v(2,1) = tv.v(2,2) = 0.0f;
		tv.v (2, 3) = 1.0f ;// z-component is always 1.0f
	}

	if (h () != 0) {
		tv.gety()->mul(w() / (float)h());
	}
}

Vector3 EditorViewWindow::GetCameraPos ()
{
	if (mode == MAP_3D) {
		return cam.GetOrigin ();
	}
	return Vector3(0,0,1);
}

void EditorViewWindow::SetupProjectionMatrix()
{
	if(mode == MAP_3D) 
	{
		gluPerspective (70.0f, 1, 1.0f, 4000.0f);
		glScalef(1.0f,1.0f,-1.0f);
	}
	else Ortho ();
}

void EditorViewWindow::SetMode (int m)
{
	mode = m;

	cam.Reset();
	if (m == MAP_3D && cam.ctlmode==Camera::FPS)
		cam.pos.set(0,0,-10);
	if (m == MAP_3D && cam.ctlmode==Camera::Rotating)
	{
		cam.pitch=-M_PI/6;
		cam.zoom=20;
	}
	if (m != MAP_3D)
		cam.zoom = 0.1f;
	SetupProjectionMatrix ();
	redraw();
}


int EditorViewWindow::GetMode ()
{
	return mode;
}

template<int rmode> static void RenderTypeCB(fltk::Widget* w, void *data)
{
	EditorViewWindow *wnd = (EditorViewWindow *)data;
	wnd->rendermode=rmode;
	wnd->redraw();
}

template<int MapType> static void MapTypeCB(fltk::Widget* w, void *data)
{
	EditorViewWindow *wnd = (EditorViewWindow *)data;
	wnd->SetMode(MapType);
}

template<Camera::CtlMode ctlmode> static void CameraModeCB(fltk::Widget *w,void *data)
{
	EditorViewWindow *wnd= (EditorViewWindow *)data;
	wnd->cam.SetCtlMode (ctlmode);
	wnd->SetMode(MAP_3D);
	wnd->redraw();
}

template<bool EditorViewWindow::*prop> static void BooleanPropCB(fltk::Widget *w,void *data) {
	EditorViewWindow *wnd = (EditorViewWindow *)data;
	wnd->*prop = !(wnd->*prop);
	wnd->redraw();
}

inline void setvalue(fltk::Widget *w, bool v) {
	fltk::Item *i = (fltk::Item *)w;
	i->type(fltk::Item::TOGGLE);
	i->set_flag(fltk::STATE, v);
}

template<bool bHor> static void SplitViewCB(fltk::Widget *w,void *data)
{
	EditorViewWindow *v=(EditorViewWindow *)data;

	EditorViewWindow *nv;
	if (bHor) {
		nv = new EditorViewWindow (v->x(), v->y(), v->w()/2,v->h(), v->editor);
		v->set_x(v->x() + v->w()/2);
	} else {
		nv = new EditorViewWindow (v->x(), v->y(), v->w(), v->h()/2, v->editor);
		v->set_y(v->y() + v->h()/2);
	}

	nv->cam = v->cam;
	nv->mode = v->mode;
	nv->rendermode = v->rendermode;
	nv->bCullFaces = v->bCullFaces;
	nv->bLighting = v->bLighting;
	nv->backgroundColor = v->backgroundColor;
	nv->bHideGrid = v->bHideGrid;
	nv->bDrawCenters = v->bDrawCenters;
	nv->bDrawRadius = v->bDrawRadius;
	nv->bLockLightToCam = v->bLockLightToCam;

	v->editor->AddView (nv);
}

void EditorViewWindow::ShowPopupMenu()
{
	fltk::Menu *p = new fltk::Menu (0,0,PopupBoxW,PopupBoxH);

	setvalue(p->add("Camera/3D FPS",0,&CameraModeCB<Camera::FPS>, this),mode==MAP_3D && cam.ctlmode==Camera::FPS);
	setvalue(p->add("Camera/Rotating",0,&CameraModeCB<Camera::Rotating>, this),mode==MAP_3D && cam.ctlmode==Camera::Rotating);
	setvalue(p->add("Camera/XY",0,&MapTypeCB<MAP_XY>, this),mode==MAP_XY);
	setvalue(p->add("Camera/XZ",0,&MapTypeCB<MAP_XZ>, this),mode==MAP_XZ);
	setvalue(p->add("Camera/YZ",0,&MapTypeCB<MAP_YZ>, this),mode==MAP_YZ);

	setvalue(p->add("Render mode/Wireframe", 0,&RenderTypeCB<M3D_WIRE>, this),rendermode==M3D_WIRE);
	setvalue(p->add("Render mode/Solid", 0,&RenderTypeCB<M3D_SOLID>, this),rendermode==M3D_SOLID);
	setvalue(p->add("Render mode/Texture", 0,&RenderTypeCB<M3D_TEX>, this),rendermode==M3D_TEX);

	setvalue(p->add("Cull faces", 0, &BooleanPropCB<&EditorViewWindow::bCullFaces>, this), bCullFaces);
	setvalue(p->add("Lighting", 0, &BooleanPropCB<&EditorViewWindow::bLighting>, this), bLighting);
	setvalue(p->add("Lock light to camera", 0, &BooleanPropCB<&EditorViewWindow::bLockLightToCam>,this), bLockLightToCam);
	setvalue(p->add("Hide grid", 0, &BooleanPropCB<&EditorViewWindow::bHideGrid>, this), bHideGrid);
	setvalue(p->add("Draw model radius+height", 0, &BooleanPropCB<&EditorViewWindow::bDrawRadius>,this), bDrawRadius);
	setvalue(p->add("Render vertex normals", 0, &BooleanPropCB<&EditorViewWindow::bDrawVertexNormals>,this), bDrawVertexNormals);
	setvalue(p->add("Render mesh smoothing", 0, &BooleanPropCB<&EditorViewWindow::bDrawMeshSmoothing>,this), bDrawMeshSmoothing);

	p->add("View/Split Horizontally",0,SplitViewCB<true>,this);
	p->add("View/Split Vertically",0,SplitViewCB<false>,this);

	p->popup(fltk::Rectangle(PopupBoxW,PopupBoxH), "View Settings");
	delete p;
}

struct MergeViewData {
	IEditor *editor;
	EditorViewWindow *other;
	EditorViewWindow *own;
};

static void MergeViewCallback (fltk::Widget *w, void *data)
{
	MergeViewData *mvd = (MergeViewData*)data;
	mvd->editor->MergeView (mvd->own, mvd->other);
}

const int ViewMergeDistance=4;

void EditorViewWindow::ShowViewModifyMenu (int cx,int cy)
{
	vector <EditorViewWindow*> views=editor->GetViews ();

	const int d=ViewMergeDistance;
	EditorViewWindow *candidate = 0;

	for (int a=0;a<views.size();a++) 
	{
		EditorViewWindow *nb = views[a];
		if (nb == this) continue;

		bool hor=false,ver=false;
		if (cx < d && nb->x() + nb->w() == x()) hor = true;
		if (cx >= w()-d && nb->x () == w()+x()) hor = true;
		if (cy < d && nb->y() + nb->h() == y()) ver = true;
		if (cy >= h()-d && nb->y() == y()+h()) ver = true;

		// does it have the same vertical/horizontal dimensions?
		if ((hor && nb->h () == h() && nb->y() == y()) || 
			(ver && nb->w () == w() && nb->x() == x()))
		{
			candidate=nb;
			break;
		}
	}
	if (candidate) {
		MergeViewData mvd;
		mvd.editor = editor;
		mvd.other = candidate;
		mvd.own = this;

		fltk::Menu *p = new fltk::Menu (0,0,PopupBoxW,PopupBoxH);
		p->add ("Merge", 0, MergeViewCallback, &mvd);
		p->popup(fltk::Rectangle(cx + x(), cy + y(), PopupBoxW,PopupBoxH), 0);//"View config");
		delete p;
	}
}

float EditorViewWindow::GetConfig (int cfg)
{
	switch (cfg) {
		case CFG_OBJCENTERS:
			return true;//bDrawCenters ? 1 : 0;
		case CFG_DRAWRADIUS:
		case CFG_DRAWHEIGHT:// radius and height drawing are coupled atm
			return bDrawRadius ? 1 : 0;
		case CFG_POLYSELECT:{
			Tool *t=editor->GetTool();
			if (t) 	return t->needsPolySelect() ? 1 : 0;
			return 0;}
		case CFG_VRTNORMALS:
			return bDrawVertexNormals ? 1.0f : 0.0f;
		case CFG_MESHSMOOTH:
			return bDrawMeshSmoothing ? 1.0f : 0.0f;
	}
	return 0;
}


TextureHandler* EditorViewWindow::GetTextureHandler()
{
	return editor->GetTextureHandler();
}

/*
  FLTK event handler
 */
int EditorViewWindow::handle (int msg)
{
	if (msg == fltk::DRAG)
	{
		if (!bBoxSelect) {
			Tool *tool = editor->GetTool();
			if(tool) 
				tool->mouse (this, fltk::DRAG, Point (fltk::event_x() - last.x, fltk::event_y () - last.y));
		}
	}

	int r = ViewWindow::handle (msg);

	if (msg == fltk::PUSH) {
		if (click.x < PopupBoxW && click.y < PopupBoxH) {
			ShowPopupMenu ();
			return -1;
		}
	}

	return r;
}

void EditorViewWindow::ClickBorder(int cx,int cy)
{
	ShowViewModifyMenu (cx,cy);
}

void EditorViewWindow::PushSelector (ViewSelector *s)  {
	ViewWindow::PushSelector(s);
}
void EditorViewWindow::PopSelector () {
	ViewWindow::PopSelector();
}

void EditorViewWindow::Serialize(CfgList& cfg, bool store)
{
	ViewWindow::Serialize (cfg, store);

	Camera::CtlMode& camCtlMode=cam.ctlmode;
	if (store) {
		CFG_STOREN(cfg,mode);
		CFG_STOREN(cfg,rendermode);
		cfg.AddNumeric ("camCtlMode", (int)camCtlMode);
		CFG_STORE(cfg, bDrawCenters);
		CFG_STORE(cfg, bCullFaces);
		CFG_STORE(cfg, bLighting);
		CFG_STORE(cfg, bHideGrid);
		CFG_STORE(cfg, bDrawRadius);
	} else {
		CFG_LOADN(cfg,mode);
		SetMode(mode);
		CFG_LOADN(cfg,rendermode);
		camCtlMode=(Camera::CtlMode)cfg.GetInt ("camCtlMode",0);
		CFG_LOAD(cfg, bDrawCenters);
		CFG_LOAD(cfg, bCullFaces);
		CFG_LOAD(cfg, bLighting);
		CFG_LOAD(cfg, bHideGrid);
		CFG_LOAD(cfg, bDrawRadius);
	}
}
