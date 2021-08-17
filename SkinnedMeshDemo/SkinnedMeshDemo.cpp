#include "SkinnedMeshDemo.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include <vector>
#include <fstream>
#include <string>

D3DMAIN(SkinnedMeshDemo);

SkinnedMeshDemo::SkinnedMeshDemo(HINSTANCE hInstance)
    : D3DApp(hInstance), mShapesVB(0), mShapesIB(0), mShapeInputLayout(0), mTexVS(0), mTexPS(0),
    mBoxVertexOffset(0), mGridVertexOffset(0), mSphereVertexOffset(0), mCylinderVertexOffset(0),
    mBoxIndexCount(0), mGridIndexCount(0), mSphereIndexCount(0), mCylinderIndexCount(0), mSkullIndexCount(0),
    mBoxIndexOffset(0), mGridIndexOffset(0), mSphereIndexOffset(0), mCylinderIndexOffset(0),
    mSkullVB(0), mSkullIB(0), mSky(0), mPerObjectBuffer(0), mPerFrameBuffer(0), mNormalVS(0), mNormalPS(0),
    mFloorNormal(0), mBrickNormal(0), mStoneNormal(0), mRenderOptions(RenderOptionsNormalMap),
    mDoWireframe(false), mWireframeRS(0), mDispVS(0), mDispPS(0), mDispHS(0), mDispDS(0),
    mComparisonState(0), mSmap(0), mShadowVS(0), mShadowTessVS(0), mShadowPS(0), mShadowHS(0), mShadowDS(0),
    mLightRotationAngle(0.0f), mShadowRS(0), mQuadVB(0), mQuadIB(0), mQuadInputLayout(0), mQuadVS(0), mQuadPS(0),
    mPerQuadBuffer(0), mQuadDS(0), mSkinnedVS(0), mCharacterModel(0), mSkinnedLayout(0), mBoneTransforms(0), 
    mShadowSkinnedVS(0)
{
    mMainWndCaption = L"Shadow Map Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCamera.SetPosition(0.0f, 2.0f, -15.0f);

    // Estimate the scene bounding sphere manually since we know how the scene was constructed.
    // The grid is the "widest object" with a width of 20 and depth 30.0f, and centered at
    // the world space origin. In general, you need to loop over every world space vertex
    // position and compute the bounding sphere.
    mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mGridWorld, I);

    XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

    // We create 5 rows of 2 cylinders and spheres per row
    for (int i = 0; i < 5; ++i)
    {
        XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(5.0f, 1.5f, -10.0f + i * 5.0f));

        XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(5.0f, 3.5f, -10.0f + i * 5.0f));
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

    mOriginalLightDir[0] = mDirLights[0].Direction;
    mOriginalLightDir[1] = mDirLights[1].Direction;
    mOriginalLightDir[2] = mDirLights[2].Direction;

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
    mSphereMat.Reflect = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

    mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mSkullMat.Reflect = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

SkinnedMeshDemo::~SkinnedMeshDemo()
{
    ReleaseCOM(mShapesVB);
    ReleaseCOM(mShapesIB);
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mShapeInputLayout);
    ReleaseCOM(mTexVS);
    ReleaseCOM(mTexPS);
    ReleaseCOM(mNormalVS);
    ReleaseCOM(mNormalPS);
    ReleaseCOM(mDispVS);
    ReleaseCOM(mDispHS);
    ReleaseCOM(mDispDS);
    ReleaseCOM(mDispPS);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mPerObjectBuffer);
    ReleaseCOM(mFloorTex);
    ReleaseCOM(mBrickTex);
    ReleaseCOM(mStoneTex);
    ReleaseCOM(mFloorNormal);
    ReleaseCOM(mBrickNormal);
    ReleaseCOM(mStoneNormal);
    ReleaseCOM(mSampleState);
    SafeDelete(mSky);
    ReleaseCOM(mWireframeRS);
    ReleaseCOM(mComparisonState);
    SafeDelete(mSmap);
    ReleaseCOM(mShadowVS);
    ReleaseCOM(mShadowPS);
    ReleaseCOM(mShadowHS);
    ReleaseCOM(mShadowTessVS);
    ReleaseCOM(mShadowDS);
    ReleaseCOM(mShadowRS);
    ReleaseCOM(mQuadVB);
    ReleaseCOM(mQuadIB);
    ReleaseCOM(mQuadInputLayout);
    ReleaseCOM(mQuadVS);
    ReleaseCOM(mQuadPS);
    ReleaseCOM(mPerQuadBuffer);
    ReleaseCOM(mQuadDS);
    ReleaseCOM(mSkinnedVS);
    SafeDelete(mCharacterModel);
    ReleaseCOM(mSkinnedLayout);
    ReleaseCOM(mBoneTransforms);
    ReleaseCOM(mShadowSkinnedVS);
}

