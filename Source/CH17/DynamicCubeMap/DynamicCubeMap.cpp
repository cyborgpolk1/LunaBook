#include "DynamicCubeMap.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include <vector>
#include <fstream>
#include <string>

D3DMAIN(DynamicCubeMap);

DynamicCubeMap::DynamicCubeMap(HINSTANCE hInstance)
	: D3DApp(hInstance), mShapesVB(0), mShapesIB(0), mShapeInputLayout(0), mTexVS(0), mPerObjectBuffer(0), mPerFrameBuffer(0),
	mBoxVertexOffset(0), mGridVertexOffset(0), mSphereVertexOffset(0), mCylinderVertexOffset(0),
	mBoxIndexCount(0), mGridIndexCount(0), mSphereIndexCount(0), mCylinderIndexCount(0), mSkullIndexCount(0),
	mBoxIndexOffset(0), mGridIndexOffset(0), mSphereIndexOffset(0), mCylinderIndexOffset(0),
    mLightCount(0), mSkullVB(0), mSkullIB(0), mSky(0), mDynamicCubeMapDSV(0), mDynamicCubeMapSRV(0)
{
	mMainWndCaption = L"Dynamic Cube Map Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	for (int i = 0; i < 4; ++i)
	{
		mTexPS[i] = 0;
	}
		
    mCamera.SetPosition(0.0f, 2.0f, -15.0f);

    BuildCubeFaceCamera(0.0f, 2.0f, 0.0f);

    for (UINT i = 0; i < 6; ++i)
    {
        mDynamicCubeMapRTV[i] = 0;
    }

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
    XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
    XMStoreFloat4x4(&mCenterSphereWorld, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	// We create 5 rows of 2 cylinders and spheres per row
	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(5.0f, 3.5f, -10.0f + i*5.0f));
	}

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
    mSphereMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mSkullMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mCenterSphereMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mCenterSphereMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mCenterSphereMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mCenterSphereMat.Reflect = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
}

DynamicCubeMap::~DynamicCubeMap()
{
	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mShapeInputLayout);
	ReleaseCOM(mTexVS);
	for (int i = 0; i < 4; ++i)
	{
		ReleaseCOM(mTexPS[i]);
	}
	ReleaseCOM(mPerFrameBuffer);
	ReleaseCOM(mPerObjectBuffer);
	ReleaseCOM(mFloorTex);
	ReleaseCOM(mBrickTex);
	ReleaseCOM(mStoneTex);
	ReleaseCOM(mSampleState);
    SafeDelete(mSky);
    ReleaseCOM(mDynamicCubeMapDSV);
    ReleaseCOM(mDynamicCubeMapSRV);
    for (UINT i = 0; i < 6; ++i)
    {
        ReleaseCOM(mDynamicCubeMapRTV[i]);
    }
}

bool DynamicCubeMap::Init()
{
	if (!D3DApp::Init())
		return false;

    BuildDynamicCubeMapViews();
	BuildShapeGeometryBuffer();
	BuildSkullGeometryBuffer();
	BuildFX();
	BuildTex();

    mSky = new Sky(md3dDevice, ExePath().append(L"../../../Textures/grasscube1024.dds"), 5000.0f);

	return true;
}

