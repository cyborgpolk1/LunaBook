#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <Windows.h>
#include <DirectXMath.h>
#include <float.h>
#include <cmath>

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
	inline XMVECTOR RandHemisphereUnitVec3(XMVECTOR n)
    {
        XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
        XMVECTOR Zero = XMVectorZero();

        // Keep trying until we get a point on/in the hemisphere
        while (true)
        {
            // Generate random point in the cube [-1, 1]^3
            XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

            // Ignore points outside the unit sphere in order to get an even distribution
            // over the unit sphere. Otherwise points will clump more on the sphere near
            // the corners of the cube.
            if (XMVector3Greater(XMVector3LengthSq(v), One))
                continue;

            // Ignore points in the bottom hemisphere
            if (XMVector3Less(XMVector3Dot(n, v), Zero))
                continue;

            return XMVector3Normalize(v);
        }
    }

	const float Infinity = FLT_MAX;
	const float Pi = XM_PI;
};

#endif