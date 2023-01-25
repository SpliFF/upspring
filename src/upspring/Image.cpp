#include "EditorIncl.h"
#include "EditorDef.h"

#include "Image.h"
#include "Util.h"

// If defined, use SDL_image, otherwise use DevIL/OpenIL
//#define USE_SDL_IMAGE 

#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#else
#include <IL/il.h>
#include <IL/ilu.h>

struct DevILInitializer
{
	DevILInitializer()
	{
		ilInit();
		iluInit();
	}
	~DevILInitializer()
	{
		ilShutDown();
	}
} static devilInstance;
#endif


// define this to use color blending with mipmap generation
#define BLEND_MIPMAP

// ------------------------------ ImgFormat -------------------------------

ImgFormat::ImgFormat(Type t)
{
	for(int a=0;a<4;a++) 
		mask[a]=shift[a]=loss[a]=0;
	bytesPerPixel=0; 
	type = t;

	switch (type) {
		case RGB:
		case BGR:
			bytesPerPixel = 3;
			if (type == RGB){ 
				shift[0] = 16;
				shift[1] = 8;
				shift[2] = 0;
			} else {
				shift[2] = 16;
				shift[1] = 8;
				shift[0] = 0;
			}
			loss[3] = 8;
			break;
		case RGBA:
		case BGRA:
			bytesPerPixel = 4;
			if (type == RGBA) {
				shift[0] = 24;
				shift[1] = 16;
				shift[2] = 8;
			} else {
				shift[2] = 24;
				shift[1] = 16;
				shift[0] = 8;
			}
			break;
		case ABGR:
			bytesPerPixel = 4;
			shift[0] = 0;
			shift[1] = 8;
			shift[2] = 16;
			shift[3] = 24;
			break;
		case ARGB:
			bytesPerPixel = 4;
			shift[0] = 16;
			shift[1] = 8;
			shift[2] = 0;
			shift[3] = 24;
			break;
		case RGB565:
		case BGR565:
			bytesPerPixel = 2;
			if (type == RGB565) {
				shift[0] = 11;
				shift[1] = 5;
			} else {
				shift[2] = 11;
				shift[1] = 5;
			}
			loss[0] = loss[2] = 3;
			loss[1] = 2;
			loss[3] = 8;
			break;
		case LUMINANCE:
			bytesPerPixel = 1;
			loss[3]=8;
			mask[0]=mask[1]=mask[2]=0xff;
			break;
		case LUMINANCE_ALPHA:
			bytesPerPixel = 2;
			mask[0]=mask[1]=mask[2]=0xff;
			mask[3]=0xff00;
			break;
		case UNDEF:
			break;
	}
	CalcMask ();
}

void ImgFormat::CalcMask ()
{
	for(int a=0;a<4;a++)
		mask[a] = (0xff >> loss[a]) << shift[a];
}

void ImgFormat::CalcLossShift()
{
	uint m;
	for(int a=0;a<4;a++)
	{
		shift[a] = 0;
		loss[a] = 8;
		if (mask[a]) {
			for (m = mask[a]; !(m&1); m >>= 1)
				++shift[a];
			for (;m&1; m >>= 1)
				--loss[a];
		}
	}
}



// -------------------------------- Image ---------------------------------

Image::Image(const char *file)
{
	data = 0;
	Load(file);
}

Image::Image()
{
	data = 0;
	w = h = 0;
}

Image::Image (int _w, int _h, const ImgFormat& fmt)
{
	Alloc(_w,_h, fmt);
}

Image::~Image()
{
	if(data) { delete[] data; data=0; }
	w=h=0;
}

void Image::Free()
{
	if(data) {delete[] data; data=0; }
	w=h=0;
}

void Image::Alloc (int _w, int _h, const ImgFormat& fmt)
{
	format = fmt;
	w = _w;
	h = _h;
	data = new uchar[w*h*format.bytesPerPixel];
	assert(data);
	memset (data, 0, w*h*format.bytesPerPixel);
}

