#include "SubdivideIcos.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include <vector>

D3DMAIN(SubdivideIcosahedron);

SubdivideIcosahedron::SubdivideIcosahedron(HINSTANCE hInstance)
	: D3DApp(hInstance), mIcosVB(0), mIcosIB(0), mIcosIndexCount(0),
	mVS(0), mGS(0), mPS(0), mInputLayout(0), mConstantBuffer(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Subdivide Icosahedron";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

SubdivideIcosahedron::~SubdivideIcosahedron()
{
	ReleaseCOM(mIcosVB);
	ReleaseCOM(mIcosIB);
	ReleaseCOM(mVS);
	ReleaseCOM(mGS);
	ReleaseCOM(mPS);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mConstantBuffer);
}

bool SubdivideIcosahedron::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildIcosahedron();
	BuildFX();

	return true;
}

void SubdivideIcosahedron::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void SubdivideIcosahedron::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void SubdivideIcosahedron::DrawScene()
{

}

void SubdivideIcosahedron::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void SubdivideIcosahedron::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SubdivideIcosahedron::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box
		mTheta += dx;
		mPhi += dy;

		// Restart the angle mPhi
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input
		mRadius += dx - dy;

		// Restrict the radius
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SubdivideIcosahedron::BuildIcosahedron()
{

}

void SubdivideIcosahedron::BuildFX()
{

}