#include "MeshViewer.h"
#include "MathHelper.h"

D3DMAIN(MeshViewer);

MeshViewer::MeshViewer(HINSTANCE hInstance)
    : D3DApp(hInstance), mSky(0), mTreeModel(0), mBaseModel(0), mStairsModel(0),
    mPillar1Model(0), mPillar2Model(0), mPillar3Model(0), mPillar4Model(0), 
    mRockModel(0), mSmap(0), mLightRotationAngle(0.0f), mInputLayout(0),
    mNormalVS(0), mNormalPS(0), mBuildVS(0), mBuildPS(0), mPerFrameBuffer(0),
    mPerObjectBuffer(0), mSampleState(0), mComparisonState(0), mShadowRS(0)
{
    mMainWndCaption = L"Mesh Viewer Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCamera.SetPosition(0.0f, 2.0f, -15.0f);

    mDirLights[0].Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.7f, 0.6f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.7f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);

    mOriginalLightDir[0] = mDirLights[0].Direction;
    mOriginalLightDir[1] = mDirLights[1].Direction;
    mOriginalLightDir[2] = mDirLights[2].Direction;
}

MeshViewer::~MeshViewer()
{
    SafeDelete(mSky);
    SafeDelete(mTreeModel);
    SafeDelete(mBaseModel);
    SafeDelete(mStairsModel);
    SafeDelete(mPillar1Model);
    SafeDelete(mPillar2Model);
    SafeDelete(mPillar3Model);
    SafeDelete(mPillar4Model);
    SafeDelete(mRockModel);
    SafeDelete(mSmap);

    ReleaseCOM(mInputLayout);
    ReleaseCOM(mNormalVS);
    ReleaseCOM(mNormalPS);
    ReleaseCOM(mBuildVS);
    ReleaseCOM(mBuildPS);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mPerObjectBuffer);
    ReleaseCOM(mSampleState);
    ReleaseCOM(mComparisonState);
    ReleaseCOM(mShadowRS);
}