void DynamicCubeMap::OnResize()
{
	D3DApp::OnResize();

    mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void DynamicCubeMap::UpdateScene(float dt)
{
	// Camera control
    if (GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(10.0f*dt);

    if (GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-10.0f*dt);

    if (GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-10.0f*dt);

    if (GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(10.0f*dt);

    mCamera.UpdateViewMatrix();

    // Light switches
	if (GetAsyncKeyState('0') & 0x8000)
		mLightCount = 0;

	if (GetAsyncKeyState('1') & 0x8000)
		mLightCount = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		mLightCount = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		mLightCount = 3;

    // Animate the skull around the center sphere.
    XMMATRIX skullScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
    XMMATRIX skullOffset = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
    XMMATRIX skullLocalRotate = XMMatrixRotationY(2.0f * mTimer.TotalTime());
    XMMATRIX skullGlobalRotate = XMMatrixRotationY(0.5f * mTimer.TotalTime());
    XMStoreFloat4x4(&mSkullWorld, skullScale * skullLocalRotate * skullOffset * skullGlobalRotate);
}

void DynamicCubeMap::DrawScene()
{
    ID3D11RenderTargetView* renderTargets[1];

    // Generate the cube map bu rendering to each cube map face.
    md3dImmediateContext->RSSetViewports(1, &mCubeMapViewport);
    for (UINT i = 0; i < 6; ++i)
    {
        md3dImmediateContext->ClearRenderTargetView(mDynamicCubeMapRTV[i], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
        md3dImmediateContext->ClearDepthStencilView(mDynamicCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Bind cube mape face as render target.
        renderTargets[0] = mDynamicCubeMapRTV[i];
        md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDynamicCubeMapDSV);

        // Draw the scene with the exception of the 
        // center sphere to this cube map face.
        DrawScene(mCubeMapCamera[i], false);
    }

    // Restore old viewport and render targets.
    md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
    renderTargets[0] = mRenderTargetView;
    md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

    // Have hardware generate lower mipmap levels of cube map.
    md3dImmediateContext->GenerateMips(mDynamicCubeMapSRV);

    // Now draw the scene as normal, but with the center sphere.
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    DrawScene(mCamera, true);

    HR(mSwapChain->Present(0, 0));
}

void DynamicCubeMap::DrawScene(const Camera& camera, bool drawCenterSphere)
{
	md3dImmediateContext->IASetInputLayout(mShapeInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(TexVertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
	md3dImmediateContext->PSSetShader(mTexPS[mLightCount], 0, 0);

    XMMATRIX view = camera.View();
    XMMATRIX proj = camera.Proj();
    XMMATRIX viewProj = camera.ViewProj();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	PerFrameBuffer* frameDataPtr = (PerFrameBuffer*)mappedResource.pData;
	frameDataPtr->EyePosW = camera.GetPosition();
	frameDataPtr->DirLights[0] = mDirLights[0];
	frameDataPtr->DirLights[1] = mDirLights[1];
	frameDataPtr->DirLights[2] = mDirLights[2];

	md3dImmediateContext->Unmap(mPerFrameBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	// Draw the grid
	XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	PerObjectBuffer* dataPtr = (PerObjectBuffer*)mappedResource.pData;
	dataPtr->World = XMMatrixTranspose(world);
	dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
	dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	dataPtr->gTexTransform = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	dataPtr->Mat = mGridMat;
    dataPtr->Options = USE_TEXTURES;
	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetShaderResources(0, 1, &mFloorTex);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
	md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	// Draw the box
	world = XMLoadFloat4x4(&mBoxWorld);
	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	dataPtr = (PerObjectBuffer*)mappedResource.pData;
	dataPtr->World = XMMatrixTranspose(world);
	dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
	dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	dataPtr->gTexTransform = XMMatrixIdentity();
	dataPtr->Mat = mBoxMat;
    dataPtr->Options = USE_TEXTURES;
	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
	md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
	md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	// Draw the cylinders
	for (int i = 0; i < 10; ++i)
	{
		world = XMLoadFloat4x4(&mCylWorld[i]);
		HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		dataPtr = (PerObjectBuffer*)mappedResource.pData;
		dataPtr->World = XMMatrixTranspose(world);
		dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
		dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
		dataPtr->gTexTransform = XMMatrixIdentity();
		dataPtr->Mat = mCylinderMat;
        dataPtr->Options = USE_TEXTURES;
		md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
		md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
		md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
		md3dImmediateContext->PSSetShaderResources(0, 1, &mBrickTex);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
		md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
	}

	// Draw the spheres
	for (int i = 0; i < 10; ++i)
	{
		world = XMLoadFloat4x4(&mSphereWorld[i]);
		HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		dataPtr = (PerObjectBuffer*)mappedResource.pData;
		dataPtr->World = XMMatrixTranspose(world);
		dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
		dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
		dataPtr->gTexTransform = XMMatrixIdentity();
		dataPtr->Mat = mSphereMat;
        dataPtr->Options = USE_TEXTURES;
		md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
		md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
		md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
		md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
	}

    if (drawCenterSphere)
    {
        world = XMLoadFloat4x4(&mCenterSphereWorld);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        dataPtr->Mat = mCenterSphereMat;
        dataPtr->Options = USE_TEXTURES | USE_ENV_MAPPING;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
        md3dImmediateContext->PSSetShaderResources(1, 1, &mDynamicCubeMapSRV);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

        // Render target needs to write to this resource, so unbind.
        ID3D11ShaderResourceView* nullSRV = 0;
        md3dImmediateContext->PSSetShaderResources(1, 1, &nullSRV);
    }

	md3dImmediateContext->IASetInputLayout(mShapeInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	stride = sizeof(BasicVertex);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

	// Draw skull
	world = XMLoadFloat4x4(&mSkullWorld);
	HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	dataPtr = (PerObjectBuffer*)mappedResource.pData;
	dataPtr->World = XMMatrixTranspose(world);
	dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
	dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
	dataPtr->gTexTransform = XMMatrixIdentity();
	dataPtr->Mat = mSkullMat;
    dataPtr->Options = STANDARD_LIGHTING;
	md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
	md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

    mSky->Draw(md3dImmediateContext, camera);
}

void DynamicCubeMap::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void DynamicCubeMap::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DynamicCubeMap::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mCamera.RotateY(dx);
        mCamera.Pitch(dy);
	}
    else if ((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));

        mCamera.Roll(dx);
    }

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void DynamicCubeMap::BuildShapeGeometryBuffer()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;
	mGridVertexOffset = (UINT)box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + (UINT)grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount = (UINT)box.Indices.size();
	mGridIndexCount = (UINT)grid.Indices.size();
	mSphereIndexCount = (UINT)sphere.Indices.size();
	mCylinderIndexCount = (UINT)cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount = (UINT)(box.Vertices.size() + grid.Vertices.size() + sphere.Vertices.size() + cylinder.Vertices.size());

	UINT totalIndexCount = mBoxIndexCount + mGridIndexCount + mSphereIndexCount + mCylinderIndexCount;

	// Extract the vertex elements we are interested in and pack all
	// the vertices of all meshes into one vertex buffer
	std::vector<TexVertex> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TexVertex) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	// Pack the indices of all the meshes into one index buffer.
	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &initData, &mShapesIB));
}

void DynamicCubeMap::BuildSkullGeometryBuffer()
{
	// Load skull
	std::ifstream fin(ExePath().append(L"../../../Models/skull.txt").c_str());

	if (!fin)
	{
		MessageBox(0, L"skull.txt not found.", 0, 0);
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

void DynamicCubeMap::BuildFX()
{
	// Create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D_SHADER_MACRO basicEffectDefines[] = {
		{ "NUM_LIGHTS", "0" },
		{ 0, 0 }
	};

	ShaderHelper::CreateShader(md3dDevice, &mTexVS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "VS", 0, &mShapeInputLayout, vertexDesc, 3);

	for (int i = 0; i < 4; ++i)
	{
		std::string lightMacro = std::to_string(i);
		basicEffectDefines[0] = { "NUM_LIGHTS", lightMacro.c_str() };
		ShaderHelper::CreateShader(md3dDevice, &mTexPS[i], ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "PS", basicEffectDefines);
	}

	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.ByteWidth = sizeof(PerObjectBuffer);
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerObjectBuffer));

	D3D11_BUFFER_DESC matrixBufferDesc2;
	matrixBufferDesc2.ByteWidth = sizeof(PerFrameBuffer);
	matrixBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc2.MiscFlags = 0;
	matrixBufferDesc2.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc2, 0, &mPerFrameBuffer));
}

