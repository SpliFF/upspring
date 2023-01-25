//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef C_MATHLIB_H
#define C_MATHLIB_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926536f
#endif

#ifndef ABS
#define ABS(a) ( ((a) > 0) ? (a) : (-(a)) ) 
#endif

#define EPSILON (0.01f)

class Vector4;
class Plane;
class Matrix;
class Quaternion;

class Vector3
{
public:
	Vector3() { x=y=z=0; }
	Vector3(float tx,float ty,float tz) { x=tx;y=ty;z=tz; }

	// a few non-operator functions for working with pointers efficiently
	void set(float tx,float ty, float tz) {x=tx;y=ty;z=tz;}
	void set(float *p) { x=p[0];y=p[1];z=p[2]; }
	void sub(const Vector3 *t) { x-=t->x;y-=t->y;z-=t->z; }
	void mul(float m) { x*=m;y*=m;z*=m; }
	void add(const Vector3 *t) { x+=t->x;y+=t->y;z+=t->z; }
	float dot(const Vector3 *t) { return t->x * x + t->y * y + t->z * z; }
	float dot(const Vector3& t) { return t.x * x + t.y * y + t.z * z; }
	Vector3 operator+(const Vector3& v) const { return Vector3 (x+v.x,y+v.y,z+v.z); }
	Vector3 operator-(const Vector3& v) const { return Vector3 (x-v.x,y-v.y,z-v.z); }
	Vector3 operator*(const float a) const { return Vector3(x*a,y*a,z*a); }
	Vector3 operator*(const Vector3& v) const { return Vector3(x*v.x,y*v.y,z*v.z); }
	bool operator==(const Vector3& v) const;
	Vector3 operator-()const  { return Vector3(0,0,0) - *this; }
	Vector3 operator/(const float a) const { return Vector3(x/a,y/a,z/a); }
#ifndef SWIG
	bool operator!=(const Vector3& v) const { return !(operator==(v)); }
	Vector3 &operator+=(const Vector3& v) { x+=v.x;y+=v.y;z+=v.z; return*this; }
	Vector3 &operator-=(const Vector3& v) { x-=v.x;y-=v.y;z-=v.z; return*this; }
	Vector3 &operator*=(const float a) { x*=a;y*=a;z*=a; return*this; }
	Vector3 &operator*=(const Vector3& v) { x*=v.x;y*=v.y;z*=v.z;return *this;}
	Vector3 &operator/=(const float a) { x/=a;y/=a;z/=a; return *this; }
	Vector3 &operator/=(const Vector3& v) { x/=v.x;y/=v.y;z/=v.z; return *this;}
	float operator|(const Vector3& v) const { return v.x*x + v.y*y + v.z*z; } // dotproduct
	Vector3 operator^(const Vector3 &v)const {  return crossproduct (v); } // crossproduct
	float& operator[](unsigned i) { return v[i]; }
	const float& operator[](unsigned i) const { return v[i]; }
#endif
	Vector3 crossproduct(const Vector3& v) const;
	float length() const { return (float)sqrt(x*x+y*y+z*z); }
	// project this vector on b
	Vector3 project (const Vector3&b ) const { return b*((*this | b) / (b|b)); }
	float projectf (const Vector3&b ) const { return (*this | b) / (b|b); }
	void normalize();
	float distance(const Vector3 *c1, const Vector3 *c2) const;
	void get_normal(const Vector3 *v1, const Vector3 *v2, const Vector3 *v3);
	float *getf () { return (float*)this; }
	void incboundingmin (const Vector3 *check);
	void incboundingmax (const Vector3 *check);
	bool epsilon_compare (const Vector3 *v, float epsilon) const ;

	// copy to float[3]
	void copy(float *t) const { t[0] = x; t[1] = y; t[2] = z; }
	void copy(Vector3 *t) const { t->x = x; t->y = y; t->z = z; }

#ifdef SWIG
	float x,y,z;
#else
	union {
		struct {float x,y,z;};
		float v[3];
	};
#endif
};


class Vector2 {
public:
	Vector2(float X,float Y):x(X),y(Y) {}
	Vector2() {x=y=0.0f;}
	float x,y;
};

