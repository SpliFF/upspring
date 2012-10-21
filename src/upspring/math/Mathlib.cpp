//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#include <assert.h>
#include <stdlib.h>
#include "Mathlib.h"
#include "Util.h"

//-----------------------------------------------------------------------
// Register metadata
//-----------------------------------------------------------------------
CR_BIND(Vector2,());
CR_REG_METADATA(Vector2, (CR_MEMBER(x), CR_MEMBER(y)));

CR_BIND(Vector3,());
CR_REG_METADATA(Vector3, (CR_MEMBER(x), CR_MEMBER(y), CR_MEMBER(z)));

CR_BIND(Vector4,());
CR_REG_METADATA(Vector4, (CR_MEMBER(x), CR_MEMBER(y), CR_MEMBER(z), CR_MEMBER(w)));

CR_BIND(Plane,());
CR_REG_METADATA(Plane, (CR_MEMBER(a), CR_MEMBER(b), CR_MEMBER(c), CR_MEMBER(d)));

CR_BIND(Matrix,());
CR_REG_METADATA(Matrix, (CR_MEMBER(m)));

CR_BIND(Quaternion,());
CR_REG_METADATA(Quaternion, (CR_MEMBER(x), CR_MEMBER(y), CR_MEMBER(z), CR_MEMBER(w)));
//-----------------------------------------------------------------------

#define USE_RADIANS

#ifdef USE_RADIANS
#define mcos(_x) cos(_x)
#define msin(_x) sin(_x)
#else
#define mcos(_x) cos(pi_div180*(_x))
#define msin(_x) sin(pi_div180*(_x))
#endif

const float pi_div180 = 0.01745329252f;

void Vector3::normalize()
{
	float m = (float)1.0f / sqrt(x*x+y*y+z*z);
	x *= m;
	y *= m;
	z *= m;
}

bool Vector3::operator==(const Vector3& v)const 
{
	if(x<v.x+EPSILON && x>v.x-EPSILON &&
		y<v.y+EPSILON && y>v.y-EPSILON &&
		z<v.z+EPSILON && z>v.z-EPSILON)
	{
		return true;
	}
	return false;
}

Vector3 Vector3::crossproduct (const Vector3 &v)const 
{
	Vector3 r;
	r.x = (y*v.z) - (z*v.y);
	r.y = (z*v.x) - (x*v.z);
	r.z = (x*v.y) - (y*v.x);
	return r;
}

float Vector3::distance(const Vector3 *c1, const Vector3 *c2) const
{
	Vector3 m, diff;
	diff.set(x - c1->x, y - c1->y, z - c1->z);
	m.set(c2->x - c1->x, c2->y - c1->y, c2->z - c1->z);
	float t=m.x * diff.x + m.y * diff.y + m.z * diff.z;
	t /= (m.x * m.x);
	diff.set (diff.x - t*m.x, diff.y - t*m.y, diff.z - t*m.z);
	return diff.length();
}

void Vector3::get_normal(const Vector3 *v1, const Vector3 *v2, const Vector3 *v3)
{
	float rx1=v2->x-v1->x,ry1=v2->y-v1->y,rz1=v2->z-v1->z;
	float rx2=v3->x-v1->x,ry2=v3->y-v1->y,rz2=v3->z-v1->z;
  x=ry1*rz2-ry2*rz1;
  y=rz1*rx2-rz2*rx1;
  z=rx1*ry2-rx2*ry1;
	normalize();
}

void Vector3::incboundingmax (const Vector3 *check)
{
	if(check->x > x) x = check->x;
	if(check->y > y) y = check->y;
	if(check->z > z) z = check->z;
}

void Vector3::incboundingmin (const Vector3 *check)
{
	if(check->x < x) x = check->x;
	if(check->y < y) y = check->y;
	if(check->z < z) z = check->z;
}

bool Vector3::epsilon_compare (const Vector3 *v, float epsilon) const
{
	float d = fabs(v->x - x);
	if(d > epsilon) return false;
	d= fabs(v->y - y);
	if(d > epsilon) return false;
	d = fabs(v->z - z);
	if(d > epsilon) return false;
	return true;
}

//--------------------------------------------------------- Plane

void Plane::MakePlane(const Vector3& v1,const Vector3& v2,const Vector3& v3)
{
	float rx1=v2.x-v1.x,ry1=v2.y-v1.y,rz1=v2.z-v1.z;
	float rx2=v3.x-v1.x,ry2=v3.y-v1.y,rz2=v3.z-v1.z;
	a=ry1*rz2-ry2*rz1;
	b=rz1*rx2-rz2*rx1;
	c=rx1*ry2-rx2*ry1;
	float len = (float)sqrt(a*a+b*b+c*c);
	a /= len;
	b /= len;
	c /= len;
	d=a*v2.x+b*v2.y+c*v2.z;
}

bool Plane::operator==(const Plane &pln)
{
	if((pln.a < a + EPSILON) && (pln.a > a - EPSILON) &&
		(pln.b < b + EPSILON) && (pln.b > b - EPSILON) &&
		(pln.c < c + EPSILON) && (pln.c > c - EPSILON) &&
		(pln.d < d + EPSILON) && (pln.d > d - EPSILON))  return true;
	return false;
}