void DynamicCubeMap::BuildTex() 
{
	ID3D11Resource *floorResource, *brickResource, *stoneResource;

	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/floor.dds").c_str(), &floorResource, &mFloorTex));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/bricks.dds").c_str(), &brickResource, &mBrickTex));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/stone.dds").c_str(), &stoneResource, &mStoneTex));

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
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
	ReleaseCOM(stoneResource);
}

void DynamicCubeMap::BuildCubeFaceCamera(float x, float y, float z)
{
    // Generate the cube map about the given position.
    XMFLOAT3 center(x, y, z);
    XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

    // Look along each coordinate axis.
    XMFLOAT3 targets[6] = 
    {
        XMFLOAT3(x + 1.0f, y, z), // +X
        XMFLOAT3(x - 1.0f, y, z), // -X
        XMFLOAT3(x, y + 1.0f, z), // +Y
        XMFLOAT3(x, y - 1.0f, z), // -Y
        XMFLOAT3(x, y, z + 1.0f), // +Z
        XMFLOAT3(x, y, z - 1.0f)  // -Z
    };

    // Use world up vector (0, 1, 0) for all directions except +Y/-Y.
    // In these cases, we are looking down +Y or -Y, so we need a
    // different "up" vector.
    XMFLOAT3 ups[6] =
    {
        XMFLOAT3(0.0f, 1.0f, 0.0f), // +X
        XMFLOAT3(0.0f, 1.0f, 0.0f), // -X
        XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
        XMFLOAT3(0.0f, 0.0f, 1.0f), // -Y
        XMFLOAT3(0.0f, 1.0f, 0.0f), // +Z
        XMFLOAT3(0.0f, 1.0f, 0.0f)  // -Z
    };

    for (int i = 0; i < 6; ++i)
    {
        mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
        mCubeMapCamera[i].SetLens(0.5f * MathHelper::Pi, 1.0f, 0.1f, 1000.0f);
        mCubeMapCamera[i].UpdateViewMatrix();
    }
}