class Vector4
{
public:
	Vector4(float tx=0, float ty=0, float tz=0, float tw=0)
	{   x=tx; y=ty; z=tz; w=tw; }

	float x,y,z,w;
};

class Plane
{
public:
	float a,b,c,d;
	Plane() { a=b=c=d=0; }
	Plane(float ta,float tb,float tc,float td) { a=ta;b=tb;c=tc;d=td; }
	Plane(Vector3 dir, float dis) { a=dir.x; b=dir.y;c=dir.z; d=dis; }
	float Dis(const Vector3 *v) const { return a*v->x+b*v->y+c*v->z-d; }
	float Dis(float x,float y,float z) const {  return a*x+b*y+z*c-d; }
	bool EpsilonCompare (const Plane& pln, float epsilon);
#ifndef SWIG
	bool operator==(const Plane &pln);
	bool operator!=(const Plane &pln) { return !operator==(pln); }
#endif
	void MakePlane(const Vector3& v1, const Vector3& v2,const Vector3& v3);
	void Inverse() {a=-a; b=-b; c=-c; d=-d;} // Plane is the same, but is pointing to the inverse direction
	void SetVec(Vector3 v) { a=v.x;b=v.y;c=v.z; }
	void CalcDis(const Vector3& p) { d = a*p.x + b*p.y + c*p.z; }
	void copy (Plane *pl) { pl->a=a; pl->b=b; pl->c=c; pl->d=d; }
	Vector3& GetVector() { return *(Vector3*)this; }
	Vector3 GetCenter() { return Vector3 (a*d,b*d,c*d); }
};

class Matrix
{
public:
	Matrix () { clear(); } 
	void vector_rotation (const Vector3 &v, float angle);
	void translation(float tx, float ty, float tz);
	void translation(const Vector3& t);
	void addtranslation(const Vector3& t);
	void addscale(const Vector3& t) { addscale(t.x,t.y,t.z); }
	void addscale(float x,float y,float z);
	void scale(float sx,float sy, float sz);
	void scale(const Vector3& s);
	void camera(Vector3 *p, Vector3 *right, Vector3 *up, Vector3 *front);
	void transpose(Matrix *dest) const;
	void apply(const Vector3 *v, Vector3 *o) const;
	void apply(const Vector4 *v, Vector4 *o) const;
	void multiply(const Matrix& lastOperation, Matrix& dst) const;
	Vector3 camera_pos() const;
	Matrix& operator*=(const Matrix& lastop) { multiply (lastop, *this); return *this; }
	Matrix operator*(const Matrix& o) { Matrix r; multiply (o, r); return r; }
	Matrix operator*(float f) { Matrix r=*this; r.multiply(f); return r; }
	Matrix& operator*=(float f) { multiply(f); return *this; }
	void align(Vector3 *right, Vector3 *up, Vector3 *front);
	void xrotate(float r);
	void yrotate(float r);
	void zrotate(float r);
	void eulerZXY(float x, float y, float z) {eulerZXY(Vector3(x,y,z)); }
	void eulerZXY(const Vector3& rot); // euler->matrix. rotation applied in ZXY order (=TA style)
	void eulerYXZ(float x,float y, float z) { eulerYXZ(Vector3(x,y,z)); }
	void eulerYXZ(const Vector3& rot);
	Vector3 calcEulerZXY() const; // matrix->euler.ZXY
	Vector3 calcEulerYXZ() const; // matrix->euler.YXZ
	void makequat (Quaternion *q) const;
	void identity ();
	void clear ();
//	bool affine_inverse(Matrix *out) const;
//	bool inverse (Matrix *out) const;
	bool inverse(Matrix& out) const;
	float determinant()  const;
	Matrix adjoint() const;
	void copy (float *d) { for(int a=0;a<16;a++) d[a]=m[a]; }
	void copy (Matrix *d) { for(int a=0;a<16;a++) d->m[a]=m[a]; }
	void perspective_lh (float fovY, float aspect, float zn, float zf);
	void multiply (float a);

	// like Allegro's v[][] and t[] array
	float &v(int row, int col) { return m[row*4+col]; }
	float &t(int y) { return m[3+4*y]; }
	const float &v(int row, int col) const { return m[row*4+col]; }
	const float &t(int y) const { return m[3+4*y]; }

