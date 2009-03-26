//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef JC_TEXTURE_H
#define JC_TEXTURE_H

#include "Referenced.h"
#include "Image.h"

class ZipFile;
class CfgList;

class Texture : public Referenced
{
	CR_DECLARE(Texture);
public:
	Texture ();
	Texture (const string& filename);
	Texture (const string& filename, const string& hintpath);
	Texture (void *buf, int len, const char *fn);
	~Texture ();

	bool Load (const string& filename, const string& hintpath);
	bool IsLoaded() { return image; }
	bool VideoInit ();
	void SetImage (Image *img);
	int Width() { return image->w; }
	int Height() { return image->h; }

	uint glIdent;
	string name;
	RefPtr<Image> image;

	static string textureLoadDir;
};

// manages 3do textures
class TextureHandler
{
public:
	TextureHandler ();
	~TextureHandler ();

	bool Load (const char *zip); // load archive
	Texture* GetTexture (const char *name);

protected:
	Texture* LoadTexture (ZipFile *zf, int index, const char *name);

	struct TexRef {
		TexRef (){zip=index=0; }

		int zip;
		int index;
		RefPtr<Texture> texture;
	};

	vector <ZipFile *> zips;
	map <string, TexRef> textures;

	friend class TexGroupUI;
};


class TextureGroup
{
public:
	string name;
	set <Texture *> textures;
};

class TextureGroupHandler
{
public:
	TextureGroupHandler(TextureHandler *th);
	~TextureGroupHandler();

	bool Load (const char *fname);
	bool Save (const char *fname);

	CfgList* MakeConfig (TextureGroup *tg);
	TextureGroup* LoadGroup (CfgList *cfg);


	vector <TextureGroup*> groups;
	TextureHandler *textureHandler;
};


/*
Collects a set of small textures and creates a big one out of it
used for lightmap packing
*/
class TextureBinTree
{
public:
	struct Node
	{
		Node ();
		Node (int X, int Y, int W, int H);
		~Node ();

		int x,y,w,h;
		int img_w, img_h;
		Node *child[2];
	};

	TextureBinTree ();
	~TextureBinTree ();

	Node* AddNode (Image *subtex);
	
	void Init (int w, int h, ImgFormat *fmt);
	bool IsEmpty () { return !tree; }

	inline float GetU (Node *n, float u) { return u * (float)n->img_w / float (texture.w) + (n->x + 0.5f) / texture.w; }
	inline float GetV (Node *n, float v) { return v * (float)n->img_h / float (texture.h)+ (n->y + 0.5f) / texture.h; }

	// Unused by class: uint for storing texture rendering id
	uint render_id;

	Image* GetResult() { return &texture; }

protected:
	void StoreNode (Node *pm, Image *tex);
	Node* InsertNode (Node *n, int w, int h);

	Node *tree;
	Image texture;
};

/*
class TextureAtlas
{
public:
	TextureAtlas();

	Texture* Process(const std::vector <Texture*>& src);
protected:
	int w,h;

	struct Slot {
		int x, w, h;
		Texture *texture;
	};

	struct HeightSet {
		HeightSet() { totalw=row=0; }
		int totalw;
		int row;
		std::list<Slot> slots;
	}
	std::map<int, HeightSet> hs;

	Slot* AddTexture(Texture *t);
};
*/

#endif // JC_TEXTURE_H