bool MeshViewer::Init()
{
    if (!D3DApp::Init())
        return false;

    mTexMgr.Init(md3dDevice);

    auto textureDir = ExePath().append(L"../../../Textures/");

    mSky = new Sky(md3dDevice, (textureDir +  L"desertcube1024.dds").c_str(), 5000.0f);
    mSmap = new ShadowMap(md3dDevice, SMapSize, SMapSize);

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

    auto modelDir = ExePath().append(L"../../../Models/");

    mTreeModel = new BasicModel(md3dDevice, mTexMgr, modelDir + L"tree.m3d", textureDir);
    mBaseModel = new BasicModel(md3dDevice, mTexMgr, modelDir + L"base.m3d", textureDir);
    mStairsModel = new BasicModel(md3dDevice, mTexMgr, modelDir + L"stairs.m3d", textureDir);
    mPillar1Model = new BasicModel(md3dDevice, mTexMgr, modelDir + L"pillar1.m3d", textureDir);
    mPillar2Model = new BasicModel(md3dDevice, mTexMgr, modelDir + L"pillar2.m3d", textureDir);
    mPillar3Model = new BasicModel(md3dDevice, mTexMgr, modelDir + L"pillar5.m3d", textureDir);
    mPillar4Model = new BasicModel(md3dDevice, mTexMgr, modelDir + L"pillar6.m3d", textureDir);
    mRockModel = new BasicModel(md3dDevice, mTexMgr, modelDir + L"rock.m3d", textureDir);

    BasicModelInstance treeInstance;
    BasicModelInstance baseInstance;
    BasicModelInstance stairsInstance;
    BasicModelInstance pillar1Instance;
    BasicModelInstance pillar2Instance;
    BasicModelInstance pillar3Instance;
    BasicModelInstance pillar4Instance;
    BasicModelInstance rockInstance1;
    BasicModelInstance rockInstance2;
    BasicModelInstance rockInstance3;

    treeInstance.Model = mTreeModel;
    baseInstance.Model = mBaseModel;
    stairsInstance.Model = mStairsModel;
    pillar1Instance.Model = mPillar1Model;
    pillar2Instance.Model = mPillar2Model;
    pillar3Instance.Model = mPillar3Model;
    pillar4Instance.Model = mPillar4Model;
    rockInstance1.Model = mRockModel;
    rockInstance2.Model = mRockModel;
    rockInstance3.Model = mRockModel;

    XMMATRIX modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
    XMMATRIX modelRot = XMMatrixRotationY(0.0f);
    XMMATRIX modelOffset = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    XMStoreFloat4x4(&treeInstance.World, modelScale * modelRot * modelOffset);
    XMStoreFloat4x4(&baseInstance.World, modelScale * modelRot * modelOffset);
    
    modelRot = XMMatrixRotationY(0.5f * XM_PI);
    modelOffset = XMMatrixTranslation(0.0f, -2.5f, -12.0f);
    XMStoreFloat4x4(&stairsInstance.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
    modelOffset = XMMatrixTranslation(-5.0f, 1.5f, 5.0f);
    XMStoreFloat4x4(&pillar1Instance.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
    modelOffset = XMMatrixTranslation(5.0f, 1.5f, 5.0f);
    XMStoreFloat4x4(&pillar2Instance.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
    modelOffset = XMMatrixTranslation(5.0f, 1.5f, -5.0f);
    XMStoreFloat4x4(&pillar3Instance.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
    modelOffset = XMMatrixTranslation(-5.0f, 1.0f, -5.0f);
    XMStoreFloat4x4(&pillar4Instance.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
    modelOffset = XMMatrixTranslation(-1.0f, 1.4f, -7.0f);
    XMStoreFloat4x4(&rockInstance1.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
    modelOffset = XMMatrixTranslation(5.0f, 1.2f, -2.0f);
    XMStoreFloat4x4(&rockInstance2.World, modelScale * modelRot * modelOffset);

    modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
    modelOffset = XMMatrixTranslation(-4.0f, 1.3f, 3.0f);
    XMStoreFloat4x4(&rockInstance3.World, modelScale * modelRot * modelOffset);

    mAlphaClippedModelInstances.push_back(treeInstance);

    mModelInstances.push_back(baseInstance);
    mModelInstances.push_back(stairsInstance);
    mModelInstances.push_back(pillar1Instance);
    mModelInstances.push_back(pillar2Instance);
    mModelInstances.push_back(pillar3Instance);
    mModelInstances.push_back(pillar4Instance);
    mModelInstances.push_back(rockInstance1);
    mModelInstances.push_back(rockInstance2);
    mModelInstances.push_back(rockInstance3);

    //
    // Compute scene bounding box.
    //

    XMFLOAT3 minPt(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
    XMFLOAT3 maxPt(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
    for (UINT i = 0; i < mModelInstances.size(); ++i)
    {
        for (UINT j = 0; j < mModelInstances[i].Model->Vertices.size(); ++j)
        {
            XMFLOAT3 P = mModelInstances[i].Model->Vertices[j].Pos;

            minPt.x = MathHelper::Min(minPt.x, P.x);
            minPt.y = MathHelper::Min(minPt.y, P.y);
            minPt.z = MathHelper::Min(minPt.z, P.z);

            maxPt.x = MathHelper::Max(maxPt.x, P.x);
            maxPt.y = MathHelper::Max(maxPt.y, P.y);
            maxPt.z = MathHelper::Max(maxPt.z, P.z);
        }
    }

    //
    // Derive scene bounding sphere from bounding box.
    //
    mSceneBounds.Center = XMFLOAT3(
        0.5f * (minPt.x + maxPt.x),
        0.5f * (minPt.y + maxPt.y),
        0.5f * (minPt.z + maxPt.z));

    XMFLOAT3 extent(
        0.5f * (maxPt.x - minPt.x),
        0.5f * (maxPt.y - minPt.y),
        0.5f * (maxPt.z - minPt.z));

    mSceneBounds.Radius = sqrtf(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z);

    BuildFX();

    return true;
}

void MeshViewer::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void MeshViewer::UpdateScene(float dt)
{
    if (GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(10.0f * dt);

    if (GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-10.0f * dt);

    if (GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-10.0f * dt);

    if (GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(10.0f * dt);

    BuildShadowTransform();
    
    mCamera.UpdateViewMatrix();
}

void MeshViewer::DrawScene()
{
    mSmap->BindDsvAndSetNullRenderTarget(md3dImmediateContext);

    DrawSceneToShadowMap();

    md3dImmediateContext->RSSetState(0);

    // Restore the back and depth buffer to the OM stage.
    ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
    md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
    md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

    ID3D11ShaderResourceView* shadowMap = mSmap->DepthMapSRV();

    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    md3dImmediateContext->VSSetShader(mNormalVS, 0, 0);
    md3dImmediateContext->PSSetShader(mNormalPS, 0, 0);

    XMMATRIX view = mCamera.View();
    XMMATRIX proj = mCamera.Proj();
    XMMATRIX viewProj = mCamera.ViewProj();

    XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* framePtr = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);
    framePtr->EyePosW = mCamera.GetPosition();
    framePtr->DirLights[0] = mDirLights[0];
    framePtr->DirLights[1] = mDirLights[1];
    framePtr->DirLights[2] = mDirLights[2];
    framePtr->ViewProj = XMMatrixTranspose(viewProj);

    md3dImmediateContext->Unmap(mPerFrameBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->PSSetSamplers(1, 1, &mComparisonState);

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    
    for (UINT modelIndex = 0; modelIndex < mModelInstances.size(); ++modelIndex)
    {
        world = XMLoadFloat4x4(&mModelInstances[modelIndex].World);
        worldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);

        const BasicModelInstance& currentInstance = mModelInstances[modelIndex];
        for (UINT subset = 0; subset < currentInstance.Model->subsetCount; ++subset)
        {
            HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

            PerObjectBuffer* objPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
            objPtr->World = XMMatrixTranspose(world);
            objPtr->WorldInvTranspose = worldInvTranspose;
            objPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f));
            objPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
            objPtr->Mat = currentInstance.Model->Mat[subset];
            objPtr->Options = USE_TEXTURES | USE_NORMAL_MAP;

            md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
            md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
            md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

            md3dImmediateContext->PSSetShaderResources(0, 1, &(currentInstance.Model->DiffuseMapSRV[subset]));
            md3dImmediateContext->PSSetShaderResources(2, 1, &(currentInstance.Model->NormalMapSRV[subset]));
            md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
            currentInstance.Model->ModelMesh.Draw(md3dImmediateContext, subset);
        }
    }

    for (UINT modelIndex = 0; modelIndex < mAlphaClippedModelInstances.size(); ++modelIndex)
    {
        world = XMLoadFloat4x4(&mAlphaClippedModelInstances[modelIndex].World);
        worldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);

        const BasicModelInstance& currentInstance = mAlphaClippedModelInstances[modelIndex];
        for (UINT subset = 0; subset < currentInstance.Model->subsetCount; ++subset)
        {
            HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

            PerObjectBuffer* objPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
            objPtr->World = XMMatrixTranspose(world);
            objPtr->WorldInvTranspose = worldInvTranspose;
            objPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f));
            objPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
            objPtr->Mat = currentInstance.Model->Mat[subset];
            objPtr->Options = USE_TEXTURES | USE_NORMAL_MAP | USE_ALPHA_CLIPPING;

            md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
            md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
            md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

            md3dImmediateContext->PSSetShaderResources(0, 1, &(currentInstance.Model->DiffuseMapSRV[subset]));
            md3dImmediateContext->PSSetShaderResources(2, 1, &(currentInstance.Model->NormalMapSRV[subset]));
            md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
            currentInstance.Model->ModelMesh.Draw(md3dImmediateContext, subset);
        }
    }

    ID3D11ShaderResourceView* nullSRV = { 0 };
    md3dImmediateContext->PSSetShaderResources(0, 1, &nullSRV);
    md3dImmediateContext->PSSetShaderResources(2, 1, &nullSRV);
    md3dImmediateContext->PSSetShaderResources(3, 1, &nullSRV);

    mSky->Draw(md3dImmediateContext, mCamera);

    md3dImmediateContext->RSSetState(0);

    HR(mSwapChain->Present(0, 0));
}

void MeshViewer::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void MeshViewer::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void MeshViewer::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mCamera.Pitch(dy);
        mCamera.RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void MeshViewer::DrawSceneToShadowMap()
{
    md3dImmediateContext->RSSetState(mShadowRS);

    XMMATRIX view = XMLoadFloat4x4(&mLightView);
    XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    // Ignore tessellation values since we won't be using displacement mapping in this demo
    PerFrameBuffer* framePtr = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);
    framePtr->EyePosW = mCamera.GetPosition();
    framePtr->ViewProj = XMMatrixTranspose(viewProj);

    md3dImmediateContext->Unmap(mPerFrameBuffer, 0);

    md3dImmediateContext->VSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    md3dImmediateContext->IASetInputLayout(mInputLayout);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    md3dImmediateContext->VSSetShader(mBuildVS, 0, 0);
    md3dImmediateContext->PSSetShader(mBuildPS, 0, 0);

    XMMATRIX world;
    XMMATRIX worldInvTranspose;

    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

    for (UINT modelIndex = 0; modelIndex < mModelInstances.size(); ++modelIndex)
    {
        world = XMLoadFloat4x4(&mModelInstances[modelIndex].World);
        worldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);

        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

        PerObjectBuffer* objPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
        objPtr->World = XMMatrixTranspose(world);
        objPtr->WorldInvTranspose = worldInvTranspose;
        objPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f));

        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);

        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);

        const BasicModelInstance& currentInstance = mModelInstances[modelIndex];
        for (UINT subset = 0; subset < currentInstance.Model->subsetCount; ++subset)
        {
            md3dImmediateContext->PSSetShaderResources(0, 1, &(currentInstance.Model->DiffuseMapSRV[subset]));
            // No tessellation. Don't set normal map.
            currentInstance.Model->ModelMesh.Draw(md3dImmediateContext, subset);
        }
    }

    for (UINT modelIndex = 0; modelIndex < mAlphaClippedModelInstances.size(); ++modelIndex)
    {
        world = XMLoadFloat4x4(&mAlphaClippedModelInstances[modelIndex].World);
        worldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);

        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

        PerObjectBuffer* objPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
        objPtr->World = XMMatrixTranspose(world);
        objPtr->WorldInvTranspose = worldInvTranspose;
        objPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 1.0f));

        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);

        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);

        const BasicModelInstance& currentInstance = mAlphaClippedModelInstances[modelIndex];
        for (UINT subset = 0; subset < currentInstance.Model->subsetCount; ++subset)
        {
            md3dImmediateContext->PSSetShaderResources(0, 1, &(currentInstance.Model->DiffuseMapSRV[subset]));
            // No tessellation. Don't set normal map.
            currentInstance.Model->ModelMesh.Draw(md3dImmediateContext, subset);
        }
    }
}