// 0xRRGGBBAA
void Image::SetPixel(int x, int y, uint src)
{
	uint c = format.Make ( (src & 0xff000000) >> 24, (src & 0xff0000) >> 16, (src & 0xff00) >> 8, (src & 0xff) >> 0);

	switch (format.bytesPerPixel) {
		case 1: data[y*w+x] = (uchar)c; break;
		case 2: ((ushort*)data)[y*w+x] = (ushort)c; break;
		case 3: 
			data[3*(y*w+x)+0] = (src & 0xff);
			data[3*(y*w+x)+1] = (src & 0xff00) >> 8;
			data[3*(y*w+x)+0] = (src & 0xff0000) >> 24;
			break;
		case 4: ((uint*)data)[y*w+x] = c; break;
	}
}

#ifdef USE_SDL_IMAGE


void Image::LoadGrayscale (void *buf, int len)
{
}

static void ImgFormatToSDLPF(ImgFormat& format, SDL_PixelFormat& pf)
{
	pf.alpha = 0;
	pf.colorkey = 0;
	pf.Rloss = format.loss [0];
	pf.Gloss = format.loss [1];
	pf.Bloss = format.loss [2];
	pf.Aloss = format.loss [3];

	pf.Rmask = format.mask [0];
	pf.Gmask = format.mask [1];
	pf.Bmask = format.mask [2];
	pf.Amask = format.mask [3];

	pf.Rshift = format.shift [0];
	pf.Gshift = format.shift [1];
	pf.Bshift = format.shift [2];
	pf.Ashift = format.shift [3];

	pf.BytesPerPixel = format.bytesPerPixel;
	pf.BitsPerPixel = format.bytesPerPixel * 8;
}

void Image::LoadFromMemory (void *buf, int len)
{
	SDL_RWops *rw = SDL_RWFromMem(buf, len);

	SDL_Surface *img = IMG_Load_RW(rw, 1);
	if (!img) 
		throw content_error(IMG_GetError());

	Free();

	// does it have alpha?
	bool hasAlpha;
	if (img->format->BytesPerPixel == 1) // paletted?
		hasAlpha = false;
	else
		hasAlpha = img->format->Amask != 0;

	if (hasAlpha)
		format.LoadRGBA32();
	else
		format.LoadRGB24();

	SDL_PixelFormat dstFormat;
	ImgFormatToSDLPF (format, dstFormat);

	SDL_Surface *dst = SDL_ConvertSurface(img, &dstFormat, SDL_SWSURFACE);
	SDL_LockSurface(dst);
	Alloc();

	SDL_UnlockSurface(dst);

	SDL_FreeSurface(dst);
	SDL_FreeSurface(img);
}

#else

// DevIL 


/* In a grayscale picture,
 red bits are 8 and the rest is 0 */
void Image::LoadGrayscale (void *buf, int len)
{
	uint id;
	int bpp;

	ilGenImages (1, &id);
	ilBindImage (id);

	ILboolean ret;

	ilDisable (IL_CONV_PAL);
	ilHint (IL_MEM_SPEED_HINT, IL_FASTEST);

	ret = ilLoadL(IL_TYPE_UNKNOWN, buf, len);
	if(ret == IL_FALSE) {
		ilDeleteImages (1, &id);
		ILenum err = ilGetError();
		throw content_error((const char *)iluErrorString(err));
	}

	w = ilGetInteger (IL_IMAGE_WIDTH);
	h = ilGetInteger (IL_IMAGE_HEIGHT);
	bpp = ilGetInteger (IL_IMAGE_BITS_PER_PIXEL);
	if(bpp != 8)
		throw content_error(SPrintf("Grayscale image should be 8 bit, instead of %d", bpp));

	Alloc (w, h, ImgFormat(ImgFormat::LUMINANCE));

	ilCopyPixels (0, 0, 0, w, h, 1, IL_COLOR_INDEX, IL_UNSIGNED_BYTE, data);
	ilDeleteImages (1, &id);
}

