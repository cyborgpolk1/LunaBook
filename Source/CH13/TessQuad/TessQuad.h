#pragma once

#include "D3DApp.h"

struct MatrixBuffer
{
    XMMATRIX World;
    XMMATRIX WorldViewProj;
    XMFLOAT3 EyePosW;
};

class TessQuad : public D3DApp
{
public:
    TessQuad(HINSTANCE hInstance);
    ~TessQuad();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildQuadPatchBuffer();
    void BuildFX();

private:
    ID3D11Buffer* mQuadPatchVB;
    
    ID3D11VertexShader* mVS;
    ID3D11HullShader* mHS;
    ID3D11DomainShader* mDS;
    ID3D11PixelShader* mPS;

    ID3D11InputLayout* mInputLayout;

    ID3D11Buffer* mMatrixBuffer;

    ID3D11RasterizerState* mWireframeRS;

    XMFLOAT4X4 mWorld;
    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;
    XMFLOAT3 mEyePosW;

    float mTheta;
    float mPhi;
    float mRadius;

    POINT mLastMousePos;
};