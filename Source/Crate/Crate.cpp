#include "Crate.h"
#include <d3dcompiler.h>

D3DMAIN(CrateApp);

CrateApp::CrateApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mInputLayout(0), mVS(0), mPS(0), mMatrixBuffer(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Crate Demo";

	mLastMousePoint.x = 0;
	mLastMousePoint.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

CrateApp::~CrateApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mVS);
	ReleaseCOM(mPS);
	ReleaseCOM(mMatrixBuffer);
}

bool CrateApp::Init() 
{ 
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();

	return true; 
}

void CrateApp::OnResize() 
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompile the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CrateApp::UpdateScene(float dt) {}

void CrateApp::DrawScene() {}

void CrateApp::OnMouseDown(WPARAM btnState, int x, int y) 
{
	mLastMousePoint.x = x;
	mLastMousePoint.y = y;

	SetCapture(mhMainWnd);
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y) 
{
	ReleaseCapture();
}

void CrateApp::OnMouseMove(WPARAM btnState, int x, int y) 
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePoint.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePoint.y));

		// Update angles based on input to orbit camera around the box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePoint.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePoint.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePoint.x = x;
	mLastMousePoint.y = y;
}

void CrateApp::BuildGeometryBuffers() {}

void CrateApp::BuildFX() {}