void DynamicCubeMap::BuildDynamicCubeMapViews()
{
    //
    // Cubemap is a special texture array with 6 elements.
    // We bind this as a render target to draw to the cube
    // faces, and also as a shader resource, so we can use
    // it in a pixel shader.
    //
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = CubeMapSize;
    texDesc.Height = CubeMapSize;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 6;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* cubeTex = 0;
    HR(md3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex));

    //
    // Create a render target view to each cube map face
    // (i.e., each element in texture array).
    //
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.MipSlice = 0;

    // Only create a view to one array element.
    rtvDesc.Texture2DArray.ArraySize = 1;

    for (UINT i = 0; i < 6; ++i)
    {
        // Create a render target view to the ith element.
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        HR(md3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTV[i]));
    }

    //
    // Create a shader resource view to the cube map.
    //
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = -1;

    HR(md3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRV));

    ReleaseCOM(cubeTex);

    //
    // Create the depth stencil view.
    //
    D3D11_TEXTURE2D_DESC depthTexDesc;
    depthTexDesc.Width = CubeMapSize;
    depthTexDesc.Height = CubeMapSize;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    ID3D11Texture2D* depthTex = 0;
    HR(md3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex));

    // Create the depth stencil view for the entire buffer.
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = depthTexDesc.Format;
    dsvDesc.Flags = 0;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    HR(md3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV));

    ReleaseCOM(depthTex);

    //
    // Create viewport.
    //
    mCubeMapViewport.TopLeftX = 0.0f;
    mCubeMapViewport.TopLeftY = 0.0f;
    mCubeMapViewport.Width = (float)CubeMapSize;
    mCubeMapViewport.Height = (float)CubeMapSize;
    mCubeMapViewport.MinDepth = 0.0f;
    mCubeMapViewport.MaxDepth = 1.0f;
}