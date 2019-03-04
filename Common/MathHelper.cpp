//***************************************************************************************
// MathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "MathHelper.h"
#include <float.h>
#include <cmath>
#include <random>

using namespace DirectX;

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::RandF_Ex()
{
	static std::default_random_engine dre;
	static std::uniform_real_distribution<> urd; // range [0,1]
	return urd(dre);
}

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;

	// Quadrant ¥° or ¥³
	if (x >= 0.0f)
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if (theta < 0.0f)
			theta += 2.0f*Pi; // in [0, 2*pi]
	}
	// Quadrant ¥± or ¥²
	else
		theta = atanf(y / x) + Pi;

	return theta;
}

XMVECTOR MathHelper::RandUnitVector()
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	// Keep trying untill get a point on/in the hemisphere
	while (true)
	{
		// Generate random point in the cube[-1,1]^3
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		if (XMVector3Greater(XMVector3LengthSq(v), One)) // x^2 + y^2 + z^2 > 1
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR MathHelper::RandHemisphereUnitVector(XMVECTOR n)
{
	XMVECTOR One   = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying untill get a point on/in the hemisphere
	while (true)
	{
		// Generate random point in the cube[-1,1]^3
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		if (XMVector3Greater(XMVector3LengthSq(v), One)) // x^2 + y^2 + z^2 > 1
			continue;

		// Igonre points in the bottom hemisphere.
		if (XMVector3Less(XMVector3Dot(n, v), Zero))
			continue;

		return XMVector3Normalize(v);
	}
}
