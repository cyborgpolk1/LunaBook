#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

class ComputeAddTexture : public D3DApp
{
public:
    ComputeAddTexture(HINSTANCE hInstance);
    ~ComputeAddTexture();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildFullScreenQuad();
    void BuildFX();
    void BuildTex();
    void DoComputeWork();

private:
    ID3D11Buffer* mQuadVB;
    ID3D11Buffer* mQuadIB;

    UINT mQuadIndexCount;

    ID3D11VertexShader* mVS;
    ID3D11PixelShader* mPS;
    ID3D11ComputeShader* mCS;

    ID3D11Buffer* mConstantBuffer;

    ID3D11InputLayout* mInputLayout;

    ID3D11SamplerState* mSampler;

    ID3D11ShaderResourceView* mTex1;
    ID3D11ShaderResourceView* mTex2;

    ID3D11ShaderResourceView* mTexOut;
    ID3D11UnorderedAccessView* mTexWrite;
};