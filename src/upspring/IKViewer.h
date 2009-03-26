
#include "View.h"


class IKViewer : public ViewWindow
{
public:
	IKViewer (int X,int Y,int W,int H, const char *lbl=0) :
	  ViewWindow(X,Y,W,H,lbl) {}

	void DrawScene ();
	void Draw2D ();
	void GetViewMatrix (Matrix &m);
	void SetupProjectionMatrix ();
};