bool Plane::EpsilonCompare (const Plane& pln, float epsilon)
{
	Plane t;
	t.a = fabs(a-pln.a);
	t.b = fabs(b-pln.b);
	t.c = fabs(c-pln.c);
	t.d = fabs(d-pln.d);
	if(t.a > epsilon || t.b > epsilon || t.c > epsilon || t.d > epsilon)
		return false;
	return true;
}

//--------------------- Matrix class -----------------------

/*
	matrix layout:

	x       y       z       w
x   0       1       2       3
y   4       5       6       7
z   8       9       10  11
w   12  13  14  15
*/

void Matrix::clear ()
{
	for (int a=0;a<16;a++)
		m[a] = 0.0f;
}

void Matrix::identity ()
{
	for (int a=0;a<16;a++)
		m[a] = 0.0f;
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
}

void Matrix::zrotate (float t)
{
	float st=msin(t), ct=mcos(t);
	clear ();
	/*
		 | cos t -sin t  0 |
	Mr = | sin t  cos t  0 |
		 | 0      0      0 |
	*/
	m[10] = m[15] = 1.0f;
	m[0] = ct;
	m[1] = -st;
	m[4] = st;
	m[5] = ct;
}

void Matrix::yrotate (float t)
{
	float st=msin(t), ct=mcos(t);
	clear ();
	/*
		 | cos t  0   -sin t |
	Mr = | 0      1     0    |
		 | sin t  0    cos t |
	*/
	m[0] = ct;
	m[2] = -st;
	m[5] = 1.0f;
	m[8] = st;
	m[10] = ct;
	m[15] = 1.0f;
}

void Matrix::xrotate (float t)
{
	float st = msin(t), ct = mcos(t);
	clear ();
	/*
		 | 1     0      0   |
	Mr = | 0   cos t -sin t |
		 | 0   sin t  cos t |
	*/
	m[0] = 1.0f;
	m[5] = ct;
	m[6] = -st;
	m[9] = st;
	m[10] = ct;
	m[15] = 1.0f;
}


// Calculate a rotation matrix from euler angles. Rotation is applied in ZXY order
void Matrix::eulerZXY(const Vector3& rot)
{
	float cy = cosf(rot.y), sy = sinf(rot.y);
	float cx = cosf(rot.x), sx = sinf(rot.x);
	float cz = cosf(rot.z), sz = sinf(rot.z);

/*	[cz cy+sz sx sy,    -sz cx,     -cz sy+sz sx cy]
	[sz cy-cz sx sy,     cz cx,     -sz sy-cz sx cy]
	[cx sy,                sx,               cx cy]*/
	clear();
	m[0] = cy*cz+sz*sx*sy;
	m[1] = -sz*cx;
	m[2] = -sy*cz+sz*sx*cy;
	m[4] = cy*sz-cz*sx*sy;
	m[5] = cx*cz;
	m[6] = -sy*sz-sx*cz*cy;
	m[8] = sy*cx;
	m[9] = sx;
	m[10] = cx*cy;
	m[15] = 1.0f;
}

// Calculate a rotation matrix from euler angles. Rotation is applied in YXZ order
void Matrix::eulerYXZ(const Vector3& rot)
{
	float cy = cosf(rot.y), sy = sinf(rot.y);
	float cx = cosf(rot.x), sx = sinf(rot.x);
	float cz = cosf(rot.z), sz = sinf(rot.z);

	/*
	[cz cy-sz sx sy,   -sz cy-cz sx sy,    -cx sy]
	[sz cx,            cz cx,                 -sx]
	[cz sy+sz sx cy,   -sz sy+cz sx cy,    cx cy]]
	*/

	clear();
	m[0] = cz*cy-sz*sx*sy;
	m[1] = -sz*cy-cz*sx*sy;
	m[2] = -cx*sy;
	m[4] = sz*cx;
	m[5] = cz*cx;
	m[6] = -sx;
	m[8] = cz*sy+sz*sx*cy;
	m[9] = -sz*sy+cz*sx*cy;
	m[10] = cx*cy;
	m[15]=1.0f;
}

// Assuming m is a pure rotation matrix
Vector3 Matrix::calcEulerZXY() const
{
	Vector3 r;

	// If fabs(sx) = 1, then cx is zero and the Y and Z angle (heading and bank) can't be calculated
	if (m[9] > 0.998f) {
		/*
		| cy*cz+sy*sz   0   -(sy*cz-cy*sz) |     | cos(Y-Z)   0  -cos(Y-Z) |
		|-(sy*cz-cy*sz) 0   -(cy*cz+sy*sz) |  =  | -sin(Y-Z)  0  cos(Y-Z)  |
		|   0           1          0       |     |    0       1     0      |

		sin(Y+Z) = sy*cz + cy*sz
		cos(Y+Z) = cy*cz - sy*sz
		sin(Y-Z) = sy*cz - cy*sz
		cos(Y-Z) = cy*cz + sy*sz

		*/
		r.y = atan2f(-m[4], m[0]);  // it can be represented with both Y rotation and Z rotation. Y is used here
		r.z = 0.0f;
		r.x = M_PI*0.5f;
	} else if (m[9] < -0.998f) {
	/*
	| cy*cz-sy*sz      0      -sy*cz-sz*cy  |     | cos(Y+Z)  ...
	| sy*cz+cy*sz      0      -sy*sz+cz*cy  |  =  | sin(Y+Z) ..
	|        0         -1            0      |     | 
	*/
		r.y = atan2f(m[4], m[0]);
		r.z = 0.0f;
		r.x = -M_PI*0.5f;
	} else {
		r.x = asinf(m[9]);
		r.y = atan2f(m[8], m[10]);
		r.z = atan2f(-m[1], m[5]);
	}

	return r;
}



