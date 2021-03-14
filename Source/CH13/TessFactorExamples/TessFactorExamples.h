#pragma once

#include "D3DApp.h"

class TessFactorExamples : public D3DApp
{
public:
    TessFactorExamples(HINSTANCE hInstance);
    ~TessFactorExamples();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildPatchBuffers();
    void BuildFX();

private:
    ID3D11Buffer* mTriPatchVB;
    ID3D11Buffer* mQuadPatchVB;

    UINT mTriVertexCount;
    UINT mQuadVertexCount;

    ID3D11VertexShader* mVS;
    ID3D11HullShader* mTriHS;
    ID3D11HullShader* mQuadHS;
    ID3D11DomainShader* mTriDS;
    ID3D11DomainShader* mQuadDS;
    ID3D11PixelShader* mPS;

    ID3D11InputLayout* mInputLayout;

    ID3D11RasterizerState* mWireframeRS;
};