void MeshViewer::BuildShadowTransform()
{
    // Only the first "main" light casts a shadow.
    XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
    XMVECTOR lightPos = -2.0f * mSceneBounds.Radius * lightDir;
    XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

    // Transform bounding sphere to light space.
    XMFLOAT3 sphereCenterLS;
    XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - mSceneBounds.Radius;
    float b = sphereCenterLS.y - mSceneBounds.Radius;
    float n = sphereCenterLS.z - mSceneBounds.Radius;
    float r = sphereCenterLS.x + mSceneBounds.Radius;
    float t = sphereCenterLS.y + mSceneBounds.Radius;
    float f = sphereCenterLS.z + mSceneBounds.Radius;
    XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX S = V*P*T;

    XMStoreFloat4x4(&mLightView, V);
    XMStoreFloat4x4(&mLightProj, P);
    XMStoreFloat4x4(&mShadowTransform, S);
}

void MeshViewer::BuildFX()
{
    // Create the vertex input layout
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    D3D_SHADER_MACRO basicEffectDefines[] = {
        {"NUM_LIGHTS", "3"},
        {0,0}
    };

    auto ShadowMapFilename = ExePath().append(L"../../../Shaders/ShadowMap.hlsl");
    auto ShadowMapString = ShadowMapFilename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mNormalVS, ShadowMapString, "NormalVS", 0, &mInputLayout, vertexDesc, 4);
    ShaderHelper::CreateShader(md3dDevice, &mNormalPS, ShadowMapString, "PS", basicEffectDefines);

    auto BuildMapFilename = ExePath().append(L"../../../Shaders/BuildShadowMap.hlsl");
    auto BuildMapString = BuildMapFilename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mBuildVS, BuildMapString, "VS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mBuildPS, BuildMapString, "PS", 0);

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    HR(md3dDevice->CreateBuffer(&bufferDesc, 0, &mPerFrameBuffer));

    bufferDesc.ByteWidth = sizeof(PerObjectBuffer);

    HR(md3dDevice->CreateBuffer(&bufferDesc, 0, &mPerObjectBuffer));

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

    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    HR(md3dDevice->CreateSamplerState(&samplerDesc, &mComparisonState));

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.DepthBias = 100000;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.SlopeScaledDepthBias = 1.0f;
    
    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mShadowRS));
}