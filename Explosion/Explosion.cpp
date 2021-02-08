#include "Explosion.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include <vector>

D3DMAIN(Explosion);

Explosion::Explosion(HINSTANCE hInstance)
	:D3DApp(hInstance), mIcosVB(0), mIcosIB(0), mIcosIndexCount(0),
	mVS(0), mGS(0), mPS(0), mInputLayout(0), mConstantBuffer(0), mNoCullRS(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Explosion Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

Explosion::~Explosion()
{
	ReleaseCOM(mIcosVB);
	ReleaseCOM(mIcosIB);
	ReleaseCOM(mVS);
	ReleaseCOM(mGS);
	ReleaseCOM(mPS);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mConstantBuffer);
	ReleaseCOM(mNoCullRS);
}

bool Explosion::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildIcosahedron();
	BuildFX();

	D3D11_RASTERIZER_DESC noCullDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	noCullDesc.CullMode = D3D11_CULL_NONE;
	HR(md3dDevice->CreateRasterizerState(&noCullDesc, &mNoCullRS));

	return true;
}

void Explosion::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Explosion::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void Explosion::DrawScene()
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
	D3D11_MAPPED_SUBRESOURCE mappredResource;
	HR(md3dImmediateContext->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappredResource));

	ConstantBuffer* dataPtr = (ConstantBuffer*)mappredResource.pData;
	dataPtr->WorldViewProj = XMMatrixTranspose(view * proj);
	dataPtr->Time = mTimer.TotalTime();

	md3dImmediateContext->Unmap(mConstantBuffer, 0);
	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->GSSetShader(mGS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->RSSetState(mNoCullRS);
	md3dImmediateContext->DrawIndexed(mIcosIndexCount, 0, 0);
	md3dImmediateContext->RSSetState(0);

	HR(mSwapChain->Present(0, 0));
}

void Explosion::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Explosion::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Explosion::OnMouseMove(WPARAM btnState, int x, int y)
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

void Explosion::BuildIcosahedron()
{
	GeometryGenerator::MeshData icos;
	GeometryGenerator geoGen;
	geoGen.CreateGeosphere(1.0f, 0, icos);

	std::vector<Vertex> vertices(icos.Vertices.size());

	for (UINT i = 0; i < vertices.size(); ++i)
	{
		vertices[i].Pos = icos.Vertices[i].Position;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.StructureByteStride = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices.data();
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mIcosVB));

	mIcosIndexCount = icos.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIcosIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = icos.Indices.data();
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mIcosIB));
}

void Explosion::BuildFX()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	auto filename = ExePath().append(L"../../../Shaders/Explosion.hlsl");
	auto cstr = filename.c_str();

	CreateShader(&mVS, cstr, "VS", 0, &mInputLayout, vertexDesc, 1);
	CreateShader(&mGS, cstr, "GS", 0);
	CreateShader(&mPS, cstr, "PS", 0);

	D3D11_BUFFER_DESC constantDesc;
	constantDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantDesc.ByteWidth = sizeof(ConstantBuffer);
	constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantDesc.MiscFlags = 0;
	constantDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&constantDesc, 0, &mConstantBuffer));
}