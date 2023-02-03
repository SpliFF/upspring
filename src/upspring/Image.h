
#ifndef CMD_IMAGE_H
#define CMD_IMAGE_H

#include "Referenced.h"

struct ImgFormat
{
	// RGBA
	uint mask[4];
	uint shift[4];
	uint loss[4];
	uint bytesPerPixel;

	enum Type 
	{
		UNDEF,
		RGB,  // 24 bit
		RGBA,  // 32 bit
		BGR,
		BGRA,
		RGB565,
		BGR565,
		LUMINANCE,
		LUMINANCE_ALPHA,
		ABGR,
		ARGB,
	};
	Type type;

	ImgFormat(Type t = UNDEF);

	bool HasAlpha()
	{
		return loss[3] != 8;
	}

	uint Make(int* /*cols*/)
	{
		uint r = 0;
		for (int x=0;x<4;x++)
			r |= ((r<<shift[0] >> loss[0])&mask[0]);
		return r;
	}

	uint Make(int r,int g,int b) 
	{
		return ((r<<shift[0] >> loss[0])&mask[0]) |
			((g<<shift[1] >> loss[1])&mask[1]) |
			((b<<shift[2] >> loss[2])&mask[2]);
	}

	uint Make(int r,int g,int b,int /*a*/)
	{
		return ((r<<shift[0] >> loss[0])&mask[0]) |
			((g<<shift[1] >> loss[1])&mask[1]) |
			((b<<shift[2] >> loss[2])&mask[2]) |
			((b<<shift[3] >> loss[3])&mask[3]);
	}

	uint R(uint v) { return (v&mask[0]) >> shift[0] << loss[0]; }
	uint G(uint v) { return (v&mask[1]) >> shift[1] << loss[1]; }
	uint B(uint v) { return (v&mask[2]) >> shift[2] << loss[2]; }
	uint A(uint v) { return (v&mask[3]) >> shift[3] << loss[3]; }

	void CalcLossShift(); // calculate loss and shift from mask
	void CalcMask (); // calculate mask from loss and shift
	void Swap4 (); // RGBA <-> ABGR
	void Swap3 (); // RGB <-> BGR
	void LoadRGBA32();
	void LoadRGB24();
	void LoadRGB16();
	void LoadGrayscale (); // mask[R] = mask[G] = mask[B] = 255

	void LoadABGR32() { LoadRGBA32(); Swap4(); }

	void SetShift (uint r, uint g, uint b, uint a)
	{ shift[0]=r; shift[1]=g; shift[2]=b; shift[3]=a; }
	void SetMask (uint r, uint g, uint b, uint a)
	{ mask[0]=r; mask[1]=g; mask[2]=b; mask[3]=a; }
	void SetLoss (uint r, uint g, uint b, uint a)
	{ loss[0]=r; loss[1]=g; loss[2]=b; loss[3]=a; }
};

class Image : public Referenced
{
public:
	Image (const char *file);
	Image ();
	Image (int _w, int _h, const ImgFormat& fmt);
	~Image ();

	void Alloc(int _w, int _h,const ImgFormat& fmt);

	void Free ();	// free image data
	void Load(const char *file, bool IsGrayscale=false);

	bool Save(const char *file);
	void SaveTGA(const char *file);

	void LoadFromMemory (void *data, int len);
	void LoadGrayscale (void *data, int len);
	void FromIL(uint id);
	uint ToIL();
	void DeleteIL(uint id);

	Image* Clone();
	void FillAlpha ();
	void Convert (Image *dst); // copy this to dst in dst.format
	void Blit (Image *dst, int sx, int sy, int dx, int dy, int w, int h);

	// Slow fallback versions, converting to and from the format
	void SetPixel (int x,int y, uint rgba);
	uint GetPixel (int x,int y); // return in RGBA format

	// create a scaled version of this image in dst (2x as small)
	// returns false if this is the smallest mipmap possible
	// format can be 16 bit (565) or 32 bit (8888)
	// the image must have proper dimensions (like 256x128 or 64x64)
	bool GenMipmap (Image *dst); 
	
	/* ------------- Inlines ------------- */
	inline int MemoryUse () 
	{ 
		return w * h * format.bytesPerPixel + sizeof(Image); 
	}

	inline uint GetPixel32 (int x,int y)
	{
		return *(uint*)&data[ (y * w + x) * 4 ];
	}

	inline void SetPixel32 (int x,int y, uint col)
	{
		*(uint*)&data[(y*w+x) * 4 ] = col;
	}

	inline ushort GetPixel16 (int x,int y)
	{
		return *(ushort*)&data[ (y * w + x) * 2 ];
	}

	inline void SetPixel16 (int x,int y, ushort col)
	{
		*(ushort*)&data[(y*w+x) * 2] = col;
	}

	inline uchar GetPixel8 (int x,int y)
	{
		return data[ y * w + x ];
	}

	inline void SetPixel8 (int x,int y, uchar col)
	{
		data[y * w + x] = col;
	}

//-------------- Vars ----------------
	uchar *data;
	ImgFormat format;
	int w,h;
};

#endif
