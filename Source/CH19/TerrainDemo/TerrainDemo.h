#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "LightHelper.h"
#include "Terrain.h"
#include "Camera.h"
#include "Sky.h"

class TerrainDemo : public D3DApp
{
public:
    TerrainDemo(HINSTANCE hInstance);
    ~TerrainDemo();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:

private:
    DirectionalLight mDirLights[3];

    Terrain mTerrain;

    Camera mCamera;
    bool mWalkCamMode;

    bool mRenderWireframe;
    ID3D11RasterizerState* mWireframeRS;

    Sky* mSky;

    POINT mLastMousePos;
};