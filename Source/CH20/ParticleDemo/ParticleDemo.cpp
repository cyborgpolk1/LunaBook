#include "ParticleDemo.h"
#include "MathHelper.h"
#include <vector>

D3DMAIN(ParticleDemo);

ParticleDemo::ParticleDemo(HINSTANCE hInstance)
    : D3DApp(hInstance), mSky(0), mWalkCamMode(true), mRenderWireframe(false), mWireframeRS(0),
    mRandomTexSRV(0), mFlareSRV(0), mRainSRV(0)
{
    mMainWndCaption = L"Particle Demo";
    
    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCamera.SetPosition(0.0f, 2.0f, 100.0f);

    mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);
}

ParticleDemo::~ParticleDemo()
{
    md3dImmediateContext->ClearState();

    SafeDelete(mSky);
    
    ReleaseCOM(mWireframeRS);
    ReleaseCOM(mRandomTexSRV);
    ReleaseCOM(mFlareSRV);
    ReleaseCOM(mRainSRV);
}

bool ParticleDemo::Init()
{
    if (!D3DApp::Init())
        return false;

    auto texPath = ExePath().append(L"../../../Textures/");

    mSky = new Sky(md3dDevice, ExePath().append(L"../../../Textures/grasscube1024.dds"), 5000.0f);

    Terrain::InitInfo tii;
    tii.HeightMapFilename = ExePath().append(L"../../../Textures/terrain.raw");
    tii.LayerMapFilename0 = ExePath().append(L"../../../Textures/grass.dds");
    tii.LayerMapFilename1 = ExePath().append(L"../../../Textures/darkdirt.dds");
    tii.LayerMapFilename2 = ExePath().append(L"../../../Textures/stone_mtn.dds");
    tii.LayerMapFilename3 = ExePath().append(L"../../../Textures/lightdirt.dds");
    tii.LayerMapFilename4 = ExePath().append(L"../../../Textures/snow.dds");
    tii.BlendMapFilename = ExePath().append(L"../../../Textures/blend.dds");
    tii.HeightScale = 50.0f;
    tii.HeightmapWidth = 2049;
    tii.HeightmapHeight = 2049;
    tii.CellSpacing = 0.5f;

    mTerrain.Init(md3dDevice, md3dImmediateContext, tii);

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mWireframeRS));

    mRandomTexSRV = D3DHelper::CreateRandomTexture1DSRV(md3dDevice);

    std::vector<std::wstring> flares;
    flares.push_back(ExePath().append(L"../../../Textures/flare0.dds")); 
    mFlareSRV = D3DHelper::CreateTexture2DArraySRV(md3dDevice, md3dImmediateContext, flares);

    mFireSystem.Init(md3dDevice, ParticleSystem::SystemType::FIRE, mFlareSRV, mRandomTexSRV, 500);
    mFireSystem.SetEmitPos(XMFLOAT3(0.0f, 1.0f, 120.0f));

    std::vector<std::wstring> raindrops;
    raindrops.push_back(ExePath().append(L"../../../Textures/raindrop.dds"));
    mRainSRV = D3DHelper::CreateTexture2DArraySRV(md3dDevice, md3dImmediateContext, raindrops);

    mRainSystem.Init(md3dDevice, ParticleSystem::SystemType::RAIN, mRainSRV, mRandomTexSRV, 10000);

    return true;
}

void ParticleDemo::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void ParticleDemo::UpdateScene(float dt)
{
    //
    // Control the camera.
    //
    if (GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(10.0f * dt);

    if (GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-10.0f * dt);

    if (GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-10.0f * dt);

    if (GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(10.0f * dt);

    static bool mOnePressed = false;
    if (GetAsyncKeyState('1') & 0x8000)
    {
        if (!mOnePressed)
            mRenderWireframe = !mRenderWireframe;

        mOnePressed = true;
    }
    else
        mOnePressed = false;

    //
    // Walk/fly mode
    //
    if (GetAsyncKeyState('2') & 0x8000)
        mWalkCamMode = true;

    if (GetAsyncKeyState('3') & 0x8000)
        mWalkCamMode = false;

    //
    // Clamp camera to terrain surface in walk mode.
    //
    if (mWalkCamMode)
    {
        XMFLOAT3 camPos = mCamera.GetPosition();
        float y = mTerrain.GetHeight(camPos.x, camPos.z);
        mCamera.SetPosition(camPos.x, y + 2.0f, camPos.z);
    }

    mFireSystem.Update(dt, mTimer.TotalTime());
    mRainSystem.Update(dt, mTimer.TotalTime());

    mCamera.UpdateViewMatrix();
}

void ParticleDemo::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (mRenderWireframe)
        md3dImmediateContext->RSSetState(mWireframeRS);

    mTerrain.Draw(md3dImmediateContext, mCamera, mDirLights);

    md3dImmediateContext->RSSetState(0);

    mSky->Draw(md3dImmediateContext, mCamera);

    mFireSystem.SetEyePos(mCamera.GetPosition());
    mFireSystem.Draw(md3dImmediateContext, mCamera);

    mRainSystem.SetEyePos(mCamera.GetPosition());
    mRainSystem.SetEmitPos(mCamera.GetPosition());
    mRainSystem.Draw(md3dImmediateContext, mCamera);

    HR(mSwapChain->Present(0, 0));
}

void ParticleDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void ParticleDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void ParticleDemo::OnMouseMove(WPARAM btnState, int x, int y)
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