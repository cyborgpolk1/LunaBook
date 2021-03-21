#pragma once

#include "d3dUtil.h"
#include "Camera.h"

class Sky
{
public:
    Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius);
    ~Sky();

    ID3D11ShaderResourceView* CubeMapSRV();

    void Draw(ID3D11DeviceContext* dc, const Camera& camera);

private:
    Sky(const Sky& rhs);
    Sky& operator=(const Sky& rhs);

private:
    ID3D11Buffer* mVB;
    ID3D11Buffer* mIB;
    ID3D11Buffer* mMatrixBuffer;

    ID3D11InputLayout* mInputLayout;

    ID3D11ShaderResourceView* mCubeMapSRV;
    ID3D11SamplerState* mCubeMapSS;

    ID3D11VertexShader* mVS;
    ID3D11PixelShader* mPS;

    ID3D11RasterizerState* mRS;
    ID3D11DepthStencilState* mDDS;

    UINT mIndexCount;

    struct MatrixBuffer
    {
        XMMATRIX WorldViewProj;
    };
};

