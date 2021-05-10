#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "LightHelper.h"
#include "Terrain.h"
#include "ParticleSystem.h"
#include "Camera.h"
#include "Sky.h"

class ParticleDemo : public D3DApp
{
public:
    ParticleDemo(HINSTANCE hInstance);
    ~ParticleDemo();

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

    ParticleSystem mFireSystem;
    ParticleSystem mRainSystem;

    ID3D11ShaderResourceView* mRandomTexSRV;
    ID3D11ShaderResourceView* mFlareSRV;
    ID3D11ShaderResourceView* mRainSRV;

    Camera mCamera;
    bool mWalkCamMode;

    bool mRenderWireframe;
    ID3D11RasterizerState* mWireframeRS;

    Sky* mSky;

    POINT mLastMousePos;
};