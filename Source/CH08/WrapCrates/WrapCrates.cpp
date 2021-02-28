#include "WrapCrates.h"
#include <d3dcompiler.h>
#include <vector>

D3DMAIN(WrapCratesApp);

WrapCratesApp::WrapCratesApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mInputLayout(0), mVS(0), mPS(0), mMatrixBuffer(0),
	 mViewPorts(),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Crate Demo";

	mLastMousePoint.x = 0;
	mLastMousePoint.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	for (int i = 0; i < 4; ++i)
	{
		mSampleState[i] = 0;
		mViewPorts[i] = 0;
	}
}

WrapCratesApp::~WrapCratesApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mVS);
	ReleaseCOM(mPS);
	ReleaseCOM(mMatrixBuffer);
	ReleaseCOM(mTexture);

	for (int i = 0; i < 4; ++i)
	{
		delete mViewPorts[i];
		ReleaseCOM(mSampleState[i]);
	}
}

bool WrapCratesApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildTex();
	BuildViewports();

	return true;
}

void WrapCratesApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompile the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

	BuildViewports();
}

void WrapCratesApp::UpdateScene(float dt)
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

void WrapCratesApp::DrawScene()
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

	for (int i = 0; i < 4; ++i)
	{
		md3dImmediateContext->RSSetViewports(1, mViewPorts[i]);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState[i]);
		md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);
	}
	
	
	HR(mSwapChain->Present(0, 0));;
}

void WrapCratesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePoint.x = x;
	mLastMousePoint.y = y;

	SetCapture(mhMainWnd);
}

void WrapCratesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void WrapCratesApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void WrapCratesApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator geoGen;
	geoGen.CreateBox(2.0f, 2.0f, 2.0f, box);

	std::vector<Vertex> vertices(box.Vertices.size());

	for (UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Tex.x = 3.0f * box.Vertices[i].TexC.x - 0.5f;
		vertices[i].Tex.y = 3.0f * box.Vertices[i].TexC.y - 0.5f;
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

void WrapCratesApp::BuildFX()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//ShaderHelper::CreateShader(md3dDevice, &mVS, L"../../../Shaders/BasicTexture.hlsl", "VS", 0, &mInputLayout, vertexDesc, 2);
	//ShaderHelper::CreateShader(md3dDevice, &mPS, L"../../../Shaders/BasicTexture.hlsl", "PS", 0);

	ShaderHelper::CreateShader(md3dDevice, &mVS, ExePath().append(L"../../../Shaders/BasicTexture.hlsl").c_str(), "VS", 0, &mInputLayout, vertexDesc, 2);
	ShaderHelper::CreateShader(md3dDevice, &mPS, ExePath().append(L"../../../Shaders/BasicTexture.hlsl").c_str(), "PS", 0);

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

void WrapCratesApp::BuildTex()
{
	ID3D11Resource* textureResource;
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/WoodCrate01.dds").c_str(), &textureResource, &mTexture));

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
	HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState[0]));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState[1]));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState[2]));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState[3]));

	ReleaseCOM(textureResource);
}

void WrapCratesApp::BuildViewports()
{
	D3D11_VIEWPORT a, b, c, d;

	a.TopLeftX = 0.0f;
	a.TopLeftY = 0.0f;
	a.Width = static_cast<float>(mClientWidth) / 2.0f;
	a.Height = static_cast<float>(mClientHeight) / 2.0f;
	a.MinDepth = 0.0f;
	a.MaxDepth = 1.0f;

	b = a;
	b.TopLeftX = a.Width;

	c = a;
	c.TopLeftY = a.Height;

	d = b;
	d.TopLeftY = c.TopLeftY;

	mViewPorts[0] = new D3D11_VIEWPORT(a);
	mViewPorts[1] = new D3D11_VIEWPORT(b);
	mViewPorts[2] = new D3D11_VIEWPORT(c);
	mViewPorts[3] = new D3D11_VIEWPORT(d);
}