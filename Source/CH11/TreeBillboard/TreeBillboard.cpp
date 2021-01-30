#include "TreeBillboard.h"
#include <d3dcompiler.h>
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "Waves.h"
#include <vector>

D3DMAIN(TreeBillboard);

TreeBillboard::TreeBillboard(HINSTANCE hInstance)
	: D3DApp(hInstance), mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0), mTreeVB(0), mCrateVB(0), mCrateIB(0),
	mVS(0), mPS(0), mBillVS(0), mBillGS(0), mBillPS(0), mInputLayout(0), mTreeLayout(0),
	mPerObjectBuffer(0), mPerFrameBuffer(0), mNoCullRS(0), mGridIndexCount(0),
	mCrateTexture(0), mLandTexture(0), mWaterTexture(0), mTreeTextureArray(0),
	mTheta(1.5f*MathHelper::Pi), mPhi(0.1f*MathHelper::Pi), mRadius(200.0f), mWaterTexOffset(0.0f, 0.0f),
	mEyePosW(0.0f, 0.0f, 0.0f), mTransparentBlend(0), mAlphaToCoverageBS(0)
{
	mMainWndCaption = L"Tree Billboard Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	m4xMsaaQuality = true;

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

	mTreeMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

TreeBillboard::~TreeBillboard()
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mCrateVB);
	ReleaseCOM(mCrateIB);
	ReleaseCOM(mTreeVB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mTreeLayout);
	ReleaseCOM(mVS);
	ReleaseCOM(mPS);
	ReleaseCOM(mBillVS);
	ReleaseCOM(mBillGS);
	ReleaseCOM(mBillPS);
	ReleaseCOM(mPerObjectBuffer);
	ReleaseCOM(mPerFrameBuffer);
	ReleaseCOM(mLandTexture);
	ReleaseCOM(mWaterTexture);
	ReleaseCOM(mCrateTexture);
	ReleaseCOM(mTreeTextureArray);
	ReleaseCOM(mSampleState);
	ReleaseCOM(mNoCullRS);
	ReleaseCOM(mTransparentBlend);
	ReleaseCOM(mAlphaToCoverageBS);
}

bool TreeBillboard::Init()
{
	if (!D3DApp::Init())
		return false;

	mWaves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWavesGeometryBuffers();
	BuildCrateGeometryBuffers();
	BuildTreeGeometryBuffers();
	BuildFX();
	BuildTex();

	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&noCullDesc, &mNoCullRS));

	D3D11_BLEND_DESC blendDesc = {0};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(md3dDevice->CreateBlendState(&blendDesc, &mTransparentBlend));

	D3D11_BLEND_DESC a2CDesc = {0};
	a2CDesc.AlphaToCoverageEnable = true;
	a2CDesc.IndependentBlendEnable = false;
	a2CDesc.RenderTarget[0].BlendEnable = false;
	a2CDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(md3dDevice->CreateBlendState(&a2CDesc, &mAlphaToCoverageBS));

	return true;
}

void TreeBillboard::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void TreeBillboard::UpdateScene(float dt)
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