// Assuming m is a pure rotation matrix
Vector3 Matrix::calcEulerYXZ() const
{
	Vector3 r;
	/*
	[cz cy-sz sx sy,   -sz cy-cz sx sy,    -cx sy]  0 1 2
	[sz cx,            cz cx,                 -sx]  4 5 6
	[cz sy+sz sx cy,   -sz sy+cz sx cy,    cx cy]]  8 9 10
	*/

	if (m[6] > 0.998f) {
		 // sx = -1, cx=0
	/*
	[cy*cz+sy*sz,   -cy*sz+sy*cz,   0]  0 1 2
	[0,              0,             1]  4 5 6
	[sy*cz-cy*sz,   -(cy*cz+sy*sz), 0]  8 9 10
         =
	[cos(y-z),   sin(y-z),   0]  0 1 2
	[0,              0,      1]  4 5 6
	[sin(y-z),   -cos(y-z),  0]  8 9 10
	*/

		/*
		sin(Y+Z) = sy*cz + cy*sz
		cos(Y+Z) = cy*cz - sy*sz
		sin(Y-Z) = sy*cz - cy*sz
		cos(Y-Z) = cy*cz + sy*sz
		*/
		r.y = atan2f(m[1], m[0]);  // it can be represented with both Y rotation and Z rotation. Y is used here
		r.z = 0.0f;
		r.x = -M_PI*0.5f;
	} else if (m[9] < -0.998f) {
		// sx = 1, cx = 0
	/*
	[cz cy-sz sy,   -sz cy-cz sy,    0]  0 1 2
	[0,               0,            -1]  4 5 6
	[cz sy+sz cy,   -sz sy+cz cy,    0]  8 9 10
	    =
	[cos(y+z),      -sin(y+z),  0]  0 1 2
	[0,               0,            -1]  4 5 6
	[sin(y+z),      cos(y+z),    0]  8 9 10
	*/
		r.y = atan2f(m[8], m[0]);
		r.z = 0.0f;
		r.x = M_PI*0.5f;
	} else {
		r.y = atan2f(-m[2],m[10]);
		r.z = atan2f(m[4], m[5]);
		r.x = asinf(-m[6]);
	}

	return r;
}


void Matrix::multiply (const Matrix& t, Matrix& dst) const
{
	Matrix temp;
	const Matrix *a = &t, *b = this;

	if (a == &dst)
	{
		temp = *a;
		a = &temp;
	}
	if (b == &dst)
	{
		temp = *b;
		b = &temp;
	}

	for (int row=0;row<4;row++)
		for (int col=0;col<4;col++)
		{
			dst.v(row, col) = a->v(row, 0) * b->v(0,col) + a->v(row,1) * b->v(1,col) + 
								a->v(row,2) * b->v(2,col) + a->v(row,3) * b->v(3,col);
		}
}
/*
static inline float Calc3x3Determinant (float m[9])
{
	//0 1 2
	//3 4 5
	//6 7 8
	
	return m [0] * (m[4]*m[8] - m[5]*m[7]) - m[1] * (m[3]*m[8]-m[5]*m[6]) + m[2] * (m[3]*m[7]-m[6]*m[4]);
}

static inline float CalcSubmatrixDeterminant (const Matrix &m, int sc)
{
	float s[9];
	for (int y=1;y<4;y++)
		for (int x=0;x<4;x++)
		{
			if (x == sc)
				continue;

			int c=(x < sc) ? x : x-1;

			s [c+(y-1)*3] = m.m[x+y*4];
		}

	return Calc3x3Determinant (s);
}

static inline float CalcSubmatrixDeterminant (const Matrix &m, int sc, int sr)
{
	float s[9];

	for (int y=0;y<4;y++)
	{
		if (y == sr)
			continue;
		int r=(y<sr) ? y : y-1;

		for (int x=0;x<4;x++)
		{
			if (x == sc)
				continue;

			int c=(x < sc) ? x : x-1;

			s [c+r*3] = m.m[x+y*4];
		}
	}

	return Calc3x3Determinant (s);
}

float Matrix::determinant () const
{
	int x;
	float sign = 1.0f, det=0.0f;

	for (x=0;x<4;x++)
	{
		det += sign * CalcSubmatrixDeterminant (*this, x);
		sign *= -1;
	}
	return det;
}
*/


//-----------------------------------------------------------------------
inline static float MINOR(const Matrix& m, const size_t r0, const size_t r1, const size_t r2, 
							const size_t c0, const size_t c1, const size_t c2)
{
    return m[r0*4+c0] * (m[r1*4+c1] * m[r2*4+c2] - m[r2*4+c1] * m[r1*4+c2]) -
        m[r0*4+c1] * (m[r1*4+c0] * m[r2*4+c2] - m[r2*4+c0] * m[r1*4+c2]) +
        m[r0*4+c2] * (m[r1*4+c0] * m[r2*4+c1] - m[r2*4+c0] * m[r1*4+c1]);
}

