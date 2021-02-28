#include "OverdrawStencil.h"
#include <d3dcompiler.h>
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "Waves.h"
#include <vector>

D3DMAIN(OverdrawStencilApp);

OverdrawStencilApp::OverdrawStencilApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0), mQuadVB(0), mQuadIB(0),
	mVS(0), mPS(0), mSceneLayout(0), mQuadVS(0), mQuadPS(0), mQuadLayout(0),
	mPerObjectBuffer(0), mPerFrameBuffer(0), mPerQuadBuffer(0), mNoCullRS(0), 
	mGridIndexCount(0), mCrateIndexCount(0), mQuadIndexCount(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.1f*MathHelper::Pi), mRadius(200.0f), mWaterTexOffset(0.0f, 0.0f),
	mEyePosW(0.0f, 0.0f, 0.0f), mCrateVB(0), mCrateIB(0), mCrateTexture(0), 
	mTransparentBlend(0), mNoRenderTargetWriteBS(0), mRenderOverdrawDSS(0), mRenderToStencilDSS(0)
{
	mMainWndCaption = L"Overdraw Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX crateMatrix = XMMatrixScaling(15.0f, 15.0f, 15.0f) * XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&mCrateWorld, crateMatrix);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);;

	mLandMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 0.5f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);

	mCrateMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCrateMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCrateMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

OverdrawStencilApp::~OverdrawStencilApp()
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mCrateVB);
	ReleaseCOM(mCrateIB);
	ReleaseCOM(mQuadVB);
	ReleaseCOM(mQuadIB);
	ReleaseCOM(mSceneLayout);
	ReleaseCOM(mQuadLayout);
	ReleaseCOM(mVS);
	ReleaseCOM(mPS);
	ReleaseCOM(mQuadVS);
	ReleaseCOM(mQuadPS);
	ReleaseCOM(mPerObjectBuffer);
	ReleaseCOM(mPerFrameBuffer);
	ReleaseCOM(mPerQuadBuffer);
	ReleaseCOM(mLandTexture);
	ReleaseCOM(mWaterTexture);
	ReleaseCOM(mCrateTexture);
	ReleaseCOM(mSampleState);
	ReleaseCOM(mNoCullRS);
	ReleaseCOM(mRenderToStencilDSS);
	ReleaseCOM(mRenderOverdrawDSS);
	ReleaseCOM(mTransparentBlend);
	ReleaseCOM(mNoRenderTargetWriteBS);
}

bool OverdrawStencilApp::Init()
{
	if (!D3DApp::Init())
		return false;

	mWaves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	BuildQuadGeometryBuffers();
	BuildLandGeometryBuffers();
	BuildWavesGeometryBuffers();
	BuildCrateGeometryBuffers();
	BuildFX();
	BuildTex();
	BuildRasterizerStates();
	BuildDepthStencilStates();
	BuildBlendStates();

	return true;
}

void OverdrawStencilApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void OverdrawStencilApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	// Every quarter second, generate a random wave
	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	// Water texture transform
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	mWaterTexOffset.y += 0.05f * dt;
	mWaterTexOffset.x += 0.1f * dt;
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);


	// Update the wave vertex buffer with the new solution
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Normal = mWaves.Normal(i);
		v[i].Tex.x = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	md3dImmediateContext->Unmap(mWavesVB, 0);
}

void OverdrawStencilApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	float blendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	md3dImmediateContext->OMSetBlendState(mNoRenderTargetWriteBS, blendFactors, 0xffffffff);
	md3dImmediateContext->OMSetDepthStencilState(mRenderToStencilDSS, 0);
	drawToStencil();
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
	md3dImmediateContext->OMSetBlendState(0, blendFactors, 0xffffffff);

	//
	// Draw Overdraw Quads
	//
	md3dImmediateContext->IASetInputLayout(mQuadLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(QuadVertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);
	md3dImmediateContext->VSSetShader(mQuadVS, 0, 0);
	md3dImmediateContext->PSSetShader(mQuadPS, 0, 0);

	for (UINT i = 0; i < colors.size(); ++i) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HR(md3dImmediateContext->Map(mPerQuadBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

		PerQuadBuffer* quadDataPtr = (PerQuadBuffer*)mappedResource.pData;
		quadDataPtr->Color = colors[i];

		md3dImmediateContext->Unmap(mPerQuadBuffer, 0);
		md3dImmediateContext->VSSetConstantBuffers(0, 1, &mPerQuadBuffer);

		md3dImmediateContext->OMSetDepthStencilState(mRenderOverdrawDSS, i);
		md3dImmediateContext->DrawIndexed(mQuadIndexCount, 0, 0);
	}
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(mSwapChain->Present(0, 0));
}

void OverdrawStencilApp::drawToStencil()
{
	md3dImmediateContext->IASetInputLayout(mSceneLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set up Per Frame constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PerFrameBuffer* frameDataPtr = (PerFrameBuffer*)mappedResource.pData;
	frameDataPtr->DirLights[0] = mDirLights[0];
	frameDataPtr->DirLights[1] = mDirLights[1];
	frameDataPtr->DirLights[2] = mDirLights[2];
	frameDataPtr->EyePosW = mEyePosW;

	frameDataPtr->FogStart = 15.0f;
	frameDataPtr->FogRange = 175.0f;
	frameDataPtr->FogColor = XMFLOAT4(reinterpret_cast<const float*>(&Colors::Silver));

	md3dImmediateContext->Unmap(mPerFrameBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);


	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);



	// LAND
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
	XMMATRIX worldViewProj = world * view * proj;

	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PerObjectBuffer* dataPtr = (PerObjectBuffer*)mappedResource.pData;
	dataPtr->World = XMMatrixTranspose(world);
	dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	dataPtr->WorldViewProj = XMMatrixTranspose(worldViewProj);
	dataPtr->TextureTransform = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	dataPtr->Mat = mLandMat;

	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

	md3dImmediateContext->PSSetShaderResources(0, 1, &mLandTexture);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);


	// CRATE
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mCrateVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mCrateIB, DXGI_FORMAT_R32_UINT, 0);

	world = XMLoadFloat4x4(&mCrateWorld);
	worldViewProj = world * view * proj;

	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	dataPtr = (PerObjectBuffer*)mappedResource.pData;
	dataPtr->World = XMMatrixTranspose(world);
	dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	dataPtr->WorldViewProj = XMMatrixTranspose(worldViewProj);
	dataPtr->TextureTransform = XMMatrixIdentity();
	dataPtr->Mat = mCrateMat;

	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

	md3dImmediateContext->PSSetShaderResources(0, 1, &mCrateTexture);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->RSSetState(mNoCullRS);

	md3dImmediateContext->DrawIndexed(mCrateIndexCount, 0, 0);

	md3dImmediateContext->RSSetState(0);


	// WATER
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	world = XMLoadFloat4x4(&mWavesWorld);
	worldViewProj = world * view * proj;

	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	dataPtr = (PerObjectBuffer*)mappedResource.pData;
	dataPtr->World = XMMatrixTranspose(world);
	dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	dataPtr->WorldViewProj = XMMatrixTranspose(worldViewProj);
	dataPtr->TextureTransform = XMMatrixTranspose(XMLoadFloat4x4(&mWaterTexTransform));
	dataPtr->Mat = mWavesMat;

	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

	md3dImmediateContext->PSSetShaderResources(0, 1, &mWaterTexture);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

	float blendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	md3dImmediateContext->OMSetBlendState(mTransparentBlend, blendFactors, 0xfffffff);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

	md3dImmediateContext->OMSetBlendState(mNoRenderTargetWriteBS, blendFactors, 0xffffffff);
}

void OverdrawStencilApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void OverdrawStencilApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void OverdrawStencilApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input
		mRadius += dx - dy;

		// Restrict the radius
		mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void OverdrawStencilApp::BuildQuadGeometryBuffers()
{
	GeometryGenerator::MeshData quad;
	GeometryGenerator geoGen;
	geoGen.CreateFullScreenQuad(quad);

	std::vector<QuadVertex> vertices(quad.Vertices.size());

	for (UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos = quad.Vertices[i].Position;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(QuadVertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mQuadVB));

	mQuadIndexCount = quad.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mQuadIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &(quad.Indices[0]);
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mQuadIB));
}

float OverdrawStencilApp::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 OverdrawStencilApp::GetHillNormal(float x, float z) const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1,
		-0.03f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void OverdrawStencilApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mGridIndexCount = (UINT)grid.Indices.size();

	// Extract the vertex elements we are interested in and apply the height function to each vertex.
	// In addition, color the vertices based on their height so we have sandy looking beaches,
	// grassy low hills, and snow mountain peaks.

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * (UINT)grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));

	// Pack the indices of all the meshes into one index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)grid.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &grid.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mLandIB));
}