	float &operator() (size_t i) { return m[i]; }
	const float &operator() (size_t i) const { return m[i]; }
	float &operator[] (size_t i) { return m[i]; }
	const float &operator[] (size_t i) const { return m[i]; }

	// direct row access
	Vector3 *getx () { return (Vector3*)&m[0]; }
	Vector3 *gety () { return (Vector3*)&m[4]; }
	Vector3 *getz () { return (Vector3*)&m[8]; }
	Vector3 *getw () { return (Vector3*)&m[12]; }
	const Vector3 *getx () const { return (Vector3*)&m[0]; }
	const Vector3 *gety () const { return (Vector3*)&m[4]; }
	const Vector3 *getz () const { return (Vector3*)&m[8]; }
	const Vector3 *getw () const { return (Vector3*)&m[12]; }

	// column access
	void getcx (Vector3& v) const { v.x = m[0]; v.y = m[4]; v.z = m[8]; }
	void getcy (Vector3& v) const { v.x = m[1]; v.y = m[5]; v.z = m[9]; }
	void getcz (Vector3& v) const { v.x = m[2]; v.y = m[6]; v.z = m[10]; }
	void getcw (Vector3& v) const { v.x = m[3]; v.y = m[7]; v.z = m[11]; }
	void setcx (const Vector3& v) { m[0] = v.x; m[4] = v.y; m[8] = v.z; }
	void setcy (const Vector3& v) { m[1] = v.x; m[5] = v.y; m[9] = v.z; }
	void setcz (const Vector3& v) { m[2] = v.x; m[6] = v.y; m[10] = v.z; }
	void setcw (const Vector3& v) { m[3] = v.x; m[7] = v.y; m[11] = v.z; }

#ifndef SWIG
	float& operator[](int i) { return m[i]; }
#endif

	/*
	matrix layout:

	x       y       z       w
x   0       1       2       3
y   4       5       6       7
z   8       9       10		11
w   12		13		14		15
	*/
	float m[16];
};


class Quaternion
{
public:
	Quaternion () { identity (); }
	Quaternion (float inx,float iny, float inz, float inw) 
	{   x=inx; y=iny; z=inz; w=inw; }

	enum SlerpType {
		qshort, qlong, qcw, qccw, quser
	};

	void identity () {x=y=z=0.0f; w=1.0f; }
	void copy(Quaternion *p) const { p->x=x;p->y=y;p->z=z; p->w=w; }
	float normal () const { return ((w*w) + (x*x) + (y*y) + (z*z)); }
	void conjugate () { x=-x; y=-y; z=-z; }
	void conjugate (Quaternion *dest) const { dest->x = -x; dest->y = y; dest->z = -z; dest->w = w; }

	void normalize ();
	Quaternion& operator*=(const Quaternion& q) { mul(&q, this); return *this; }
	Quaternion operator*(const Quaternion& q) { Quaternion r; mul(&q,&r); return r; }
	void mul(const Quaternion *q, Quaternion *out) const;
	void xrotate(float a);
	void yrotate(float a);
	void zrotate(float a);
	void rotation(float x,float y, float z);
	void vector_rot(Vector3 *ax, float r);
	void slerp (const Quaternion *to, float t, Quaternion *out, SlerpType how) const;
	void apply(float x, float y, float z, float *xout, float *yout, float *zout) const;
	void apply(Vector3 *in, Vector3 *out) const;
	void makematrix (Matrix *m) const;
	void inverse(Quaternion *dest) const;

	float x,y,z,w;
};

namespace Math
{
	// compare vectors using a large epsilon, needed for collision detection
	bool SamePoint(const Vector3 *p1, const Vector3 *p2, float epsilon);
	void NearestBoxVertex(const Vector3 *min, const Vector3 *max, const Vector3 *pos, Vector3 *out);
	void ComputeOrientation (float yaw, float pitch, float roll, 
														 Vector3 *right=0, Vector3 *up=0, Vector3 *front=0);
	void NearestBoxPoint(const Vector3 *min, const Vector3 *max, const Vector3 *pos, Vector3 *out);
//	Matrix CreateNormalTransform (const Matrix& transform);
	Matrix GetTransform (const Vector3& offset, const Vector3& rotation, const Vector3& scale);
};

#endif