Matrix Matrix::adjoint() const
{
    Matrix r;
	
	r[0] = MINOR(*this, 1, 2, 3, 1, 2, 3);
    r[1] = -MINOR(*this, 0, 2, 3, 1, 2, 3);
	r[2] = MINOR(*this, 0, 1, 3, 1, 2, 3);
	r[3] = -MINOR(*this, 0, 1, 2, 1, 2, 3);

	r[4] = -MINOR(*this, 1, 2, 3, 0, 2, 3);
	r[5] = MINOR(*this, 0, 2, 3, 0, 2, 3);
	r[6] = -MINOR(*this, 0, 1, 3, 0, 2, 3);
	r[7] = MINOR(*this, 0, 1, 2, 0, 2, 3);

	r[8] = MINOR(*this, 1, 2, 3, 0, 1, 3);
	r[9] = -MINOR(*this, 0, 2, 3, 0, 1, 3);
	r[10] = MINOR(*this, 0, 1, 3, 0, 1, 3);
	r[11] = -MINOR(*this, 0, 1, 2, 0, 1, 3);

	r[12] = -MINOR(*this, 1, 2, 3, 0, 1, 2);
	r[13] = MINOR(*this, 0, 2, 3, 0, 1, 2);
	r[14] = -MINOR(*this, 0, 1, 3, 0, 1, 2);
	r[15] = MINOR(*this, 0, 1, 2, 0, 1, 2);

	return r;
}

