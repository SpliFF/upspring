//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#ifndef JC_VIEWS_GROUP_H
#define JC_VIEWS_GROUP_H
#include <fltk/TiledGroup.h>

class ViewsGroup : public fltk::TiledGroup
{
public:
	ViewsGroup (int X,int Y,int W,int H, const char *l=0) :
		TiledGroup (X,Y,W,H,l) { wasIconic=false; lastWidth=W; lastHeight=H; suppressRescale=false; }

	void layout () {
		fltk::Window* pwnd = (fltk::Window *)parent();
		if (pwnd->iconic ()) {
			// store view dimensions
			viewDims.resize (children ());
			for (int a=0;a<children();a++) {
				fltk::Widget *w = child (a);
				Rectangle dim(w->x(),w->y(),w->w(),w->h());
				viewDims[a] = dim;
			}
			wasIconic=true;
		} else if(wasIconic) {
			// restore view dimensions
			for (int a=0;a<children() && a<int(viewDims.size()); a++) {
				fltk::Widget *w = child (a);
				Rectangle& dim=viewDims[a];
				w->resize(dim.x(),dim.y(),dim.w(),dim.h());
			}
			wasIconic=false;
		} else if (!suppressRescale) { // this is a normal resize, scale all child-widgets
			float xs=w()/(float)lastWidth, ys=h()/(float)lastHeight;
			for (int a=0;a<children();a++) {
				fltk::Widget *ch = child (a);
				ch->resize(ch->x()*xs+0.5f,ch->y()*ys+0.5f,ch->w()*xs+0.5f,ch->h()*ys+0.5f);
			}
		}
		lastWidth = w();
		lastHeight = h();

		fltk::TiledGroup::layout();
	}
	bool isWindowEdge(int x,int y, fltk::Window *wnd)
	{
		const int bs=3; // bordersize

		if (x<0 || y<0 || x>=wnd->w() || y>=wnd->h())
			return false;

		return x<bs || y<bs || x>wnd->w()-bs || y>wnd->h()-bs;
	}
	int handle(int event) {
		if (event == fltk::PUSH && fltk::event_button ()==3)
		{
			// see if the click is one the edge of a child window
			for (int a=0;a<children();a++) {
				EditorViewWindow *v = (EditorViewWindow *)child(a);
				int x=fltk::event_x()-v->x(), y=fltk::event_y()-v->y();
				if (isWindowEdge (x,y, v)) {
					v->ClickBorder (x,y);
					return 0;
				}
			}
		}

		return fltk::TiledGroup::handle(event);
	}

	int lastWidth, lastHeight;
	vector <Rectangle> viewDims;
	bool wasIconic, suppressRescale;
};

#endif