bool SkinnedMeshDemo::Init()
{
    if (!D3DApp::Init())
        return false;

    mTexMgr.Init(md3dDevice);

    BuildShapeGeometryBuffer();
    BuildSkullGeometryBuffer();
    BuildScreenQuadGeometryBuffer();
    BuildFX();
    BuildTex();

    mSky = new Sky(md3dDevice, ExePath().append(L"../../../Textures/sunsetcube1024.dds"), 5000.0f);
    mSmap = new ShadowMap(md3dDevice, SMapSize, SMapSize);

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mWireframeRS));

    rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.DepthBias = 100000;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.SlopeScaledDepthBias = 1.0f;
    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mShadowRS));

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
    dsDesc.DepthEnable = false;
    dsDesc.DepthFunc = D3D11_COMPARISON_NEVER;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.StencilEnable = false;
    HR(md3dDevice->CreateDepthStencilState(&dsDesc, &mQuadDS));

    auto texDir = ExePath().append(L"../../../Textures/");
    auto modelDir = ExePath().append(L"../../../Models/soldier.m3d");

    mCharacterModel = new SkinnedModel(md3dDevice, mTexMgr, modelDir, texDir);

    mCharacterInstance1.Model = mCharacterModel;
    mCharacterInstance1.TimePos = 0.0f;
    mCharacterInstance1.ClipName = "Take1";
    mCharacterInstance1.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());

    mCharacterInstance2.Model = mCharacterModel;
    mCharacterInstance2.TimePos = 0.0f;
    mCharacterInstance2.ClipName = "Take1";
    mCharacterInstance2.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());

    // Reflect to change coordinate system from the RHS the data was exported as.
    XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
    XMMATRIX modelRot = XMMatrixRotationY(MathHelper::Pi);
    XMMATRIX modelOffset = XMMatrixTranslation(-2.0f, 0.0f, -7.0f);
    XMStoreFloat4x4(&mCharacterInstance1.World, modelScale*modelRot*modelOffset);

    modelOffset = XMMatrixTranslation(2.0f, 0.0, -7.0f);
    XMStoreFloat4x4(&mCharacterInstance2.World, modelScale*modelRot*modelOffset);

    return true;
}

void SkinnedMeshDemo::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void SkinnedMeshDemo::UpdateScene(float dt)
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

    if (GetAsyncKeyState('1') & 0x8000)
        mDoWireframe = true;
    else
        mDoWireframe = false;

    if (GetAsyncKeyState('2') & 0x8000)
        mRenderOptions = RenderOptionsBasic;

    if (GetAsyncKeyState('3') & 0x8000)
        mRenderOptions = RenderOptionsNormalMap;

    if (GetAsyncKeyState('4') & 0x8000)
        mRenderOptions = RenderOptionsDisplacementMap;

    mCharacterInstance1.Update(dt);
    mCharacterInstance2.Update(dt);

    //
    // Animate the lights (and hence shadows).
    //
    mLightRotationAngle += 0.1f*dt;

    XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR lightDir = XMLoadFloat3(&mOriginalLightDir[i]);
        lightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&mDirLights[i].Direction, lightDir);
    }

    BuildShadowTransform();
}