float Matrix::determinant() const
{
    return m[0*4+0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
        m[0*4+1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
        m[0*4+2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
        m[0*4+3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
}

bool Matrix::inverse(Matrix& out) const
{
	float det = determinant();
	if(fabsf(det) < EPSILON)
		return false;
	
	out = adjoint();
	out *= 1.0f / det;
	return true;
}
/*
bool Matrix::inverse (Matrix *inv) const
{
	float det = determinant ();

	if (det == 0.0f)
		return false;

	float inv_det = 1.0f / det;
	int x,y;

	float *d = inv->m;
	for (x=0;x<4;x++)
		for (y=0;y<4;y++)
			*(d++) = CalcSubmatrixDeterminant (*this, x, y) * inv_det;

	return true;
}

// Generate the inverse matrix for an affine transformation
bool Matrix::affine_inverse (Matrix *out) const
{
	float d=determinant ();
	if (d == 0.0f)
		return false;// d=0 means multiple solutions for Ax=b, so it has no inverse

	transpose (out);
	out->multiply (1.0f / d);
	return true;
}
*/
void Matrix::multiply(float a)
{
	for (int i=0;i<16;i++)
		m[i] *= a;
}

/*
float Matrix::determinant ()
{
	float det;

	{
		float d = 
			v(1,1) * (v(2,2)*v(3,3)-v(3,2)*v(2,3)) -
			v(1,2) * (v(2,3)*v(3,1)-v(3,3)*v(2,1)) +
			v(1,3) * (v(2,1)*v(3,2)-v(3,1)*v(2,2));
		det = d * v(0,0);
	}
	{
		float d =
			v(1,0) * (v(2,2)*v(3,3)-v(3,2)*v(2,3)) +
			v(1,2) * (v(2,3)*v(3,0)-v(3,3)*v(2,0)) -
			v(1,3) * (v(2,0)*v(3,2)-v(3,0)*v(2,2));
		det -= d * v(0,1);
	}
	{
		float d =
			v(1,0) * (v(2,1)*v(3,3)-v(3,1)*v(2,3)) +
			v(1,1) * (v(2,0)*v(3,3)-v(3,0)*v(2,3)) -
			v(1,3) * (v(2,0)*v(3,1)-v(3,

	}
}
*/


void Matrix::perspective_lh(float fovY,float Aspect,float zn,float zf)
{
	float h = 1.0f / tan (fovY/2.0f);
	float w = h / Aspect;

	clear ();

	v(0,0) = w;
	v(1,1) = h;
	v(2,2) = zf / (zf-zn);
	v(3,2) = 1.0f;
	v(2,3) = -zn*zf/(zf-zn);
}

Vector3 Matrix::camera_pos () const
{
	/*
    [tx]     [-rx   -ry   -rz]    [px]
	[ty]  =  [-ux   -uy   -uz]  * [py]
	[tz]     [-fx   -fy   -fz]    [pz]
	*/

	Matrix inv;
return Vector3();

}

void Matrix::camera (Vector3 *p, Vector3 *right, Vector3 *up, Vector3 *front)
{
	right->copy (&m[0]);
	up->copy (&m[4]);
	front->copy (&m[8]);

	t(0) = -right->dot (p);
	t(1) = -up->dot (p);
	t(2) = -front->dot (p);
	m[12] = m[13] = m[14] = 0.0f;
	m[15] = 1.0f;
}

void Matrix::scale (const Vector3& s)
{
	clear ();
	m[0] = s.x;
	m[5] = s.y;
	m[10] = s.z;
	m[15] = 1.0f;
}

void Matrix::scale (float sx, float sy, float sz)
{
	clear ();
	m[0] = sx;
	m[5] = sy;
	m[10] = sz;
	m[15] = 1.0f;
}

void Matrix::addscale (float x,float y,float z)
{
	getx ()->mul (x);
	gety ()->mul (y);
	getz ()->mul (z);
}

void Matrix::translation (const Vector3& t)
{
	identity ();
	m[3] = t.x;
	m[7] = t.y;
	m[11] = t.z;
}

void Matrix::translation (float tx,float ty,float tz)
{
	identity ();
	m[3] = tx;
	m[7] = ty;
	m[11] = tz;
}

void Matrix::addtranslation (const Vector3& t)
{
	m[3] += t.x;
	m[7] += t.y;
	m[11] += t.z;
}

void Matrix::transpose (Matrix *dest) const
{
	int x,y;
	for(y=0;y<4;y++)
		for(x=0;x<4;x++)
			dest->m[x*4+y] = m[y*4+x];
}

void Matrix::apply (const Vector3 *v, Vector3 *o) const
{
	// w component is 1.0f
	o->x = m[0] * v->x + m[1] * v->y + m[2] * v->z + m[3];
	o->y = m[4] * v->x + m[5] * v->y + m[6] * v->z + m[7];
	o->z = m[8] * v->x + m[9] * v->y + m[10] * v->z + m[11];
}

void Matrix::apply (const Vector4 *v, Vector4 *o)const
{
	o->x = m[0] * v->x + m[1] * v->y + m[2] * v->z + m[3] * v->w;
	o->y = m[4] * v->x + m[5] * v->y + m[6] * v->z + m[7] * v->w;
	o->z = m[8] * v->x + m[9] * v->y + m[10] * v->z + m[11] * v->w;
	o->w = m[12] * v->x + m[13] * v->y + m[14] * v->z + m[15] * v->w;
}

void Matrix::align (Vector3 *front, Vector3 *up, Vector3 *right)
{
	clear ();
	front->copy (&m[0]);
	up->copy (&m[4]);
	right->copy (&m[8]);
	m[15] = 1.0f;
}

void Matrix::vector_rotation (const Vector3& cv, float angle)
{
	float c = mcos(angle);
	float s = msin(angle);
	float cc = 1 - c;

	clear ();

	Vector3 axis(cv);
	axis.normalize ();

	v(0,0) = (cc * axis.x * axis.x) + c;
	v(0,1) = (cc * axis.x * axis.y) + (axis.z * s);
	v(0,2) = (cc * axis.x * axis.z) - (axis.y * s);

	v(1,0) = (cc * axis.x * axis.y) - (axis.z * s);
	v(1,1) = (cc * axis.y * axis.y) + c;
	v(1,2) = (cc * axis.z * axis.y) + (axis.x * s);

	v(2,0) = (cc * axis.x * axis.z) + (axis.y * s);
	v(2,1) = (cc * axis.y * axis.z) - (axis.x * s);
	v(2,2) = (cc * axis.z * axis.z) + c;
}

// ---------------------------- math funcs -----------------------------------

namespace Math {

bool SamePoint(const Vector3 *v1,const Vector3 *v2,float e)
{
	Vector3 v = *v1;
	v.sub(v2);
	if(ABS(v.x) > e) return false;
	if(ABS(v.y) > e) return false;
	if(ABS(v.z) > e) return false;
	return true;
}

void NearestBoxVertex(const Vector3 *min, const Vector3 *max, const Vector3 *pos, Vector3 *out)
{
	Vector3 mid = (*max + *min) * 0.5f;
	if(pos->x < mid.x) out->x = min->x;
	else out->x = max->x;
	if(pos->y < mid.y) out->y = min->y;
	else out->y = max->y;
	if(pos->z < mid.z) out->z = min->z;
	else out->z = max->z;
}

// Calculates the exact nearest point, not just one of the box'es vertices
void NearestBoxPoint(const Vector3 *min, const Vector3 *max, const Vector3 *pos, Vector3 *out)
{
	/*Vector3 mid = (*max + *min) * 0.5f;*/
	if(pos->x < min->x) out->x = min->x;
	else if(pos->x > max->x) out->x = max->x;
	else out->x = pos->x;
	if(pos->y < min->y) out->y = min->y;
	else if(pos->y > max->y) out->y = max->y;
	else out->y = pos->y;
	if(pos->z < min->z) out->z = min->z;
	else if(pos->z > max->z) out->z = max->z;
	else out->z = pos->z;
}

void ComputeOrientation (float yaw, float pitch, float roll, 
						   Vector3 *right, Vector3 *up, Vector3 *front)
{
	Matrix cur, temp, result;

	// get x rotation matrix:
	cur.xrotate (pitch);
	cur.copy (&result);

	// get y rotation matrix:
	cur.yrotate (yaw);
	result.copy (&temp);
	cur.multiply (temp, result);

	// get z rotation matrix
	cur.zrotate (roll);
	result.copy (&temp);
	cur.multiply (temp, result);
 
	if(right) *right = *result.getx ();
	if(up) *up = *result.gety ();
	if(front) *front = *result.getz ();
}


void GetNormal(Vector3 *v1,Vector3 *v2,Vector3 *v3,Vector3 *out)
{ 
	Vector3 t1,t2;
	t1.x = v2->x - v1->x;
	t1.y = v2->y - v1->y;
	t1.z = v2->z - v1->z;
	t2.x = v3->x - v1->x;
	t2.y = v3->y - v1->y;
	t2.z = v3->z - v1->z;
	*out = t1.crossproduct (t2);
	out->normalize();
}



Matrix CreateNormalTransform (const Matrix& transform)
{
	// create a 3x3 transform matrix for the normals
	Matrix normalTransform;
	*normalTransform.getx () = *transform.getx ();
	*normalTransform.gety () = *transform.gety ();
	*normalTransform.getz () = *transform.getz ();
	normalTransform.m[15]=1.0f;
	float d=normalTransform.determinant ();
	float s=1.0f/d;
	normalTransform.addscale (s,s,s);
	return normalTransform;
}

Matrix GetTransform (const Vector3& offset, const Vector3& rotation, const Vector3& scale)
{
	Vector3 r,u,f;
	Matrix rotationMatrix;
	rotationMatrix.eulerZXY (rotation);
	Matrix scalingMatrix;
	scalingMatrix.scale(scale.x, scale.y, scale.z);
	
	Matrix final = rotationMatrix * scalingMatrix;
	final.v(3,3) = 1.0f;

	final.t(0) = offset.x;
	final.t(1) = offset.y;
	final.t(2) = offset.z;
	return final;
}

};

// ------------------------------ Quaternion funcs -------------------------

/* quat_mul:
 *  Multiplies two quaternions, storing the result in out. The resulting
 *  quaternion will have the same effect as the combination of p and q,
 *  ie. when applied to a point, (point * out) = ((point * p) * q). Any
 *  number of rotations can be concatenated in this way. Note that quaternion
 *  multiplication is not commutative, ie. quat_mul(p, q) != quat_mul(q, p).
 */
void Quaternion::mul (const Quaternion *q, Quaternion *out) const
{
	const Quaternion *p = this;
	Quaternion temp;

	/* qp = ww' - v.v' + vxv' + wv' + w'v */

	if (p == out) 
	{
		temp = *p;
		p = &temp;
	} 
	else if (q == out) 
	{
		temp = *q;
		q = &temp;
	}

	/* w" = ww' - xx' - yy' - zz' */
	out->w = (p->w * q->w) - 
		(p->x * q->x) -
		(p->y * q->y) -
		(p->z * q->z);

	/* x" = wx' + xw' + yz' - zy' */
	out->x = (p->w * q->x) +
		(p->x * q->w) +
		(p->y * q->z) -
		(p->z * q->y);

/* y" = wy' + yw' + zx' - xz' */
	out->y = (p->w * q->y) +
		(p->y * q->w) +
		(p->z * q->x) -
		(p->x * q->z);

	/* z" = wz' + zw' + xy' - yx' */
	out->z = (p->w * q->z) +
		(p->z * q->w) +
		(p->x * q->y) -
		(p->y * q->x);
}

/* xrotate:
 *  Construct X axis rotation quaternions, storing them in q. When applied to
 *  a point, these quaternions will rotate it about the X axis by the
 *  specified angle (given in binary, 256 degrees to a circle format).
 */
void Quaternion::xrotate (float r)
{
	w = mcos(r / 2);
	x = msin(r / 2);
	y = 0;
	z = 0;
}



/* yrotate:
 *  Construct Y axis rotation quaternions, storing them in q. When applied to
 *  a point, these quaternions will rotate it about the Y axis by the
 *  specified angle (given in binary, 256 degrees to a circle format).
 */
void Quaternion::yrotate (float r)
{
	w = mcos(r / 2);
	x = 0;
	y = msin(r / 2);
	z = 0;
}

/* zrotate:
 *  Construct Z axis rotation quaternions, storing them in q. When applied to
 *  a point, these quaternions will rotate it about the Z axis by the
 *  specified angle (given in binary, 256 degrees to a circle format).
 */
void Quaternion::zrotate (float r)
{
	w = mcos(r / 2);
	x = 0;
	y = 0;
	z = msin(r / 2);
}



/* rotation:
 *  Constructs a quaternion which will rotate points around all three axis by
 *  the specified amounts (given in binary, 256 degrees to a circle format).
 */
void Quaternion::rotation (float x, float y, float z)
{
	float sx, sy, sz;
	float cx, cy, cz;
	float cycz, sysz;

	sx = msin(x / 2);
	sy = msin(y / 2);
	sz = msin(z / 2);
	cx = mcos(x / 2);
	cy = mcos(y / 2);
	cz = mcos(z / 2);

	sysz = sy * sz;
	cycz = cy * cz;

	w = (cx * cycz) + (sx * sysz);
	x = (sx * cycz) - (cx * sysz);
	y = (cx * sy * cz) + (sx * cy * sz);
	z = (cx * cy * sz) - (sx * sy * cz);
}



/* get_vector_rotation_quat:
 *  Constructs a quaternion which will rotate points around the specified
 *  x,y,z vector by the specified angle (given in binary, 256 degrees to a
 *  circle format).
 */
void Quaternion::vector_rot(Vector3 *ax, float r)
{
   float s;

	 ax->normalize ();

   w = mcos(r / 2);
   s = msin(r / 2);
   x = s * ax->x;
   y = s * ax->y;
   z = s * ax->z;
}

/* quat_to_matrix:
 * Constructs a rotation matrix from a quaternion.
 */
void Quaternion::makematrix(Matrix *m) const
{
	float ww;
	float xx;
	float yy;
	float zz;
	float wx;
	float wy;
	float wz;
	float xy;
	float xz;
	float yz;

	/* This is the layout for converting the values in a quaternion to a
	* matrix.
	*
	*  | ww + xx - yy - zz       2xy + 2wz             2xz - 2wy     |
	*  |     2xy - 2wz       ww - xx + yy - zz         2yz - 2wx     |
	*  |     2xz + 2wy           2yz - 2wx         ww + xx - yy - zz |
	*/

	ww = w * w;
	xx = x * x;
	yy = y * y;
	zz = z * z;
	wx = w * x * 2;
	wy = w * y * 2;
	wz = w * z * 2;
	xy = x * y * 2;
	xz = x * z * 2;
	yz = y * z * 2;

	m->v(0,0)  = ww + xx - yy - zz;
	m->v(1,0)  = xy - wz;
	m->v(2,0) = xz + wy;

	m->v(0,1) = xy + wz;
	m->v(1,1) = ww - xx + yy - zz;
	m->v(2,1) = yz - wx;

	m->v(0,2) = xz - wy;
	m->v(1,2) = yz + wx;
	m->v(2,2) = ww - xx - yy + zz;

	m->t(0) = 0.0;
	m->t(1) = 0.0;
	m->t(2) = 0.0;
	m->v(3,3) = 1.0f;
}

/* from_matrix:
 *  Constructs a quaternion from a rotation matrix. Translation is discarded
 *  during the conversion. Use get_align_matrix_f if the matrix is not
 *  orthonormalized, because strange things may happen otherwise.
 */
void Matrix::makequat (Quaternion *q) const
{
	float diag, s;
	int i,j,k;
	float out[4];

	static int next[3] = { 1, 2, 0 };

	diag = v(0,0) + v(1,1) + v(2,2);

	if (diag > 0.0f) 
	{
		s    = (float)(sqrt(diag + 1.0));
		q->w = s / 2.0f;
		s    = 0.5f / s;
		q->x = (v(1,2) - v(2,1)) * s;
		q->y = (v(2,0) - v(0,2)) * s;
		q->z = (v(0,1) - v(1,0)) * s;
	}
	else 
	{
		i = 0;

		if(v(1,1) > v(0,0)) i = 1;

		if(v(2,2) > v(i,i)) i = 2;

		j = next[i];
		k = next[j];

		s = v(i,i) - (v(j,j) + v(k,k));

		/* 
		* NOTE: Passing non-orthonormalized matrices can result in odd things
		*       happening, like the calculation of s below trying to find the
		*       square-root of a negative number, which is imaginary.  Some
		*       implementations of sqrt will crash, while others return this
		*       as not-a-number (NaN). NaN could be very subtle because it will
		*       not throw an exception on Intel processors.
		*/
		assert(s > 0.0);

		s = (float)(sqrt(s) + 1.0);

		out[i] = s / 2.0f;
		s      = 0.5f / s;
		out[j] = (v(i,j) + v(j,i)) * s;
		out[k] = (v(i,k) + v(k,i)) * s;
		out[3] = (v(j,k) - v(k,j)) * s;

		q->x = out[0];
		q->y = out[1];
		q->z = out[2];
		q->w = out[3];
	}
}

/* quat_inverse:
 *  A quaternion inverse is the conjugate divided by the normal.
 */
void Quaternion::inverse (Quaternion *out) const
{
	Quaternion  con;
	float norm;

	/* q^-1 = q^* / N(q) */

	conjugate (&con);
	norm = normal();

	/* NOTE: If the normal is 0 then a divide-by-zero exception will occur, we
	*       let this happen because the inverse of a zero quaternion is
	*       undefined
	*/
	assert(norm != 0);

	out->w = con.w / norm;
	out->x = con.x / norm;
	out->y = con.y / norm;
	out->z = con.z / norm;
}

/* apply_quat:
 *  Multiplies the point (x, y, z) by the quaternion q, storing the result in
 *  (*xout, *yout, *zout). This is quite a bit slower than apply_matrix_f.
 *  So, only use it if you only need to translate a handful of points.
 *  Otherwise it is much more efficient to call quat_to_matrix then use
 *  apply_matrix_f.
 */
void Quaternion::apply(float x, float y, float z, float *xout, float *yout, float *zout) const
{
   Quaternion v;
   Quaternion i;
   Quaternion t;

   /* v' = q * v * q^-1 */

   v.w = 0;
   v.x = x;
   v.y = y;
   v.z = z;

   /* NOTE: Rotating about a zero quaternion is undefined. This can be shown
	*       by the fact that the inverse of a zero quaternion is undefined
	*       and therefore causes an exception below.
	*/
	assert(!((x == 0) && (y == 0) && (z == 0)));

	inverse(&i);
	i.mul (&v, &t);
	t.mul (this, &v);

	*xout = v.x;
	*yout = v.y;	
	*zout = v.z;
}



void Quaternion::apply(Vector3* /*in*/, Vector3 *out) const
{
   Quaternion v;
   Quaternion i;
   Quaternion t;

   /* v' = q * v * q^-1 */

   v.w = 0;
   v.x = x;
   v.y = y;
   v.z = z;

   /* NOTE: Rotating about a zero quaternion is undefined. This can be shown
	*       by the fact that the inverse of a zero quaternion is undefined
	*       and therefore causes an exception below.
	*/
   assert(!((x == 0) && (y == 0) && (z == 0)));

	inverse(&i);
	i.mul (&v, &t);
	t.mul (this, &v);

	out->set (v.x, v.y, v.z);
}



/* slerp:
 *  Constructs a quaternion that represents a rotation between 'from' and
 *  'to'. The argument 't' can be anything between 0 and 1 and represents
 *  where between from and to the result will be.  0 returns 'from', 1
 *  returns 'to', and 0.5 will return a rotation exactly in between. The
 *  result is copied to 'out'.
 *
 *  The variable 'how' can be any one of the following:
 *
 *      qshort - This equivalent quat_interpolate, the rotation will
 *                   be less than 180 degrees
 *      qlong  - rotation will be greater than 180 degrees
 *      qcw    - rotation will be clockwise when viewed from above
 *      qccw   - rotation will be counterclockwise when viewed
 *                   from above
 *      quser  - the quaternions are interpolated exactly as given
 */
void Quaternion::slerp(const Quaternion *to, float t, Quaternion *out, SlerpType how) const
{
	const Quaternion *from = this;
	Quaternion   to2;
	double angle;
	double cos_angle;
	double scale_from;
	double scale_to;
	double sin_angle;

	cos_angle = (from->x * to->x) + (from->y * to->y) + (from->z * to->z) + (from->w * to->w);

	if (((how == qshort) && (cos_angle < 0.0)) ||
			((how == qlong)  && (cos_angle > 0.0)) ||
			((how == qcw)    && (from->w > to->w)) ||
			((how == qccw)   && (from->w < to->w))) {
		cos_angle = -cos_angle;
		to2.w     = -to->w;
		to2.x     = -to->x;
		to2.y     = -to->y;
		to2.z     = -to->z;
	}
	else
		to->copy (&to2);

	if ((1.0 - ABS(cos_angle)) > EPSILON) 
	{
		/* spherical linear interpolation (SLERP) */
		angle = acos(cos_angle);
		sin_angle  = sin(angle);
		scale_from = sin((1.0 - t) * angle) / sin_angle;
		scale_to   = sin(t         * angle) / sin_angle;
	}
	else {
		/* to prevent divide-by-zero, resort to linear interpolation */
		scale_from = 1.0 - t;
		scale_to   = t;
	}

	out->w = (float)((scale_from * from->w) + (scale_to * to2.w));
	out->x = (float)((scale_from * from->x) + (scale_to * to2.x));
	out->y = (float)((scale_from * from->y) + (scale_to * to2.y));
	out->z = (float)((scale_from * from->z) + (scale_to * to2.z));
}


void Quaternion::normalize ()
{
	float dot = (x * x) + (y * y) + (z * z) + (w * w);

	assert(dot != 0.0f);

	float scale = 1.0f / (float) sqrt(dot);

	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

void PrintMatrix(Matrix& m);

Matrix get_YXZRotationMatrix(Vector3& rot)
{
	Matrix r, a;
	
	// TA Style means Y-X-Z rotation:
	//logger.Print ("YRotate (%f):\n", rot.y);
	r.yrotate(rot.y);
	//PrintMatrix(r);

	//logger.Print("XRotate (%f):\n", rot.x);
	a.xrotate(rot.x);
	//PrintMatrix(a);

	r *= a;
	a.zrotate(rot.z);
	r *= a;
	return r;
}

void PrintMatrix(Matrix& m) {
	for (int a=0;a<4;a++)
		logger.Print("%3.3f, %3.3f, %3.3f, %3.3f\n", m[a*4], m[a*4+1], m[a*4+2], m[a*4+3]);
}

void math_test()
{
	Vector3 r(0.0f, 0.0f, M_PI/2);
	Matrix slow = get_YXZRotationMatrix(r);
	Matrix fast;
	fast.eulerZXY(r);

	logger.Print("Slow: \n"); PrintMatrix(slow);
	logger.Print("Fast: \n"); PrintMatrix(fast);

	bool eq=true;
	for (int a=0;a<16;a++) {
		if (fabsf(fast[a]-slow[a]) > 0.01f) {
			logger.Print("Element %d not equal: Fast: %3.3f, Slow: %3.3f\n", a, fast[a], slow[a]);
			eq=false;
		}
	}
	if (eq)
		logger.Print("Equal\n");

	logger.Print("Original angles: %3.3f, %3.3f, %3.3f\n", r.x, r.y,r.z);
	Vector3 cr=fast.calcEulerZXY();
	logger.Print("Calculated angles: %3.3f, %3.3f, %3.3f\n", cr.x,cr.y,cr.z);

	Matrix recalc;
	recalc.eulerZXY(cr);
	logger.Print("Recalculated: \n"); PrintMatrix(recalc);
	eq=true;
	for (int a=0;a<16;a++) {
		if (fabsf(fast[a]-recalc[a]) > 0.01f) {
			logger.Print("Element %d not equal: Fast: %3.3f, Slow: %3.3f\n", a, fast[a], recalc[a]);
			eq=false;
		}
	}
	if (eq)
		logger.Print("Equal\n");


	if (system("pause")) {}
}

