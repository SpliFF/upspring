//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include "EditorIncl.h"
#include "EditorDef.h"
#include "Texture.h"
#include "ZipFile.h"
#include "Util.h"
#include "CfgParser.h"
#include "Image.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <GL/gl.h>

// ------------------------------------------------------------------------------------------------
// Texture
// ------------------------------------------------------------------------------------------------

CR_BIND(Texture,());
CR_REG_METADATA(Texture, (CR_MEMBER(name), CR_MEMBER(glIdent), CR_MEMBER(image)));

string Texture::textureLoadDir;

Texture::Texture() 
{
	glIdent = 0;
}

Texture::Texture (const string& fn) {
	Load (fn, string());
}

Texture::Texture (const string& fn, const string& hintpath) {
	Load (fn, hintpath);
}

bool Texture::Load (const string& fn, const string& hintpath)
{
	name = fltk::filename_name(fn.c_str());
	glIdent = 0;

	vector<string> paths;
	paths.push_back ("");
	if (!hintpath.empty()) paths.push_back (hintpath);
	if (!textureLoadDir.empty ()) paths.push_back (textureLoadDir + "/");

	bool succes=true;
	for(int a=0;a<paths.size();a++) {
		Image *img = new Image;
		try {
			img->Load ( (paths[a] + fn).c_str());
			succes = true;
		} catch(content_error& e) {
			logger.Print ("Failed to load texture: %s\n", e.errMsg.c_str());
			succes = false;
			delete img;
		}
		if (succes) {
			SetImage(img);
			break;
		}
	}
	return succes;
}

Texture::Texture (void *buf, int len, const char *_name)
{
	name = _name;
	glIdent = 0;

	Image *img = new Image;
	try {
		img->LoadFromMemory (buf, len);

	} catch(content_error& e) {
		delete img;
		logger.Trace (NL_Error, "Image loading exception: %s\n", e.what());
		return;
	}
	SetImage(img);
}

void Texture::SetImage(Image *img)
{
	image = img;
}

Texture::~Texture()
{
	if (glIdent) {
		glDeleteTextures (1, &glIdent);
		glIdent = 0;
	}
}

bool Texture::VideoInit ()
{
	if (!image)
		return false;

	bool mipmapped = true;
	GLenum linear = true;

	glGenTextures(1, &glIdent);
	glBindTexture(GL_TEXTURE_2D, glIdent);

	GLenum format = image->format.HasAlpha () ? GL_RGBA : GL_RGB;
	Image *conv = new Image;
	conv->format = ImgFormat(format == GL_RGBA ? ImgFormat::RGBA : ImgFormat::RGB);
	image->Convert (conv);
	image = conv;

	GLenum internalFormat = format;

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	if (mipmapped) {
		if (linear) 
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

		gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, conv->w, conv->h, format, GL_UNSIGNED_BYTE, conv->data);
	} else {
		if (linear) 
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		else
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, conv->w, conv->h, 0, format, GL_UNSIGNED_BYTE, conv->data);
	}
	return true;
}


// ------------------------------------------------------------------------------------------------
// TextureHandler
// ------------------------------------------------------------------------------------------------


TextureHandler::TextureHandler ()
{}


TextureHandler::~TextureHandler ()
{
	for (int a=0;a<zips.size();a++) {
		delete zips[a];
	}
	zips.clear();
	textures.clear();
}


Texture* TextureHandler::GetTexture(const char *name)
{
	string tmp = name;
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

	map<string,TexRef>::iterator ti = textures.find(tmp);
	if (ti == textures.end()) {
		tmp += "00";
		ti = textures.find(tmp);
		if (ti == textures.end()) {
			logger.Trace(NL_Debug,"Texture %s not found.\n", tmp.c_str());
			return 0;
		}
		ti->second.texture->name = name; // HACK: start using the name without the 00 now
		return ti->second.texture.Get();
	}

	return ti->second.texture.Get();
}

Texture* TextureHandler::LoadTexture (ZipFile*zf,int index, const char *name)
{
	int len=zf->GetFileLen (index);
	char *buf=new char[len];
	TError r = zf->ReadFile (index, buf);

	if (r==RET_FAIL) {
		logger.Trace (NL_Debug, "Failed to read texture file %s from zip\n",name);
		delete[] buf;
		return 0;
	}

	Texture *tex = new Texture (buf, len, name);
	if (!tex->IsLoaded ()) {
		delete[] buf;
		delete tex;
		return 0;
	}

	//logger.Trace(NL_Debug, "Texture %s loaded.\n", name);

	delete[] buf;
	return tex;
}



