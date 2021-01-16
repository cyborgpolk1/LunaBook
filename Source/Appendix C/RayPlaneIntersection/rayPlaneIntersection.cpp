#include <windows.h>
#include <DirectXMath.h>
#include <iostream>

using namespace std;
using namespace DirectX;

// Overload the "<<" operators so that we can use the cout to
// output XMVECTOR objects
ostream& operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ")";
	return os;
}

int main()
{
	XMVECTOR p0 = XMVectorSet(-1.0f, 1.0f, -1.0f, 1.0f);
	XMVECTOR u = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR plane = XMVectorSet(1.0f / sqrtf(3), 1.0f / sqrtf(3), 1.0f / sqrtf(3), -5.0f);

	XMVECTOR intersection = XMPlaneIntersectLine(plane, p0, p0 + 100 * u);

	cout << intersection << endl;

	cout << endl;
	return 0;
}