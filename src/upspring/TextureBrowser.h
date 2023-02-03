//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef JC_TEXTURE_BROWSER_H
#define JC_TEXTURE_BROWSER_H

#include "Texture.h"

typedef void (*TextureSelectCallback)(Texture *tex, void *data);

class TextureBrowser : public fltk::ScrollGroup
{
public:
	TextureBrowser (int X,int Y, int W,int H,const char *label=0);

	//void resize(int x,int y,int w,int h);
	void layout();

	struct Item : public fltk::Widget {
		Item (Texture *t);
		Texture *tex;
		bool selected;

		int handle (int event);
		void draw();
	};

	void AddTexture (Texture *t);
	void RemoveTexture (Texture *t);
	void UpdatePositions (bool bRedraw=true);
	std::vector<Texture*> GetSelection();
	void SelectAll();
	void SetSelectCallback (TextureSelectCallback cb, void *data);

protected:
	int prevWidth; 
	TextureSelectCallback selectCallback;
	void *selectCallbackData;

	void SelectItem (Item *i);
};


#endif 
