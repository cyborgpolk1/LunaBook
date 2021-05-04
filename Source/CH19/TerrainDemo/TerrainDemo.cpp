#include "TerrainDemo.h"
#include "MathHelper.h"

D3DMAIN(TerrainDemo);

TerrainDemo::TerrainDemo(HINSTANCE hInstance)
    : D3DApp(hInstance), mSky(0), mWalkCamMode(true), mRenderWireframe(false), mWireframeRS(0)
{
    mMainWndCaption = L"Terrain Demo";
    
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

TerrainDemo::~TerrainDemo()
{
    md3dImmediateContext->ClearState();

    SafeDelete(mSky);
    
    ReleaseCOM(mWireframeRS);
}

bool TerrainDemo::Init()
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

    return true;
}

void TerrainDemo::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void TerrainDemo::UpdateScene(float dt)
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
    
    mCamera.UpdateViewMatrix();
}

void TerrainDemo::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (mRenderWireframe)
        md3dImmediateContext->RSSetState(mWireframeRS);

    mTerrain.Draw(md3dImmediateContext, mCamera, mDirLights);

    md3dImmediateContext->RSSetState(0);

    mSky->Draw(md3dImmediateContext, mCamera);

    HR(mSwapChain->Present(0, 0));
}

void TerrainDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void TerrainDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void TerrainDemo::OnMouseMove(WPARAM btnState, int x, int y)
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