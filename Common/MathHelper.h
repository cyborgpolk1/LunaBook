#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <Windows.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace MathHelper
{
	// Returns random float in [0, 1)
	inline float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b)
	inline float RandF(float a, float b)
	{
		return a + RandF() * (b - a);
	}

	template<typename T>
	inline T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	inline T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template<typename T>
	inline T Lerp(const T& a, const T& b, float t)
	{
		return a + (b - a)*t;
	}

	template<typename T>
	inline T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI)
	float AngleFromXY(float x, float y);

	inline XMMATRIX InverseTranspose(CXMMATRIX M)
	{
		XMMATRIX A = M;
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixTranspose(XMMatrixInverse(&det, A));
	}

	XMVECTOR RandUnitVec3();
	XMVECTOR RandHemisphereUnitVec3(XMVECTOR n);

	const float Infinity = FLT_MAX;
	const float Pi = XM_PI;
};

#endif