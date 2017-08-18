#include "Crate.h"
#include <d3dcompiler.h>
#include <vector>

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
	ReleaseCOM(mTexture);
	ReleaseCOM(mSampleState);
}

bool CrateApp::Init() 
{ 
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildTex();

	return true; 
}

void CrateApp::OnResize() 
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompile the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CrateApp::UpdateScene(float dt) 
{
	// Convert Spherical to Cartesian coordinate
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void CrateApp::DrawScene() 
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX worldViewProj = world*view*proj;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	
	MatrixBuffer* dataPtr = (MatrixBuffer*)mappedResource.pData;
	dataPtr->WorldViewProj = XMMatrixTranspose(worldViewProj);

	md3dImmediateContext->Unmap(mMatrixBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &mMatrixBuffer);

	md3dImmediateContext->PSSetShaderResources(0, 1, &mTexture);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

	md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);

	HR(mSwapChain->Present(0, 0));;
}

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

void CrateApp::BuildGeometryBuffers() 
{
	GeometryGenerator::MeshData box;
	GeometryGenerator geoGen;
	geoGen.CreateBox(2.0f, 2.0f, 2.0f, box);

	std::vector<Vertex> vertices(box.Vertices.size());

	for (UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * box.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	mIndexCount = box.Indices.size();

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &(box.Indices[0]);
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mIB));
}

void CrateApp::BuildFX() 
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	CreateShader(&mVS, L"../../../Shaders/BasicTexture.hlsl", "VS", 0, &mInputLayout, vertexDesc, 2);
	CreateShader(&mPS, L"../../../Shaders/BasicTexture.hlsl", "PS", 0);

	// Create matrix buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mMatrixBuffer));
}

void CrateApp::BuildTex()
{
	ID3D11Resource* textureResource;
	HR(CreateDDSTextureFromFile(md3dDevice, L"../../../Textures/WoodCrate01.dds", &textureResource, &mTexture));

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState));

	ReleaseCOM(textureResource);
}