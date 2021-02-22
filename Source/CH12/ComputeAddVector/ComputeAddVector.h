#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Data
{
    XMFLOAT3 v1;
    XMFLOAT2 v2;
};

class ComputeAddVector : public D3DApp
{
public:
    ComputeAddVector(HINSTANCE hInstance);
    ~ComputeAddVector();

    bool Init();
    void OnResize();
    void UpdateScene(float dt) {}
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y) {}
    void OnMouseUp(WPARAM btnState, int x, int y) {}
    void OnMouseMove(WPARAM btnState, int x, int y) {}

private:
    void BuildData();
    void BuildFX();
    void DoComputeWork();

private:
    ID3D11ComputeShader* mCS;

    ID3D11ShaderResourceView* mInputA;
    ID3D11ShaderResourceView* mInputB;

    ID3D11UnorderedAccessView* mOutput;

    ID3D11Buffer* mOutputBuffer;
    ID3D11Buffer* mOutputDebugBuffer;

    const UINT mNumElements = 32;
};