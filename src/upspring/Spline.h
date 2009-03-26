
#ifndef SPLINE_H
#define SPLINE_H

inline void CubicHermiteSplineWeights(float t, float w[4])
{
	w[0] = t*t * (2.0f*t - 3.0f) + 1;
	w[1] = t*t * (t - 2.0f) + t;
	w[2] = t*t * (-2.0f*t + 3.0f);
	w[3] = t*t * (t - 1.0f);
}

#endif
