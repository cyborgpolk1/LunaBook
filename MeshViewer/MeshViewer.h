#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "TextuerMgr.h"
#include "Sky.h"
#include "BasicModel.h"
#include "Camera.h"
#include "ShadowMap.h"

struct BoundingSphere
{
    BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
    XMFLOAT3 Center;
    float Radius;
};

struct alignas(16) PerFrameBuffer
{
    XMFLOAT3 EyePosW;

    float HeightScale;
    float MaxTessDistance;
    float MinTessDistance;
    float MinTessFactor;
    float MaxTessFactor;

    XMMATRIX ViewProj;

    DirectionalLight DirLights[3];
};

struct alignas(16) PerObjectBuffer
{
    XMMATRIX World;
    XMMATRIX WorldInvTranspose;
    XMMATRIX gTexTransform;
    XMMATRIX ShadowMapTransform;
    Material Mat;
    int Options;
};

class MeshViewer : public D3DApp
{
public:
    MeshViewer(HINSTANCE hInstance);
    ~MeshViewer();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void DrawSceneToShadowMap();
    void BuildShadowTransform();
    void BuildFX();

private:
    TextureMgr mTexMgr;

    Sky* mSky;

    BasicModel* mTreeModel;
    BasicModel* mBaseModel;
    BasicModel* mStairsModel;
    BasicModel* mPillar1Model;
    BasicModel* mPillar2Model;
    BasicModel* mPillar3Model;
    BasicModel* mPillar4Model;
    BasicModel* mRockModel;

    std::vector<BasicModelInstance> mModelInstances;
    std::vector<BasicModelInstance> mAlphaClippedModelInstances;

    ID3D11InputLayout* mInputLayout;

    ID3D11VertexShader* mNormalVS;
    ID3D11PixelShader* mNormalPS;

    ID3D11VertexShader* mBuildVS;
    ID3D11PixelShader* mBuildPS;

    ID3D11Buffer* mPerFrameBuffer;
    ID3D11Buffer* mPerObjectBuffer;

    ID3D11SamplerState* mSampleState;
    ID3D11SamplerState* mComparisonState;

    ID3D11RasterizerState* mShadowRS;

    BoundingSphere mSceneBounds;

    static const int SMapSize = 2048;
    ShadowMap* mSmap;
    XMFLOAT4X4 mLightView;
    XMFLOAT4X4 mLightProj;
    XMFLOAT4X4 mShadowTransform;

    float mLightRotationAngle;
    XMFLOAT3 mOriginalLightDir[3];
    DirectionalLight mDirLights[3];

    Camera mCamera;

    POINT mLastMousePos;
};