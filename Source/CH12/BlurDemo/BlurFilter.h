#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include "d3dUtil.h"

class BlurFilter {
public:
    BlurFilter();
    ~BlurFilter();

    ID3D11ShaderResourceView* GetBlurredOutput();

    void SetGaussianWeights(float sigma);
    void SetWeights(const float weights[9]);

    void Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);

    void BlurInPlace(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* inputSRV, 
        ID3D11UnorderedAccessView* inputUAV, int blurCount);

private:
    UINT mWidth;
    UINT mHeight;
    DXGI_FORMAT mFormat;

    ID3D11ShaderResourceView* mBlurredOutputTexSRV;
    ID3D11UnorderedAccessView* mBlurredOutputTexUAV;

    ID3D11ComputeShader* mHorzCS;
    ID3D11ComputeShader* mVertCS;
};