void SkinnedMeshDemo::DrawScene()
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

    if (mDoWireframe)
        md3dImmediateContext->RSSetState(mWireframeRS);

    md3dImmediateContext->IASetInputLayout(mShapeInputLayout);

    int useOtherMaps = 0;

    switch (mRenderOptions)
    {
    case RenderOptionsBasic:
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
        useOtherMaps = 0;
        break;
    case RenderOptionsNormalMap:
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        md3dImmediateContext->VSSetShader(mNormalVS, 0, 0);
        useOtherMaps = USE_NORMAL_MAP;
        break;
    case RenderOptionsDisplacementMap:
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        md3dImmediateContext->VSSetShader(mDispVS, 0, 0);
        md3dImmediateContext->HSSetShader(mDispHS, 0, 0);
        md3dImmediateContext->DSSetShader(mDispDS, 0, 0);
        useOtherMaps = USE_NORMAL_MAP;
        break;
    }

    md3dImmediateContext->PSSetShader(mTexPS, 0, 0);


    UINT stride = sizeof(TexVertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    XMMATRIX view = mCamera.View();
    XMMATRIX proj = mCamera.Proj();
    XMMATRIX viewProj = mCamera.ViewProj();

    XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* frameDataPtr = (PerFrameBuffer*)mappedResource.pData;
    frameDataPtr->EyePosW = mCamera.GetPosition();
    frameDataPtr->DirLights[0] = mDirLights[0];
    frameDataPtr->DirLights[1] = mDirLights[1];
    frameDataPtr->DirLights[2] = mDirLights[2];

    frameDataPtr->HeightScale = 0.07f;
    frameDataPtr->MaxTessDistance = 1.0f;
    frameDataPtr->MinTessDistance = 25.0f;
    frameDataPtr->MinTessFactor = 1.0f;
    frameDataPtr->MaxTessFactor = 5.0f;

    frameDataPtr->ViewProj = XMMatrixTranspose(viewProj);

    md3dImmediateContext->Unmap(mPerFrameBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    md3dImmediateContext->DSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    // Draw the grid
    XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    PerObjectBuffer* dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(8.0f, 10.0f, 1.0f));
    dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
    dataPtr->Mat = mGridMat;
    dataPtr->Options = USE_TEXTURES | useOtherMaps;
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->DSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(0, 1, &mFloorTex);
    md3dImmediateContext->PSSetShaderResources(2, 1, &mFloorNormal);
    md3dImmediateContext->DSSetShaderResources(2, 1, &mFloorNormal);
    md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->PSSetSamplers(1, 1, &mComparisonState);
    md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

    // Draw the box
    world = XMLoadFloat4x4(&mBoxWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(2.0f, 1.0f, 1.0f));
    dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
    dataPtr->Mat = mBoxMat;
    dataPtr->Options = USE_TEXTURES | useOtherMaps;
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->DSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
    md3dImmediateContext->PSSetShaderResources(2, 1, &mStoneNormal);
    md3dImmediateContext->DSSetShaderResources(2, 1, &mStoneNormal);
    md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->PSSetSamplers(1, 1, &mComparisonState);
    md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

    // Draw the cylinders
    for (int i = 0; i < 10; ++i)
    {
        world = XMLoadFloat4x4(&mCylWorld[i]);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(1.0f, 2.0f, 1.0f));
        dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
        dataPtr->Mat = mCylinderMat;
        dataPtr->Options = USE_TEXTURES | useOtherMaps;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->DSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mBrickTex);
        md3dImmediateContext->PSSetShaderResources(2, 1, &mBrickNormal);
        md3dImmediateContext->DSSetShaderResources(2, 1, &mBrickNormal);
        md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->PSSetSamplers(1, 1, &mComparisonState);
        md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
    }

    // Following objects are reflective
    ID3D11ShaderResourceView* envMap = mSky->CubeMapSRV();
    md3dImmediateContext->PSSetShaderResources(1, 1, &envMap);

    // No normal or displacement mapping for the spheres or skull
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
    md3dImmediateContext->HSSetShader(0, 0, 0);
    md3dImmediateContext->DSSetShader(0, 0, 0);
    md3dImmediateContext->PSSetShader(mTexPS, 0, 0);

    // Draw the spheres
    for (int i = 0; i < 10; ++i)
    {
        world = XMLoadFloat4x4(&mSphereWorld[i]);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
        dataPtr->Mat = mSphereMat;
        dataPtr->Options = USE_TEXTURES | USE_ENV_MAPPING; // | useOtherMaps;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        //md3dImmediateContext->DSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
        md3dImmediateContext->PSSetShaderResources(2, 1, &mStoneNormal);
        //md3dImmediateContext->DSSetShaderResources(2, 1, &mStoneNormal);
        md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->PSSetSamplers(1, 1, &mComparisonState);
        //md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
    }

    stride = sizeof(BasicVertex);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

    // Draw skull
    world = XMLoadFloat4x4(&mSkullWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixIdentity();
    dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
    dataPtr->Mat = mSkullMat;
    dataPtr->Options = STANDARD_LIGHTING | USE_ENV_MAPPING;
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(3, 1, &shadowMap);
    md3dImmediateContext->PSSetSamplers(1, 1, &mComparisonState);
    md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

    // Draw Animated Characters
    md3dImmediateContext->IASetInputLayout(mSkinnedLayout);
    md3dImmediateContext->VSSetShader(mSkinnedVS, 0, 0);

    HR(md3dImmediateContext->Map(mBoneTransforms, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    SkinnedBuffer* bonePtr = reinterpret_cast<SkinnedBuffer*>(mappedResource.pData);
    for (UINT i = 0; i < mCharacterInstance1.FinalTransforms.size(); ++i)
    {
        bonePtr->BoneTransforms[i] = XMMatrixTranspose(XMLoadFloat4x4(&mCharacterInstance1.FinalTransforms[i]));
    }
    md3dImmediateContext->Unmap(mBoneTransforms, 0);
    md3dImmediateContext->VSSetConstantBuffers(2, 1, &mBoneTransforms);

    for (UINT subset = 0; subset < mCharacterInstance1.Model->subsetCount; ++subset)
    {
        world = XMLoadFloat4x4(&mCharacterInstance1.World);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
        dataPtr->Mat = mCharacterInstance1.Model->Mat[subset];
        dataPtr->Options = USE_TEXTURES | USE_NORMAL_MAP;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

        md3dImmediateContext->PSSetShaderResources(0, 1, &mCharacterInstance1.Model->DiffuseMapSRV[subset]);
        md3dImmediateContext->PSSetShaderResources(2, 1, &mCharacterInstance1.Model->NormalMapSRV[subset]);
        mCharacterInstance1.Model->ModelMesh.Draw(md3dImmediateContext, subset);
    }


    HR(md3dImmediateContext->Map(mBoneTransforms, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    bonePtr = reinterpret_cast<SkinnedBuffer*>(mappedResource.pData);
    for (UINT i = 0; i < mCharacterInstance2.FinalTransforms.size(); ++i)
    {
        bonePtr->BoneTransforms[i] = XMMatrixTranspose(XMLoadFloat4x4(&mCharacterInstance2.FinalTransforms[i]));
    }
    md3dImmediateContext->Unmap(mBoneTransforms, 0);
    md3dImmediateContext->VSSetConstantBuffers(2, 1, &mBoneTransforms);

    for (UINT subset = 0; subset < mCharacterInstance2.Model->subsetCount; ++subset)
    {
        world = XMLoadFloat4x4(&mCharacterInstance2.World);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        dataPtr->ShadowMapTransform = XMMatrixTranspose(shadowTransform);
        dataPtr->Mat = mCharacterInstance2.Model->Mat[subset];
        dataPtr->Options = USE_TEXTURES | USE_NORMAL_MAP;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

        md3dImmediateContext->PSSetShaderResources(0, 1, &mCharacterInstance2.Model->DiffuseMapSRV[subset]);
        md3dImmediateContext->PSSetShaderResources(2, 1, &mCharacterInstance1.Model->NormalMapSRV[subset]);
        mCharacterInstance2.Model->ModelMesh.Draw(md3dImmediateContext, subset);
    }

    ID3D11ShaderResourceView* nullSRV = { 0 };
    md3dImmediateContext->PSSetShaderResources(3, 1, &nullSRV);

    mSky->Draw(md3dImmediateContext, mCamera);

    //DrawScreenQuad();

    md3dImmediateContext->RSSetState(0);

    HR(mSwapChain->Present(0, 0));
}

void SkinnedMeshDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void SkinnedMeshDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void SkinnedMeshDemo::OnMouseMove(WPARAM btnState, int x, int y)
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

void SkinnedMeshDemo::BuildShapeGeometryBuffer()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
    geoGen.CreateSphere(0.5f, 20, 20, sphere);
    geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

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
        vertices[k].Tangent = box.Vertices[i].TangentU;
    }

    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = grid.Vertices[i].Position;
        vertices[k].Normal = grid.Vertices[i].Normal;
        vertices[k].Tex = grid.Vertices[i].TexC;
        vertices[k].Tangent = grid.Vertices[i].TangentU;
    }

    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = sphere.Vertices[i].Position;
        vertices[k].Normal = sphere.Vertices[i].Normal;
        vertices[k].Tex = sphere.Vertices[i].TexC;
        vertices[k].Tangent = sphere.Vertices[i].TangentU;
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = cylinder.Vertices[i].Position;
        vertices[k].Normal = cylinder.Vertices[i].Normal;
        vertices[k].Tex = cylinder.Vertices[i].TexC;
        vertices[k].Tangent = cylinder.Vertices[i].TangentU;
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

void SkinnedMeshDemo::BuildSkullGeometryBuffer()
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

void SkinnedMeshDemo::BuildFX()
{
    // Create the vertex input layout
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3D_SHADER_MACRO basicEffectDefines[] = {
        { "NUM_LIGHTS", "3" },
        { 0, 0 }
    };

    auto ShadowMapFilename = ExePath().append(L"../../../Shaders/ShadowMap.hlsl");
    auto ShadowMapString = ShadowMapFilename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mTexVS, ShadowMapString, "BasicVS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mTexPS, ShadowMapString, "PS", basicEffectDefines);

    ShaderHelper::CreateShader(md3dDevice, &mNormalVS, ShadowMapString, "NormalVS", 0, &mShapeInputLayout, vertexDesc, 4);

    ShaderHelper::CreateShader(md3dDevice, &mDispVS, ShadowMapString, "DispVS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mDispHS, ShadowMapString, "HS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mDispDS, ShadowMapString, "DS", 0);

    auto BuildMapFilename = ExePath().append(L"../../../Shaders/BuildShadowMap.hlsl");
    auto BuildMapString = BuildMapFilename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mShadowVS, BuildMapString, "VS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mShadowTessVS, BuildMapString, "TessVS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mShadowHS, BuildMapString, "HS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mShadowDS, BuildMapString, "DS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mShadowPS, BuildMapString, "PS", 0);

    D3D11_INPUT_ELEMENT_DESC quadDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    auto QuadShaderFilename = ExePath().append(L"../../../Shaders/BasicTexture.hlsl");
    auto QuadShaderString = QuadShaderFilename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mQuadVS, QuadShaderString, "VS", 0, &mQuadInputLayout, quadDesc, 2);
    ShaderHelper::CreateShader(md3dDevice, &mQuadPS, QuadShaderString, "GrayscalePS", 0);

    D3D11_INPUT_ELEMENT_DESC skinnedDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    ShaderHelper::CreateShader(md3dDevice, &mSkinnedVS, ShadowMapString, "SkinnedVS", 0, &mSkinnedLayout, skinnedDesc, 6);
    ShaderHelper::CreateShader(md3dDevice, &mShadowSkinnedVS, BuildMapString, "SkinnedVS", 0);

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

    matrixBufferDesc2.ByteWidth = sizeof(PerQuadBuffer);

    HR(md3dDevice->CreateBuffer(&matrixBufferDesc2, 0, &mPerQuadBuffer));

    matrixBufferDesc2.ByteWidth = sizeof(SkinnedBuffer);

    HR(md3dDevice->CreateBuffer(&matrixBufferDesc2, 0, &mBoneTransforms));
}