void TreeBillboard::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

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

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	// Trees
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(mTreeLayout);
	UINT stride = sizeof(TreeVertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mTreeVB, &stride, &offset);

	// Set constants
	HR(md3dImmediateContext->Map(mTreeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PerTreeBuffer* treeDataPtr = (PerTreeBuffer*)mappedResource.pData;
	treeDataPtr->ViewProj = XMMatrixTranspose(view * proj);
	treeDataPtr->Mat = mTreeMat;

	md3dImmediateContext->Unmap(mTreeBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mTreeBuffer);
	md3dImmediateContext->GSSetConstantBuffers(1, 1, &mTreeBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mTreeBuffer);

	md3dImmediateContext->GSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	md3dImmediateContext->PSSetShaderResources(0, 1, &mTreeTextureArray);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

	md3dImmediateContext->VSSetShader(mBillVS, 0, 0);
	md3dImmediateContext->GSSetShader(mBillGS, 0, 0);
	md3dImmediateContext->PSSetShader(mBillPS, 0, 0);

	float blendFactors[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	md3dImmediateContext->OMSetBlendState(mAlphaToCoverageBS, blendFactors, 0xffffffff);
	md3dImmediateContext->Draw(TreeCount, 0);
	md3dImmediateContext->OMSetBlendState(0, blendFactors, 0xffffffff);

	md3dImmediateContext->GSSetShader(0, 0, 0);

	// LAND
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	stride = sizeof(Vertex);
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

	md3dImmediateContext->OMSetBlendState(mTransparentBlend, blendFactors, 0xffffffff);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

	md3dImmediateContext->OMSetBlendState(0, blendFactors, 0xffffffff);


	HR(mSwapChain->Present(0, 0));
}

void TreeBillboard::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void TreeBillboard::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TreeBillboard::OnMouseMove(WPARAM btnState, int x, int y)
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

float TreeBillboard::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 TreeBillboard::GetHillNormal(float x, float z) const
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

void TreeBillboard::BuildLandGeometryBuffers()
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

void TreeBillboard::BuildWavesGeometryBuffers()
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

void TreeBillboard::BuildCrateGeometryBuffers()
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

void TreeBillboard::BuildTreeGeometryBuffers()
{
	std::vector<TreeVertex> vertices(TreeCount);

	for (UINT i = 0; i < vertices.size(); ++i)
	{
		float x = MathHelper::RandF(-35.0f, 35.0f);
		float z = MathHelper::RandF(-35.0f, 35.0f);
		float y = GetHeight(x, z);

		// Move tree slightly above land height.
		y += 10.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(24.0f, 24.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TreeVertex) * TreeCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices.data();
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mTreeVB));
}

void TreeBillboard::BuildFX()
{
	// Create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC treeVertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D_SHADER_MACRO basicEffectDefines[] = {
		{ "CLIP", "1" },
	//	{"FOG", "1"},
		{ 0, 0 }
	};

	CreateShader(&mVS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "VS", 0, &mInputLayout, vertexDesc, 3);
	CreateShader(&mPS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "PS", basicEffectDefines);

	CreateShader(&mBillVS, ExePath().append(L"../../../Shaders/Billboard.hlsl").c_str(), "VS", 0, &mTreeLayout, treeVertexDesc, 2);
	CreateShader(&mBillGS, ExePath().append(L"../../../Shaders/Billboard.hlsl").c_str(), "GS", 0);
	CreateShader(&mBillPS, ExePath().append(L"../../../Shaders/Billboard.hlsl").c_str(), "PS", basicEffectDefines);

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

	matrixBufferDesc.ByteWidth = sizeof(PerTreeBuffer);

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mTreeBuffer));
}

void TreeBillboard::BuildTex()
{
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/grass.dds").c_str(), nullptr, &mLandTexture));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/water1.dds").c_str(), nullptr, &mWaterTexture));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/WireFence.dds").c_str(), nullptr, &mCrateTexture));

	// Create tree texture array
	std::vector<std::wstring> filenames =
	{
		ExePath().append(L"../../../Textures/tree0.dds"),
		ExePath().append(L"../../../Textures/tree1.dds"),
		ExePath().append(L"../../../Textures/tree2.dds"),
		ExePath().append(L"../../../Textures/tree3.dds")
	};

	std::vector<ID3D11Texture2D*> srcTex(filenames.size());
	for (UINT i = 0; i < filenames.size(); ++i)
	{
		HR(CreateDDSTextureFromFileEx(md3dDevice, filenames[i].c_str(), 
			0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0, false, 
			(ID3D11Resource**)&srcTex[i], 0));
	}

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[1]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = filenames.size();
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;

	ID3D11Texture2D* texArray = 0;
	HR(md3dDevice->CreateTexture2D(&texArrayDesc, 0, &texArray));

	for (UINT texElement = 0; texElement < filenames.size(); ++texElement)
	{
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HR(md3dImmediateContext->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

			md3dImmediateContext->UpdateSubresource(
				texArray,
				D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0,
				mappedTex2D.pData,
				mappedTex2D.RowPitch,
				mappedTex2D.DepthPitch);

			md3dImmediateContext->Unmap(srcTex[texElement], mipLevel);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = filenames.size();

	HR(md3dDevice->CreateShaderResourceView(texArray, &viewDesc, &mTreeTextureArray));

	ReleaseCOM(texArray);

	for (UINT i = 0; i < filenames.size(); ++i)
		ReleaseCOM(srcTex[i]);


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
}