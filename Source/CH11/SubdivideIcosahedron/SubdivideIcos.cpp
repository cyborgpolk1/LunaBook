#include "SubdivideIcos.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include <vector>

D3DMAIN(SubdivideIcosahedron);

SubdivideIcosahedron::SubdivideIcosahedron(HINSTANCE hInstance)
	: D3DApp(hInstance), mIcosVB(0), mIcosIB(0), mIcosIndexCount(0),
	mVS(0), mGS(0), mPS(0), mInputLayout(0), mConstantBuffer(0), mWireframeRS(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Subdivide Icosahedron";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	ZeroMemory(mViewports.data(), mViewports.size() * sizeof(D3D11_VIEWPORT));
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
	ReleaseCOM(mWireframeRS);
}

bool SubdivideIcosahedron::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildIcosahedron();
	BuildFX();
	BuildViewports();

	D3D11_RASTERIZER_DESC wireframeDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));

	return true;
}

void SubdivideIcosahedron::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio() / mViewports.size(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void SubdivideIcosahedron::UpdateScene(float dt)
{
	static const float originalRadius = mRadius;

	// Convert Spherical to Cartesian coordinates.
	float x = sinf(mPhi)*cosf(mTheta);
	float z = sinf(mPhi)*sinf(mTheta);
	float y = cosf(mPhi);
	XMVECTOR preRadiusPos = XMVectorSet(x, y, z, 1.0f);

	// Build the no-zoom view matrix.
	XMVECTOR pos = preRadiusPos * XMVectorSet(originalRadius, originalRadius, originalRadius, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mNoZoomView, V);

	// Build the view matrix.
	pos = preRadiusPos * XMVectorSet(mRadius, mRadius, mRadius, 1.0f);

	V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void SubdivideIcosahedron::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mIcosVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIcosIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	ConstantBuffer* dataPtr = (ConstantBuffer*)mappedResource.pData;
	dataPtr->View = XMMatrixTranspose(view);
	dataPtr->ViewProj = XMMatrixTranspose(view * proj);

	md3dImmediateContext->Unmap(mConstantBuffer, 0);
	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->GSSetShader(mGS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->RSSetViewports(1, &mViewports[0]);
	md3dImmediateContext->DrawIndexed(mIcosIndexCount, 0, 0);

	XMMATRIX noZoomView = XMLoadFloat4x4(&mNoZoomView);
	HR(md3dImmediateContext->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	dataPtr = (ConstantBuffer*)mappedResource.pData;
	dataPtr->View = XMMatrixTranspose(view);
	dataPtr->ViewProj = XMMatrixTranspose(noZoomView * proj);

	md3dImmediateContext->Unmap(mConstantBuffer, 0);
	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dImmediateContext->RSSetViewports(1, &mViewports[1]);
	md3dImmediateContext->RSSetState(mWireframeRS);
	md3dImmediateContext->DrawIndexed(mIcosIndexCount, 0, 0);

	md3dImmediateContext->RSSetState(0);

	HR(mSwapChain->Present(0, 0));
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
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 25.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SubdivideIcosahedron::BuildIcosahedron()
{
	const float X = 0.525731f;
	const float Z = 0.850651f;

	std::vector<Vertex> vertices = 
	{
		{XMFLOAT3(-X, 0.0f, Z)},	{XMFLOAT3(X, 0.0f, Z)},
		{XMFLOAT3(-X, 0.0f, -Z)},	{XMFLOAT3(X, 0.0f, -Z)},
		{XMFLOAT3(0.0f, Z, X)},		{XMFLOAT3(0.0f, Z, -X)},
		{XMFLOAT3(0.0f, -Z, X)},	{XMFLOAT3(0.0f, -Z, -X)},
		{XMFLOAT3(Z, X, 0.0f)},		{XMFLOAT3(-Z, X, 0.0f)},
		{XMFLOAT3(Z, -X, 0.0f)},	{XMFLOAT3(-Z, -X, 0.0f)}
	};

	std::vector<UINT> indices = 
	{
		1,4,0,	4,9,0,	4,5,9,	8,5,4,	1,8,4,
		1,10,8,	10,3,8,	8,3,5,	3,2,5,	3,7,2,
		3,10,7,	10,6,7,	6,11,7,	6,0,11,	6,1,0,
		10,1,6,	11,0,9,	2,11,9,	5,2,9,	11,2,7
	};

	mIcosIndexCount = indices.size();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices.data();
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mIcosVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indices.data();
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mIcosIB));
}

void SubdivideIcosahedron::BuildFX()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	CreateShader(&mVS, ExePath().append(L"../../../Shaders/SubdivideIcosahedron.hlsl").c_str(), "VS", 0, &mInputLayout, vertexDesc, 1);
	CreateShader(&mGS, ExePath().append(L"../../../Shaders/SubdivideIcosahedron.hlsl").c_str(), "GS", 0);
	CreateShader(&mPS, ExePath().append(L"../../../Shaders/SubdivideIcosahedron.hlsl").c_str(), "PS", 0);

	D3D11_BUFFER_DESC constantDesc;
	constantDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantDesc.ByteWidth = sizeof(ConstantBuffer);
	constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantDesc.MiscFlags = 0;
	constantDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&constantDesc, 0, &mConstantBuffer));
}

void SubdivideIcosahedron::BuildViewports()
{
	mViewports[0].TopLeftX = 0.0f;
	mViewports[0].TopLeftY = 0.0f;
	mViewports[0].Width = static_cast<float>(mClientWidth) / 2.0f;
	mViewports[0].Height = static_cast<float>(mClientHeight);
	mViewports[0].MinDepth = 0.0f;
	mViewports[0].MaxDepth = 1.0f;

	mViewports[1].TopLeftX = mViewports[0].Width;
	mViewports[1].TopLeftY = 0.0f;
	mViewports[1].Width = static_cast<float>(mClientWidth) / 2.0f;
	mViewports[1].Height = static_cast<float>(mClientHeight);
	mViewports[1].MinDepth = 0.0f;
	mViewports[1].MaxDepth = 1.0f;
}