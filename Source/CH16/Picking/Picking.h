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
    XMMATRIX WorldViewProj;
    XMMATRIX TexTransform;
    Material Mat;
};

class PickingDemo : public D3DApp
{
public:
    PickingDemo(HINSTANCE hInstance);
    ~PickingDemo();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildCarGeometryBuffer();
    void BuildFX();
    void Pick(int sx, int sy);

private:
    ID3D11Buffer* mCarVB;
    ID3D11Buffer* mCarIB;

    std::vector<Vertex> mCarVertices;
    std::vector<UINT> mCarIndices;

    UINT mCarIndexCount;
    int mPickedTriangle;

    BoundingBox mCarBox;

    ID3D11InputLayout* mInputLayout;

    ID3D11VertexShader* mVS;
    ID3D11PixelShader* mPS;

    ID3D11Buffer* mPerFrameBuffer;
    ID3D11Buffer* mPerObjectBuffer;

    XMFLOAT4X4 mCarWorld;

    DirectionalLight mDirLights[3];
    Material mCarMat;
    Material mPickedMat;

    ID3D11DepthStencilState* mLessEqualDS;
    
    ID3D11RasterizerState* mWireframeRS;
    bool mWireframe;

    Camera mCamera;

    bool heldDown;

    POINT mLastMousePos;
};