void Image::LoadFromMemory (void *buf, int len)
{
	uint id;

	ilGenImages (1, &id);
	ilBindImage (id);

	ILboolean ret;

	/* Convert paletted to packed colors */
	ilEnable (IL_CONV_PAL);
	ilHint (IL_MEM_SPEED_HINT, IL_FASTEST);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	ilEnable (IL_ORIGIN_SET);

	ret = ilLoadL(IL_TYPE_UNKNOWN, buf, len);
	if(ret == IL_FALSE) {
		ilDeleteImages (1, &id);
		ILenum err = ilGetError();
		throw content_error((const char *)iluErrorString(err));
	}

	FromIL(id);

	iluDeleteImage(id);
}

void Image::FromIL(uint id)
{
	ilBindImage(id);

	w = ilGetInteger (IL_IMAGE_WIDTH);
	h = ilGetInteger (IL_IMAGE_HEIGHT);
	int bpp = ilGetInteger (IL_IMAGE_BYTES_PER_PIXEL);

	// See if the format is supported
/*	ImgFormat::Type dstfmt = ImgFormat::UNDEF;
	ILint srcfmt = ilGetInteger (IL_IMAGE_FORMAT);
	if (srcfmt == IL_RGB)
		dstfmt = ImgFormat::RGB;
	else if (srcfmt == IL_RGBA)
		dstfmt = ImgFormat::RGBA;
	else if (srcfmt == IL_BGRA)
		dstfmt = ImgFormat::BGRA;
	else if (srcfmt == IL_BGR)
		dstfmt = ImgFormat::BGR;
	else if (srcfmt == IL_LUMINANCE)
		dstfmt = ImgFormat::LUMINANCE;
	else if (srcfmt == IL_LUMINANCE_ALPHA)
		dstfmt = ImgFormat::LUMINANCE_ALPHA;
	else {
		// convert
		dstfmt = ImgFormat::RGBA;
		srcfmt = IL_RGBA;
	}*/

	if (bpp == 4) {
		Alloc(w,h,ImgFormat(ImgFormat::RGBA));
		ilCopyPixels (0, 0, 0, w, h, 1, IL_RGBA, IL_UNSIGNED_BYTE, data);
	}
	else if (bpp == 3) {
		Alloc(w,h,ImgFormat(ImgFormat::RGB));
		ilCopyPixels (0, 0, 0, w, h, 1, IL_RGB, IL_UNSIGNED_BYTE, data);
	}
}

uint Image::ToIL()
{
	uint id;
	ilGenImages(1,&id);
	ilBindImage(id);

	Image dst;
	if (format.bytesPerPixel == 3)
		dst.format=ImgFormat(ImgFormat::RGB);
	else
		dst.format=ImgFormat(ImgFormat::RGBA);
	Convert(&dst);

	uint fmt;
	if (format.bytesPerPixel == 4)
		fmt = IL_RGBA;
	else 
		fmt = IL_RGB;

	if (format.bytesPerPixel == 3)
		ilTexImage(w,h,1, 3, fmt, IL_UNSIGNED_BYTE, dst.data);
	else
		ilTexImage(w,h,1, 4, fmt, IL_UNSIGNED_BYTE, dst.data);

	assert (ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL) == 3);
	return id;
}


bool Image::Save(const char *file)
{
	uint id = ToIL();

	ilEnable(IL_FILE_OVERWRITE);
	ilBindImage(id);
	bool r= ilSaveImage((const ILstring) file);
	iluDeleteImage(id);
	return r;
}

#endif

