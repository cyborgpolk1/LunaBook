#include <Windows.h>
#include <DirectXMath.h>
#include <iostream>

using namespace std;
using namespace DirectX;

// Overload the "<<" operators so that we can use cout to
// output XMVECTOR and XMMATRIX objects
ostream& operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", "
		<< dest.z << ", " << dest.w << ")";
	return os;
}

ostream& operator<<(ostream& os, CXMMATRIX m)
{
	XMFLOAT4X4 dest;
	XMStoreFloat4x4(&dest, m);

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			os << dest(i, j) << "\t";
		os << endl;
	}
	return os;
}

int main()
{
	// Check support for SSE2 (Pentium4, AMD K8, and above).
	if (!XMVerifyCPUSupport())
	{
		cout << "xna math not supported" << endl;
		return 0;
	}

	XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
			   0.0f, 2.0f, 0.0f, 0.0f,
		       0.0f, 0.0f, 4.0f, 0.0f,
		       1.0f, 2.0f, 3.0f, 1.0f);

	XMMATRIX B = XMMatrixIdentity();

	XMMATRIX C = A * B;

	XMMATRIX D = XMMatrixTranspose(A);

	XMVECTOR det = XMMatrixDeterminant(A);
	XMMATRIX E = XMMatrixInverse(&det, A);

	XMMATRIX F = A * E;

	cout << "A = " << endl << A << endl;
	cout << "B = " << endl << B << endl;
	cout << "C = A*B = " << endl << C << endl;
	cout << "D = transpose(A) = " << endl << D << endl;
	cout << "det = determinant(A) = " << det << endl << endl;
	cout << "E = inverse(A) = " << endl << E << endl;
	cout << "F = A*E = " << endl << F << endl;

	return 0;
}