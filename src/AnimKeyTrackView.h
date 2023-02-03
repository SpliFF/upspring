
#include <fltk/ScrollGroup.h>

class AnimKeyTrackView : public fltk::Group
{
public:
	AnimKeyTrackView(int X,int Y,int W,int H, const char *label=0);

	void draw();
	int handle(int event);

	void SelectKeys (int x,int y);

	float minTime, maxTime;
	float minY, maxY;
	float curTime;
	
	bool movingView, movingKeys, zooming;
	int xpos, ypos;
	int clickx, clicky;

	AnimTrackEditorUI* trackUI;
};

