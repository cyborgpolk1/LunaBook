#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    float AmbientOcc;
};

struct MatrixBuffer
{
    XMMATRIX WorldViewProj;
};

class AmbientOcclusionDemo : public D3DApp
{
public:
    AmbientOcclusionDemo(HINSTANCE hInstance);
    ~AmbientOcclusionDemo();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildGeometryBuffer();
    void BuildFX();

    void BuildVertexAmbientOcclusion(std::vector<Vertex>& vertices, const std::vector<UINT>& indices);

private:
    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;
    ID3D11VertexShader* mVS;
    ID3D11PixelShader* mPS;

    ID3D11InputLayout* mInputLayout;

    ID3D11Buffer* mMatrixBuffer;

    int mSkullIndexCount;

    XMFLOAT4X4 mWorld;
    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    float mTheta;
    float mPhi;
    float mRadius;

    POINT mLastMousePoint;
};