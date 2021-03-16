#pragma once

#include "D3DApp.h"
#include "Camera.h"
#include "LightHelper.h"
#include <DirectXCollision.h>
#include <vector>

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    //XMFLOAT2 Tex;
};

struct InstanceData
{
    XMFLOAT4X4 World;
    XMFLOAT4 Color;
};

struct alignas(16) PerFrameBuffer
{
    DirectionalLight DirLights[3];
    XMFLOAT3 EyePosW;
};

struct PerObjectBuffer
{
    XMMATRIX World;
    XMMATRIX WorldInvTranspose;
    XMMATRIX ViewProj;
    Material Mat;
};

class InstancingAndCulling : public D3DApp
{
public:
    InstancingAndCulling(HINSTANCE hInstance);
    ~InstancingAndCulling();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildSkullGeometryBuffer();
    void BuildInstanceBuffer();
    void BuildFX();

private:
    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;
    ID3D11Buffer* mInstanceBuffer;

    std::vector<InstanceData> mInstanceData;

    UINT mVisibleObjectCount;

    DirectionalLight mDirLights[3];
    Material mSkullMat;

    BoundingBox mSkullBox;
    BoundingFrustum mCameraFrustum;

    bool mEnableFrustumCulling;

    UINT mSkullIndexCount;

    XMFLOAT4X4 mSkullWorld;

    ID3D11VertexShader* mVS;
    ID3D11PixelShader* mPS;

    ID3D11InputLayout* mInputLayout;

    ID3D11Buffer* mPerFrameBuffer;
    ID3D11Buffer* mPerObjectBuffer;

    Camera mCamera;

    POINT mLastMousePos;
};