void OverdrawStencilApp::BuildWavesGeometryBuffers()
{
	// Create the vertex buffer. Note that we allocate space only,
	// as we will be updating the data every time step of the simulation.
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));

	// Create the index buffer. The index buffer is fixed,
	// so we only need to create and set once.
	std::vector<UINT> indices(3 * mWaves.TriangleCount());

	//Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6;
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mWavesIB));
}

void OverdrawStencilApp::BuildCrateGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	std::vector<Vertex> vertices(box.Vertices.size());

	for (UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mCrateVB));

	mCrateIndexCount = box.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mCrateIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &(box.Indices[0]);
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mCrateIB));
}

void OverdrawStencilApp::BuildFX()
{
	// Create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D_SHADER_MACRO basicEffectDefines[] = {
		{ "CLIP", "1" },
		{ 0, 0 }
	};

	ShaderHelper::CreateShader(md3dDevice, &mVS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "VS", 0, &mSceneLayout, vertexDesc, 3);
	ShaderHelper::CreateShader(md3dDevice, &mPS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "PS", basicEffectDefines);

	D3D11_INPUT_ELEMENT_DESC quadDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ShaderHelper::CreateShader(md3dDevice, &mQuadVS, ExePath().append(L"../../../Shaders/OverdrawQuad.hlsl").c_str(), "VS", 0, &mQuadLayout, quadDesc, 2);
	ShaderHelper::CreateShader(md3dDevice, &mQuadPS, ExePath().append(L"../../../Shaders/OverdrawQuad.hlsl").c_str(), "PS", 0);


	// Create matrix buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.ByteWidth = sizeof(PerObjectBuffer);
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerObjectBuffer));

	matrixBufferDesc.ByteWidth = sizeof(PerFrameBuffer);

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerFrameBuffer));

	matrixBufferDesc.ByteWidth = sizeof(PerQuadBuffer);

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerQuadBuffer));
}

void OverdrawStencilApp::BuildTex()
{
	ID3D11Resource* textureResource;
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/grass.dds").c_str(), &textureResource, &mLandTexture));

	ID3D11Resource* waterTextureResource;
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/water1.dds").c_str(), &waterTextureResource, &mWaterTexture));

	ID3D11Resource* crateTextureResource;
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/WireFence.dds").c_str(), &crateTextureResource, &mCrateTexture));

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
	ReleaseCOM(waterTextureResource);
	ReleaseCOM(crateTextureResource);
}

void OverdrawStencilApp::BuildRasterizerStates()
{
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&noCullDesc, &mNoCullRS));
}

void OverdrawStencilApp::BuildDepthStencilStates()
{
	D3D11_DEPTH_STENCIL_DESC renderToStencilDesc;
	renderToStencilDesc.DepthEnable = true;
	renderToStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	renderToStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	renderToStencilDesc.StencilEnable = true;
	renderToStencilDesc.StencilReadMask = 0xff;
	renderToStencilDesc.StencilWriteMask = 0xff;

	renderToStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	renderToStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	renderToStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
	renderToStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	renderToStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	renderToStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	renderToStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
	renderToStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(md3dDevice->CreateDepthStencilState(&renderToStencilDesc, &mRenderToStencilDSS));

	D3D11_DEPTH_STENCIL_DESC renderOverdrawDesc;
	renderOverdrawDesc.DepthEnable = false;
	renderOverdrawDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	renderOverdrawDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	renderOverdrawDesc.StencilEnable = true;
	renderOverdrawDesc.StencilReadMask = 0xff;
	renderOverdrawDesc.StencilWriteMask = 0xff;

	renderOverdrawDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	renderOverdrawDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	renderOverdrawDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	renderOverdrawDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	renderOverdrawDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	renderOverdrawDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	renderOverdrawDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	renderOverdrawDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(md3dDevice->CreateDepthStencilState(&renderOverdrawDesc, &mRenderOverdrawDSS));
}

void OverdrawStencilApp::BuildBlendStates()
{
	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(md3dDevice->CreateBlendState(&blendDesc, &mTransparentBlend));

	D3D11_BLEND_DESC noRenderTargetWriteDesc = { 0 };
	noRenderTargetWriteDesc.AlphaToCoverageEnable = false;
	noRenderTargetWriteDesc.IndependentBlendEnable = false;
	noRenderTargetWriteDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWriteDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWriteDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWriteDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWriteDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWriteDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWriteDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWriteDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(md3dDevice->CreateBlendState(&noRenderTargetWriteDesc, &mNoRenderTargetWriteBS));
}