// KLOOTNOTE: replacement for strlwr()
void str2lwr(char* s) {
	for (int i = 0; s[i] != '\0'; i++)
		s[i] = tolower(s[i]);
}

static char* FixTextureName(char* temp) {
	// make it lowercase
	#ifdef WIN32
	strlwr(temp);
	#else
	str2lwr(temp);
	#endif

	// remove extension
	int i = strlen(temp) - 1;
	char* ext = 0;
	while (i > 0) {
		if (temp[i] == '.') {
			temp[i] = 0;
			ext = &temp[i + 1];
			break;
		}
		i--;
	}

	string fn;

	i = strlen(temp) - 1;
	while (i > 0) {
		if (temp[i] == '/' || temp[i] == '\\') {
			fn = &temp[i + 1];
			break;
		}
		i--;
	}
	strcpy(temp, fn.c_str());
	return ext;
}

bool TextureHandler::Load (const char *zip) 
{
	FILE *f = fopen (zip, "rb");

	if (f) {
		ZipFile *zf = new ZipFile;
		if (zf->Init (f) == RET_FAIL) {
			logger.Trace (NL_Error, "Failed to load zip archive %s\n", zip);
			fclose (f);
			return false;
		}

		const char *imgExt[]={ "bmp", "jpg", "tga", "png", "dds", "pcx", "pic", "gif", "ico", 0 };

		// Add the zip entries to the texture set
		for (int a=0;a<zf->GetNumFiles ();a++)
		{
			char tempFile[64];
			zf->GetFilename (a, tempFile, sizeof(tempFile));

			char *ext=FixTextureName (tempFile);
			if (ext)
			{
				int x=0;
				for (x=0;imgExt[x];x++)
					if (!strcmp(imgExt[x],ext)) break;

				if (!imgExt[x])
					continue;

				if (textures.find(tempFile) != textures.end())
					continue;

				TexRef ref;
				ref.zip = zips.size();
				ref.index = a;
				ref.texture = LoadTexture (zf,a, tempFile);

				if(textures.find(tempFile)!=textures.end())
					logger.Trace(NL_Debug,"Texture %s already loaded\n", tempFile);

				TexRef &tr = textures[tempFile];
				tr.texture = ref.texture;
				tr.index = a;;
				tr.zip = zips.size();
			}
		}

		fclose (f);

		zips.push_back (zf);
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
// TextureGroupHandler
// ------------------------------------------------------------------------------------------------

TextureGroupHandler::TextureGroupHandler (TextureHandler *th) 
{
	textureHandler = th;
}

TextureGroupHandler::~TextureGroupHandler ()
{
	for (int a=0;a<groups.size();a++) {
		delete groups[a];
	}
	groups.clear();
}

bool TextureGroupHandler::Load(const char *fn)
{
	CfgList *cfg = CfgValue::LoadFile (fn);

	if (!cfg) 
		return false;

	for (list<CfgListElem>::iterator li = cfg->childs.begin(); li != cfg->childs.end(); ++li) {
		CfgList *gc = dynamic_cast<CfgList*>(li->value);
		if (!gc) continue;

		LoadGroup (gc);
	}

	delete cfg;
	return true;
}

bool TextureGroupHandler::Save(const char *fn)
{
	// open a cfg writer
	CfgWriter writer(fn);
	if (writer.IsFailed()) {
		fltk::message ("Unable to save texture groups to %s\n", fn);
		return false;
	}

	// create a config list and save it
	CfgList cfg;

	for (int a=0;a<groups.size();a++) {
		CfgList *gc=MakeConfig(groups[a]);

		char n [10];
		sprintf (n, "group%d", a);
		cfg.AddValue (n, gc);
	}

	cfg.Write(writer,true);
	return true;
}

TextureGroup* TextureGroupHandler::LoadGroup (CfgList *gc) {
	CfgList *texlist = dynamic_cast<CfgList*>(gc->GetValue("textures"));
	if (!texlist) return 0;

	TextureGroup *texGroup=new TextureGroup;
	texGroup->name = gc->GetLiteral("name", "unnamed");

	for (list<CfgListElem>::iterator i=texlist->childs.begin();i!=texlist->childs.end();i++) {
		CfgLiteral *l=dynamic_cast<CfgLiteral*>(i->value);
		if (l && !l->value.empty()) {
			Texture *texture = textureHandler->GetTexture(l->value.c_str());
			if (texture) 
				texGroup->textures.insert(texture);
			else {
				logger.Trace(NL_Debug, "Discarded texture name: %s\n", l->value.c_str());
			}
		}
	}
	groups.push_back(texGroup);
	return texGroup;
}

CfgList* TextureGroupHandler::MakeConfig (TextureGroup *tg)
{
	CfgList *gc = new CfgList;
	char n [10];

	CfgList *texlist=new CfgList;
	int index=0;
	for (set<Texture*>::iterator t=tg->textures.begin();t!=tg->textures.end();++t) {
		sprintf (n,"tex%d", index++);
		texlist->AddLiteral (n, (*t)->name.c_str());
	}
	gc->AddValue ("textures", texlist);
	gc->AddLiteral ("name", tg->name.c_str());
	return gc;
}

/*
TextureAtlas::TextureAtlas()
{
}

static int texCompareFunc(Texture *a, Texture *b)
{
	return a->height - b->height;
}

Texture* TextureAtlas::Process(int w, int h, const std::vector<Texture*>& src)
{
	std::map <int, std::vector <Texture*> > texmth;

	for (int a=0;a<src.size();a++)
		texmth [src[a]->height].push_back (src[a]);

	int surf = 0;

	if (w < 0 || h < 0) {
		for (int a=0;a<src.size();a++)
		{
			Texture * t = src[a];
			surf += t->width * t->height;
		}
		w = sqrtf (surf);
		h = sqrtf (surf);

		int x=1,y=1;
		while (x < w) x*=2;
		while (y < h) y*=2;
	}

	this->w = w;
	this->h = h;

	for (int a=0;a<sorted.size();a++)
		AddTexture (sorted[a]);

	return 0;
}

TextureAtlas::Slot* TextureAtlas::AddTexture(Texture *t)
{
	HeightSet& s = hs [t->height];
	
	if (s.totalw + t->width > w) {
		s.row ++;
		s.totalw = 0;
	}
	Slot slot;
	slot.x = totalw;
	slot.w = t->width;
	slot.h = t->height;
	s.slots.push_back (slot);

	totalw += slot.w;

	return &s.slots.back();
}


*/


// ------------------------------------------------------------------------------------------------
// Texture storage class based on a binary tree
// ------------------------------------------------------------------------------------------------

TextureBinTree::Node::Node ()
{
	x = y = w = h = 0;
	img_w = img_h = 0;
	child [0] = child[1] = 0;
}

TextureBinTree::Node::Node (int X,int Y,int W,int H)
{
	x = X; y = Y; w = W; h = H;
	img_w = img_h = 0;
	child [0] = child[1] = 0;
}



TextureBinTree::Node::~Node()
{
	SAFE_DELETE (child[0]);
	SAFE_DELETE (child[1]);
}

TextureBinTree::TextureBinTree ()
{
	tree = 0;
}

TextureBinTree::~TextureBinTree()
{
	SAFE_DELETE(tree);
}

void TextureBinTree::Init (int w,int h, ImgFormat *fmt)
{
	texture.Alloc (w, h, *fmt);
}

void TextureBinTree::StoreNode (Node *n, Image *tex)
{
	n->img_w = tex->w;
	n->img_h = tex->h;

	tex->Blit (&texture, 0, 0, n->x, n->y, tex->w, tex->h);
}

TextureBinTree::Node* TextureBinTree::InsertNode (Node* n, int w, int h)
{
	if (n->child [0] || n->child [1]) // not a leaf node ?
	{
		Node *r = 0;
		if (n->child [0]) r = InsertNode (n->child [0], w, h);
		if (r) return r;
		if (n->child [1]) r = InsertNode (n->child [1], w, h);
		return r;
	}
	else
	{
		// Occupied
		if (n->img_w)
			return 0;

		// Does it fit ?
		if (n->w < w || n->h < h)
			return 0;

		if (n->w == w && n->h == h)
			return n;

		int ow = n->w - w, oh = n->h - h;
		if (ow > oh)
		{
			// Split vertically
			if (ow) n->child [0] = new Node (n->x + w, n->y, ow, n->h);
			if (oh) n->child [1] = new Node (n->x, n->y + h, w, oh);
		}
		else
		{
			// Split horizontally
			if (ow) n->child [0] = new Node (n->x + w, n->y, ow, h);
			if (oh) n->child [1] = new Node (n->x, n->y + h, n->w, oh);
		}

		return n;
	}

	return 0;
}

TextureBinTree::Node* TextureBinTree::AddNode (Image *subtex)
{
	Node *pn;

	if (!tree)
	{
		// create root node
		if (subtex->w > texture.w || subtex->h > texture.h)
			return 0;

		tree = new Node (0,0, texture.w, texture.h);
	}

	pn = InsertNode (tree, subtex->w, subtex->h);
	if (!pn) return 0;
	
	StoreNode (pn, subtex);
	return pn;
}