void Image::Load (const char *file, bool /*IsGrayscale*/)
{
	/*
	FileReader fr (file);

	if (!fr.Exists())
		throw content_error("Failed to open file " + String(file));

	uint len = fr.Length();
	uchar *buf = new uchar[len];*/

	FILE *f = fopen (file,  "rb");

	if (!f) 
		throw content_error("Failed to open file " + string(file));

	fseek(f, 0, SEEK_END);
	int len=ftell(f);
	fseek(f,0,SEEK_SET);
	uchar *buf=new uchar[len];
	if (fread(buf,len,1,f)) {}

	try {
		LoadFromMemory(buf, len);
	} catch(const content_error& e) {
		delete[] buf;
		throw content_error(SPrintf("Error loading image %s: %s", file, e.what()));
	}
	delete[] buf;
}

Image* Image::Clone()
{
	Image *dst = new Image;
	dst->Alloc (w,h,format);
	memcpy(dst->data, data, w*h*format.bytesPerPixel);
	return dst;
}

/*
Saves a 24 bit uncompressed TGA image (simple RGB formatting)
*/
void Image::SaveTGA (const char *file)
{
	// open file
	FILE *pFile=fopen(file, "wb");
	if(!pFile)
		throw std::runtime_error(SPrintf("Failed to open file '%s' for writing", file));

	// write header
	char targaheader[18];

	// Set unused fields of header to 0
	memset(targaheader, 0, sizeof(targaheader));

	targaheader[2] = 2;     /* image type = uncompressed gray-scale */
	targaheader[12] = (char) (w & 0xFF);
	targaheader[13] = (char) (w >> 8);
	targaheader[14] = (char) (h & 0xFF);
	targaheader[15] = (char) (h >> 8);
	targaheader[16] = 24; /* 24 bit color */
	targaheader[17] = 0x20; /* Top-down, non-interlaced */

	fwrite(targaheader, 18, 1, pFile);

	// write file
	uint *ps = (uint*)data;
	uchar **pos;
	uchar out[3];

	pos = (uchar**)&ps;
	for(int y=0; y<h; y++)
	{
		for(int x=0;x<w;x++)
		{
			// red
			out [0] = ((*ps) & format.mask[0]) >> format.shift[0] << format.loss[0];
			// green
			out [1] = ((*ps) & format.mask[1]) >> format.shift[1] << format.loss[1];
			// blue
			out [2] = ((*ps) & format.mask[2]) >> format.shift[2] << format.loss[2];

			fwrite (out, 3, sizeof(uchar), pFile);

			(*pos) += format.bytesPerPixel;
		}
	}
	if(pFile) fclose(pFile);
}


/*
Convert an image to another format
dst->format contains the destination format, to which the image is converted.
*/
void Image::Convert(Image *dst)
{
	uchar **dstBytePos, **srcBytePos;
	uint *ps, *pd;
	int x,y,r,g,b,a;

	if(dst->w != w || dst->h != h)
	{
		dst->Free ();
		dst->Alloc (w,h,dst->format);
	}

	if(!memcmp (&dst->format, &format, sizeof(ImgFormat)))
	{
		memcpy (dst->data, data, format.bytesPerPixel * w * h);
		return;
	}

	if(!dst->data) 
		dst->Alloc (w,h,dst->format);

	ps = (uint*)data;
	pd = (uint*)dst->data;
	dstBytePos = (uchar**)&pd;
	srcBytePos = (uchar**)&ps;

	ImgFormat *df = &dst->format;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			for(a=0;a<int(df->bytesPerPixel);a++)
				(*dstBytePos)[a] = 0;

			r = ((*ps)&format.mask[0]) >> format.shift[0];
			r <<= format.loss[0];
			g = ((*ps)&format.mask[1]) >> format.shift[1];
			g <<= format.loss[1];
			b = ((*ps)&format.mask[2]) >> format.shift[2];
			b <<= format.loss[2];
			a = ((*ps)&format.mask[3]) >> format.shift[3];
			a <<= format.loss[3];

			*pd |= ((r >> df->loss[0]) << df->shift[0]) & df->mask[0];
			*pd |= ((g >> df->loss[1]) << df->shift[1]) & df->mask[1];
			*pd |= ((b >> df->loss[2]) << df->shift[2]) & df->mask[2];
			*pd |= ((a >> df->loss[3]) << df->shift[3]) & df->mask[3];

			// go to next pixel
			*dstBytePos += dst->format.bytesPerPixel;
			*srcBytePos += format.bytesPerPixel;
		}
	}
}

