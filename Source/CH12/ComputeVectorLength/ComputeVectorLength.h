#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Data
{
    XMFLOAT3 vec;
};

class ComputeVectorLength : public D3DApp
{
public:
    ComputeVectorLength(HINSTANCE hInstance);
    ~ComputeVectorLength();

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
    ID3D11ComputeShader* mStructCS;
    ID3D11ComputeShader* mTypedCS;
    ID3D11ComputeShader* mAppendCS;

    ID3D11ShaderResourceView* mStructInputSRV;
    ID3D11ShaderResourceView* mTypedInputSRV;
    ID3D11UnorderedAccessView* mAppendInputUAV;

    ID3D11UnorderedAccessView* mStructOutputUAV;
    ID3D11UnorderedAccessView* mTypedOutputUAV;
    ID3D11UnorderedAccessView* mAppendOutputUAV;

    ID3D11Buffer* mStructOutputBuffer;
    ID3D11Buffer* mTypedOutputBuffer;
    ID3D11Buffer* mAppendOutputBuffer;

    ID3D11Buffer* mStructDebugBuffer;
    ID3D11Buffer* mTypedDebugBuffer;
    ID3D11Buffer* mAppendDebugBuffer;

    const UINT mNumElements = 64;
};