void SkinnedMeshDemo::BuildTex()
{
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/floor.dds").c_str(), nullptr, &mFloorTex));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/bricks.dds").c_str(), nullptr, &mBrickTex));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/stones.dds").c_str(), nullptr, &mStoneTex));

    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/floor_nmap.dds").c_str(), nullptr, &mFloorNormal));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/bricks_nmap.dds").c_str(), nullptr, &mBrickNormal));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/stones_nmap.dds").c_str(), nullptr, &mStoneNormal));

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
}

void SkinnedMeshDemo::BuildShadowTransform()
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

    XMMATRIX S = V * P * T;

    XMStoreFloat4x4(&mLightView, V);
    XMStoreFloat4x4(&mLightProj, P);
    XMStoreFloat4x4(&mShadowTransform, S);
}

void SkinnedMeshDemo::DrawSceneToShadowMap()
{
    md3dImmediateContext->RSSetState(mShadowRS);

    XMMATRIX view = XMLoadFloat4x4(&mLightView);
    XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* framePtr = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);
    framePtr->EyePosW = mCamera.GetPosition();

    framePtr->HeightScale - 0.07f;
    framePtr->MaxTessDistance = 1.0f;
    framePtr->MinTessDistance = 25.0f;
    framePtr->MinTessFactor = 1.0f;
    framePtr->MaxTessFactor = 5.0f;

    framePtr->ViewProj = XMMatrixTranspose(viewProj);

    md3dImmediateContext->Unmap(mPerFrameBuffer, 0);

    md3dImmediateContext->VSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    md3dImmediateContext->DSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    md3dImmediateContext->IASetInputLayout(mShapeInputLayout);
    switch (mRenderOptions)
    {
    case RenderOptionsBasic:
    case RenderOptionsNormalMap:
    default:
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        md3dImmediateContext->VSSetShader(mShadowVS, 0, 0);
        break;

    case RenderOptionsDisplacementMap:
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        md3dImmediateContext->VSSetShader(mShadowTessVS, 0, 0);
        md3dImmediateContext->HSSetShader(mShadowHS, 0, 0);
        md3dImmediateContext->DSSetShader(mShadowDS, 0, 0);
        break;
    }
    md3dImmediateContext->PSSetShader(mShadowPS, 0, 0);

    UINT stride = sizeof(TexVertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    // Draw the grid
    XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    PerObjectBuffer* dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(8.0f, 10.0f, 1.0f));
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(0, 1, &mFloorTex);
    md3dImmediateContext->DSSetShaderResources(1, 1, &mFloorNormal);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

    // Draw the box
    world = XMLoadFloat4x4(&mBoxWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(2.0f, 1.0f, 1.0f));
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
    md3dImmediateContext->DSSetShaderResources(1, 1, &mStoneNormal);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

    // Draw the cylinders
    for (int i = 0; i < 10; ++i)
    {
        world = XMLoadFloat4x4(&mCylWorld[i]);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixTranspose(XMMatrixScaling(1.0f, 2.0f, 1.0f));
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mBrickTex);
        md3dImmediateContext->DSSetShaderResources(1, 1, &mBrickNormal);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
    }

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    md3dImmediateContext->VSSetShader(mShadowVS, 0, 0);
    md3dImmediateContext->HSSetShader(0, 0, 0);
    md3dImmediateContext->DSSetShader(0, 0, 0);
    md3dImmediateContext->PSSetShader(mShadowPS, 0, 0);

    // Draw the spheres
    for (int i = 0; i < 10; ++i)
    {
        world = XMLoadFloat4x4(&mSphereWorld[i]);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
    }

    stride = sizeof(BasicVertex);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

    // Draw skull
    world = XMLoadFloat4x4(&mSkullWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixIdentity();
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

    // Draw Animated Characters
    md3dImmediateContext->IASetInputLayout(mSkinnedLayout);
    md3dImmediateContext->VSSetShader(mSkinnedVS, 0, 0);

    world = XMLoadFloat4x4(&mCharacterInstance1.World);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixIdentity();
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);

    HR(md3dImmediateContext->Map(mBoneTransforms, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    SkinnedBuffer* bonePtr = reinterpret_cast<SkinnedBuffer*>(mappedResource.pData);
    for (UINT i = 0; i < mCharacterInstance1.FinalTransforms.size(); ++i)
    {
        bonePtr->BoneTransforms[i] = XMMatrixTranspose(XMLoadFloat4x4(&mCharacterInstance1.FinalTransforms[i]));
    }
    md3dImmediateContext->Unmap(mBoneTransforms, 0);
    md3dImmediateContext->VSSetConstantBuffers(2, 1, &mBoneTransforms);

    for (UINT subset = 0; subset < mCharacterInstance1.Model->subsetCount; ++subset)
    {
        //md3dImmediateContext->PSSetShaderResources(0, 1, &mCharacterInstance1.Model->DiffuseMapSRV[subset]);
        mCharacterInstance1.Model->ModelMesh.Draw(md3dImmediateContext, subset);
    }

    world = XMLoadFloat4x4(&mCharacterInstance2.World);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixIdentity();
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);

    HR(md3dImmediateContext->Map(mBoneTransforms, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    bonePtr = reinterpret_cast<SkinnedBuffer*>(mappedResource.pData);
    for (UINT i = 0; i < mCharacterInstance2.FinalTransforms.size(); ++i)
    {
        bonePtr->BoneTransforms[i] = XMMatrixTranspose(XMLoadFloat4x4(&mCharacterInstance2.FinalTransforms[i]));
    }
    md3dImmediateContext->Unmap(mBoneTransforms, 0);
    md3dImmediateContext->VSSetConstantBuffers(2, 1, &mBoneTransforms);

    for (UINT subset = 0; subset < mCharacterInstance2.Model->subsetCount; ++subset)
    {
        //md3dImmediateContext->PSSetShaderResources(0, 1, &mCharacterInstance2.Model->DiffuseMapSRV[subset]);
        mCharacterInstance2.Model->ModelMesh.Draw(md3dImmediateContext, subset);
    }

    ID3D11Buffer* nullBuffer = { 0 };
    md3dImmediateContext->VSSetConstantBuffers(0, 1, &nullBuffer);
    md3dImmediateContext->DSSetConstantBuffers(0, 1, &nullBuffer);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &nullBuffer);
    ID3D11SamplerState* nullSampler = { 0 };
    md3dImmediateContext->DSSetSamplers(0, 1, &nullSampler);
    md3dImmediateContext->PSSetSamplers(0, 1, &nullSampler);
    ID3D11ShaderResourceView* nullSRV = { 0 };
    md3dImmediateContext->DSSetShaderResources(0, 1, &nullSRV);
    md3dImmediateContext->PSSetShaderResources(1, 1, &nullSRV);

    md3dImmediateContext->VSSetShader(0, 0, 0);
    md3dImmediateContext->PSSetShader(0, 0, 0);

    md3dImmediateContext->RSSetState(0);
}

void SkinnedMeshDemo::BuildScreenQuadGeometryBuffer()
{
    GeometryGenerator::MeshData quad;

    GeometryGenerator geoGen;
    geoGen.CreateFullScreenQuad(quad);

    std::vector<QuadVertex> vertices(quad.Vertices.size());

    for (UINT i = 0; i < quad.Vertices.size(); ++i)
    {
        vertices[i].Pos = quad.Vertices[i].Position;
        vertices[i].Tex = quad.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(QuadVertex) * quad.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mQuadVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = quad.Indices.data();
    HR(md3dDevice->CreateBuffer(&ibd, &initData, &mQuadIB));
}

void SkinnedMeshDemo::DrawScreenQuad()
{
    UINT stride = sizeof(QuadVertex);
    UINT offset = 0;

    md3dImmediateContext->IASetInputLayout(mQuadInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->VSSetShader(mQuadVS, 0, 0);
    md3dImmediateContext->PSSetShader(mQuadPS, 0, 0);

    md3dImmediateContext->OMSetDepthStencilState(mQuadDS, 0);

    XMMATRIX world(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerQuadBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerQuadBuffer* dataPtr = reinterpret_cast<PerQuadBuffer*>(mappedResource.pData);
    dataPtr->WorldViewProj = XMMatrixTranspose(world);

    md3dImmediateContext->Unmap(mPerQuadBuffer, 0);

    md3dImmediateContext->VSGetConstantBuffers(0, 1, &mPerQuadBuffer);

    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    ID3D11ShaderResourceView* dsv = mSmap->DepthMapSRV();
    md3dImmediateContext->PSSetShaderResources(0, 1, &dsv);

    md3dImmediateContext->DrawIndexed(6, 0, 0);

    ID3D11Buffer* nullBuffer = { 0 };
    md3dImmediateContext->VSGetConstantBuffers(0, 1, &nullBuffer);
    ID3D11SamplerState* nullSampler = { 0 };
    md3dImmediateContext->PSSetSamplers(0, 1, &nullSampler);
    ID3D11ShaderResourceView* nullSRV = { 0 };
    md3dImmediateContext->PSSetShaderResources(0, 1, &nullSRV);

    md3dImmediateContext->VSSetShader(0, 0, 0);
    md3dImmediateContext->PSSetShader(0, 0, 0);

    md3dImmediateContext->OMSetDepthStencilState(0, 0);
}