void Image::FillAlpha ()
{
	int y;
	int x;

	uchar **pp;
	uint *p;

	pp = (uchar**)&p;
	p = (uint*)data;

	for(y=0;y<h;y++)
		for(x=0;x<w;x++)
		{
			if((*p & format.mask[0]) || (*p & format.mask[1]) || (*p & format.mask[2]))
				*p |= format.mask[3];
			else
				*p &= ~format.mask[3];
			*pp += format.bytesPerPixel;
		}
}

#define DIV4MASK_32 ((63<<24) + (63<<16) + (63<<8) + 63)
#define DIV4MASK_16 ((31<<11) + (63<<5) + 31)

/*
format can be 16 bit (565) or 32 bit (8888)
the image must have proper dimensions (like 256x128 or 64x64)
*/
bool Image::GenMipmap (Image *dst)
{
	int x, y;

	// only 32 bit (GL) and 16 bit (SW) are currently supported
	assert(format.bytesPerPixel==4 || format.bytesPerPixel==2);

	if(w < 2 || h < 2)
		return false;

	if(w % 2 > 0 || h % 2 > 0)
		return false;

	dst->Alloc (w/2,h/2, format);

	int h2=h/2, w2=w/2, sx, sy;
	uint col[4];
	for(y = 0; y < h2; y ++)
	{
		for(x = 0; x < w2; x ++)
		{
#           ifdef BLEND_MIPMAP
			sx = x*2; sy = y*2;
			// blend the colors of four pixels to one
			if(format.bytesPerPixel == 4)
			{
				col[0] = (GetPixel32(sx,sy) >> 2) & DIV4MASK_32;
				col[1] = (GetPixel32(sx+1,sy) >> 2) & DIV4MASK_32;
				col[2] = (GetPixel32(sx,sy+1) >> 2) & DIV4MASK_32;
				col[3] = (GetPixel32(sx+1,sy+1) >> 2) & DIV4MASK_32;
				dst->SetPixel32 (x,y, col[0] + col[1] + col[2] + col[3]);
			} 
			else {
				col[0] = (GetPixel16(sx,sy) >> 2) & DIV4MASK_16;
				col[1] = (GetPixel16(sx+1,sy) >> 2) & DIV4MASK_16;
				col[2] = (GetPixel16(sx,sy+1) >> 2) & DIV4MASK_16;
				col[3] = (GetPixel16(sx+1,sy+1) >> 2) & DIV4MASK_16;
				dst->SetPixel16 (x,y, col[0] + col[1] + col[2] + col[3]);
			}
#           else
			if(format.bytesPerPixel==2)
				dst->SetPixel16(x, y, GetPixel16 (x*2,y*2) );
			else
				dst->SetPixel32(x, y, GetPixel32 (x*2,y*2) );
#           endif
		}
	}

	return true;
}

/*
This function is not intended to actually draw things (it doesn't do any clipping), 
it is just a way to copy certain parts of an image.
The formats have to be exactly the same
*/
void Image::Blit (Image *dst, int sx, int sy, int dx, int dy, int bw,int bh)
{
	int y; // int x;
	assert(!memcmp (&format, &dst->format, sizeof(ImgFormat)));

	uchar *srcptr;
	uchar *dstptr;

	for(y=0;y<bh;y++)
	{
		srcptr = &data[( (y+sy)*w+sx) * format.bytesPerPixel];
		dstptr = &dst->data[((y+dy)*dst->w+dx) * format.bytesPerPixel];

		memcpy (dstptr, srcptr, format.bytesPerPixel * bw);
	}
}


