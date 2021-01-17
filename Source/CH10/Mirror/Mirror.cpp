#include "Mirror.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "DDSTextureLoader.h"
#include <vector>
#include <fstream>
#include <string>

D3DMAIN(MirrorDemo);

MirrorDemo::MirrorDemo(HINSTANCE hInstance)
	: D3DApp(hInstance), mRoomVB(0), mSkullVB(0), mSkullIB(0), mSkullIndexCount(0),
	mTexVS(0), mLitVS(0), mTexPS(0), mLitPS(0), mPerFrameBuffer(0), mPerObjectBuffer(0),
	mRoomInputLayout(0), mSkullInputLayout(0), mEyePosW(0.0f, 0.0f, 0.0f), mSampleState(0),
	mFloorTexture(0), mWallTexture(0), mMirrorTexture(0), mSkullTranslation(0.0f, 1.0f, -5.0f),
	mTheta(1.24f * MathHelper::Pi), mPhi(0.42f * MathHelper::Pi), mRadius(12.0f)
{
	mMainWndCaption = L"Mirror Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mRoomWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.55735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.55735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mRoomMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mRoomMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mRoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mMirrorMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMirrorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mShadowMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mShadowMat.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

MirrorDemo::~MirrorDemo()
{
	ReleaseCOM(mRoomVB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mRoomInputLayout);
	ReleaseCOM(mSkullInputLayout);
	ReleaseCOM(mTexVS);
	ReleaseCOM(mLitVS);
	ReleaseCOM(mTexPS);
	ReleaseCOM(mLitPS);
	ReleaseCOM(mPerObjectBuffer);
	ReleaseCOM(mPerFrameBuffer);
	ReleaseCOM(mFloorTexture);
	ReleaseCOM(mWallTexture);
	ReleaseCOM(mMirrorTexture);
	ReleaseCOM(mSampleState);
}

bool MirrorDemo::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildRoomGeometryBuffer();
	BuildSkullGeometryBuffer();
	BuildFX();
	BuildTex();

	return true;
}

void MirrorDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void MirrorDemo::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	// Move skull
	if (GetAsyncKeyState('A') & 0x8000)
		mSkullTranslation.x -= 1.0f * dt;

	if (GetAsyncKeyState('D') & 0x8000)
		mSkullTranslation.x += 1.0f * dt;

	if (GetAsyncKeyState('W') & 0x8000)
		mSkullTranslation.y += 1.0f * dt;

	if (GetAsyncKeyState('S') & 0x8000)
		mSkullTranslation.y -= 1.0f * dt;

	mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, 0.0f);

	XMMATRIX skullRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
	XMStoreFloat4x4(&mSkullWorld, skullRotate * skullScale * skullOffset);
}

void MirrorDemo::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PerFrameBuffer* frameDataPtr = (PerFrameBuffer*)mappedResource.pData;
	frameDataPtr->EyePosW = mEyePosW;
	frameDataPtr->DirLights[0] = mDirLights[0];
	frameDataPtr->DirLights[1] = mDirLights[1];
	frameDataPtr->DirLights[2] = mDirLights[2];

	md3dImmediateContext->Unmap(mPerFrameBuffer, 0);

	md3dImmediateContext->IASetInputLayout(mRoomInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(TexVertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);
	
	md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
	md3dImmediateContext->PSSetShader(mTexPS, 0, 0);

	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	//
	// Constants common to room objects
	//
	XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);

	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PerObjectBuffer* objectDataPtr = (PerObjectBuffer*)mappedResource.pData;
	objectDataPtr->World = XMMatrixTranspose(world);
	objectDataPtr->WorldViewProj = XMMatrixTranspose(world * view * proj);
	objectDataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	objectDataPtr->gTexTransform = XMMatrixIdentity();
	objectDataPtr->Mat = mRoomMat;

	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);

	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

	//
	// Floor
	//
	md3dImmediateContext->PSSetShaderResources(0, 1, &mFloorTexture);
	md3dImmediateContext->Draw(6, 0);

	//
	// Wall
	//
	md3dImmediateContext->PSSetShaderResources(0, 1, &mWallTexture);
	md3dImmediateContext->Draw(18, 6);

	//
	// Skull
	//
	md3dImmediateContext->IASetInputLayout(mSkullInputLayout);
	stride = sizeof(BasicVertex);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->VSSetShader(mLitVS, 0, 0);
	md3dImmediateContext->PSSetShader(mLitPS, 0, 0);

	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	world = XMLoadFloat4x4(&mSkullWorld);

	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	objectDataPtr = (PerObjectBuffer*)mappedResource.pData;
	objectDataPtr->World = XMMatrixTranspose(world);
	objectDataPtr->WorldViewProj = XMMatrixTranspose(world * view * proj);
	objectDataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	objectDataPtr->gTexTransform = XMMatrixIdentity();
	objectDataPtr->Mat = mSkullMat;

	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);

	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

	md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

	//
	// Mirror
	//
	md3dImmediateContext->IASetInputLayout(mRoomInputLayout);
	stride = sizeof(TexVertex);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVB, &stride, &offset);

	md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
	md3dImmediateContext->PSSetShader(mTexPS, 0, 0);

	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	world = XMLoadFloat4x4(&mRoomWorld);

	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	objectDataPtr = (PerObjectBuffer*)mappedResource.pData;
	objectDataPtr->World = XMMatrixTranspose(world);
	objectDataPtr->WorldViewProj = XMMatrixTranspose(world * view * proj);
	objectDataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	objectDataPtr->gTexTransform = XMMatrixIdentity();
	objectDataPtr->Mat = mMirrorMat;

	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);

	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
	md3dImmediateContext->PSSetShaderResources(0, 1, &mMirrorTexture);
	md3dImmediateContext->Draw(6, 24);

	HR(mSwapChain->Present(0, 0));
}

void MirrorDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void MirrorDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MirrorDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;

		mRadius = MathHelper::Clamp(mRadius, 3.0f, 50.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void MirrorDemo::BuildRoomGeometryBuffer()
{
	TexVertex v[30];

	// Floor
	v[0] = TexVertex(-3.5f, 0.0f, -10.0f,    0.0f, 1.0f, 0.0f,    0.0f, 4.0f);
	v[1] = TexVertex(-3.5f, 0.0f,   0.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f);
	v[2] = TexVertex( 7.5f, 0.0f,   0.0f,    0.0f, 1.0f, 0.0f,    4.0f, 0.0f);

	v[3] = TexVertex(-3.5f, 0.0f, -10.0f,    0.0f, 1.0f, 0.0f,    0.0f, 4.0f);
	v[4] = TexVertex( 7.5f, 0.0f,   0.0f,    0.0f, 1.0f, 0.0f,    4.0f, 0.0f);
	v[5] = TexVertex( 7.5f, 0.0f, -10.0f,    0.0f, 1.0f, 0.0f,    4.0f, 4.0f);
	
	// Wall
	v[6] = TexVertex(-3.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 2.0f);
	v[7] = TexVertex(-3.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 2.0f);
	v[8] = TexVertex(-2.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.5f, 2.0f);

	v[9]  = TexVertex(-3.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 2.0f);
	v[10] = TexVertex(-2.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.5f, 0.0f);
	v[11] = TexVertex(-2.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.5f, 2.0f);

	v[12] = TexVertex(2.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 2.0f);
	v[13] = TexVertex(2.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f);
	v[14] = TexVertex(7.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    2.0f, 0.0f);

	v[15] = TexVertex(2.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 2.0f);
	v[16] = TexVertex(7.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    2.0f, 0.0f);
	v[17] = TexVertex(7.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    2.0f, 2.0f);

	v[18] = TexVertex(-3.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f);
	v[19] = TexVertex(-3.5f, 6.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f);
	v[20] = TexVertex( 7.5f, 6.0f, 0.0f,    0.0f, 0.0f, -1.0f,    6.0f, 0.0f);

	v[21] = TexVertex(-3.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f);
	v[22] = TexVertex( 7.5f, 6.0f, 0.0f,    0.0f, 0.0f, -1.0f,    6.0f, 0.0f);
	v[23] = TexVertex( 7.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    6.0f, 1.0f);

	// Mirror
	v[24] = TexVertex(-2.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f);
	v[25] = TexVertex(-2.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f);
	v[26] = TexVertex( 2.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    1.0f, 0.0f);

	v[27] = TexVertex(-2.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f);
	v[28] = TexVertex( 2.5f, 4.0f, 0.0f,    0.0f, 0.0f, -1.0f,    1.0f, 0.0f);
	v[29] = TexVertex( 2.5f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TexVertex) * 30;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mRoomVB));
}

void MirrorDemo::BuildSkullGeometryBuffer()
{
	// Load skull
	std::ifstream fin(ExePath().append(L"../../../Models/skull.txt").c_str());

	if (!fin)
	{
		MessageBox(0, L"skull.txt not found", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<BasicVertex> skullVertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> skullVertices[i].Pos.x >> skullVertices[i].Pos.y >> skullVertices[i].Pos.z;
		fin >> skullVertices[i].Normal.x >> skullVertices[i].Normal.y >> skullVertices[i].Normal.z;
	}

	fin >> ignore >> ignore >> ignore;

	mSkullIndexCount = 3 * tcount;
	std::vector<UINT> skullIndices(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> skullIndices[i * 3 + 0] >> skullIndices[i * 3 + 1] >> skullIndices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(BasicVertex) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &skullVertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &skullIndices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mSkullIB));
}

void MirrorDemo::BuildFX()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	CreateShader(&mTexVS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "VS", 0, &mRoomInputLayout, vertexDesc, 3);
	CreateShader(&mLitVS, ExePath().append(L"../../../Shaders/BasicEffect.hlsl").c_str(), "VS", 0, &mSkullInputLayout, vertexDesc, 2);

	CreateShader(&mTexPS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "PS", 0);
	CreateShader(&mLitPS, ExePath().append(L"../../../Shaders/BasicEffect.hlsl").c_str(), "PS", 0);


	D3D11_BUFFER_DESC perObjectBufferDesc;
	perObjectBufferDesc.ByteWidth = sizeof(PerObjectBuffer);
	perObjectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectBufferDesc.MiscFlags = 0;
	perObjectBufferDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&perObjectBufferDesc, 0, &mPerObjectBuffer));

	D3D11_BUFFER_DESC perFrameBufferDesc;
	perFrameBufferDesc.ByteWidth = sizeof(PerFrameBuffer);
	perFrameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perFrameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameBufferDesc.MiscFlags = 0;
	perFrameBufferDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&perFrameBufferDesc, 0, &mPerFrameBuffer));
}

void MirrorDemo::BuildTex()
{
	ID3D11Resource *floorResource, *brickResource, *mirrorResource;

	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/floor.dds").c_str(), &floorResource, &mFloorTexture));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/bricks.dds").c_str(), &brickResource, &mWallTexture));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/ice.dds").c_str(), &mirrorResource, &mMirrorTexture));

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

	ReleaseCOM(floorResource);
	ReleaseCOM(brickResource);
	ReleaseCOM(mirrorResource);
}