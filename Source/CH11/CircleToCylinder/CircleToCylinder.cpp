#include "CircleToCylinder.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include <vector>

D3DMAIN(CircleToCylinder);

CircleToCylinder::CircleToCylinder(HINSTANCE hInstance)
	: D3DApp(hInstance), mCircleVB(0), mCircleIB(0), mCircleIndexCount(0),
	mVS(0), mGS(0), mPS(0), mInputLayout(0), mConstantBuffer(0), mRS(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Circle to Cylinder Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

CircleToCylinder::~CircleToCylinder()
{
	ReleaseCOM(mCircleVB);
	ReleaseCOM(mCircleIB);
	ReleaseCOM(mVS);
	ReleaseCOM(mGS);
	ReleaseCOM(mPS);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mConstantBuffer);
	ReleaseCOM(mRS);
}

bool CircleToCylinder::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildCircleBuffer();
	BuildFX();

	D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;

	HR(md3dDevice->CreateRasterizerState(&rsDesc, &mRS));

	return true;
}

void CircleToCylinder::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CircleToCylinder::UpdateScene(float dt)
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

void CircleToCylinder::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mCircleVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mCircleIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	ConstantBuffer* dataPtr = (ConstantBuffer*)mappedResource.pData;
	dataPtr->ViewProj = XMMatrixTranspose(view * proj);

	md3dImmediateContext->Unmap(mConstantBuffer, 0);
	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->GSSetShader(mGS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->RSSetState(mRS);

	md3dImmediateContext->DrawIndexed(mCircleIndexCount, 0, 0);

	HR(mSwapChain->Present(0, 0));
}

void CircleToCylinder::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CircleToCylinder::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CircleToCylinder::OnMouseMove(WPARAM btnState, int x, int y)
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

void CircleToCylinder::BuildCircleBuffer()
{
	XMVECTOR color1 = XMLoadFloat4(&(XMFLOAT4)Colors::Red);
	XMVECTOR color2 = XMLoadFloat4(&(XMFLOAT4)Colors::Green);

	std::vector<Vertex> vertices(mCircleVertexCount);
	for (UINT i = 0; i < vertices.size(); ++i)
	{
		float angle = i * MathHelper::Pi * 2 / mCircleVertexCount;
		vertices[i].Pos = XMFLOAT3(cosf(angle), 0.0f, sinf(angle));
		XMStoreFloat4(&vertices[i].Color, MathHelper::Lerp(color1, color2, i / (float)mCircleVertexCount));
	}

	mCircleIndexCount = mCircleVertexCount + 1;
	std::vector<UINT> indices(mCircleIndexCount);
	for (UINT i = 0; i < mCircleIndexCount - 1; ++i)
	{
		indices[i] = i;
	}
	indices[mCircleIndexCount-1] = 0;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices.data();
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mCircleVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indices.data();
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mCircleIB));
}

void CircleToCylinder::BuildFX()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ShaderHelper::CreateShader(md3dDevice, &mVS, ExePath().append(L"../../../Shaders/CircleToCylinder.hlsl").c_str(), "VS", 0, &mInputLayout, vertexDesc, 2);
	ShaderHelper::CreateShader(md3dDevice, &mGS, ExePath().append(L"../../../Shaders/CircleToCylinder.hlsl").c_str(), "GS", 0);
	ShaderHelper::CreateShader(md3dDevice, &mPS, ExePath().append(L"../../../Shaders/CircleToCylinder.hlsl").c_str(), "PS", 0);

	D3D11_BUFFER_DESC constantDesc;
	constantDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantDesc.ByteWidth = sizeof(ConstantBuffer);
	constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantDesc.MiscFlags = 0;
	constantDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&constantDesc, 0, &mConstantBuffer));
}