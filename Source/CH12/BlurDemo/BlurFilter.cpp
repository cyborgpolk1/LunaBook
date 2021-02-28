#include "BlurFilter.h"
#include <d3dcompiler.h>
#include <string>

BlurFilter::BlurFilter()
    : mBlurredOutputTexSRV(0), mBlurredOutputTexUAV(0), mHorzCS(0), mVertCS(0)
{
}

BlurFilter::~BlurFilter()
{
    ReleaseCOM(mBlurredOutputTexSRV);
    ReleaseCOM(mBlurredOutputTexUAV);
    ReleaseCOM(mHorzCS);
    ReleaseCOM(mVertCS);
}

ID3D11ShaderResourceView* BlurFilter::GetBlurredOutput()
{
    return mBlurredOutputTexSRV;
}

void BlurFilter::SetGaussianWeights(float sigma)
{
    float d = 2.0f * sigma * sigma;

    float weights[9];
    float sum = 0.0f;
    for (UINT i = 0; i < 8; ++i)
    {
        float x = (float)i;
        weights[i] = expf(-x * x / d);

        sum += weights[i];
    }

    for (UINT i = 0; i < 8; ++i)
    {
        weights[i] /= sum;
    }
}

void BlurFilter::SetWeights(const float weights[9])
{

}

void BlurFilter::Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
    ReleaseCOM(mBlurredOutputTexSRV);
    ReleaseCOM(mBlurredOutputTexUAV);

    mWidth = width;
    mHeight = height;
    mFormat = format;

    D3D11_TEXTURE2D_DESC blurredTexDesc;
    blurredTexDesc.Width = width;
    blurredTexDesc.Height = height;
    blurredTexDesc.MipLevels = 1;
    blurredTexDesc.ArraySize = 1;
    blurredTexDesc.Format = format;
    blurredTexDesc.SampleDesc.Count = 1;
    blurredTexDesc.SampleDesc.Quality = 0;
    blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
    blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    blurredTexDesc.CPUAccessFlags = 0;
    blurredTexDesc.MiscFlags = 0;

    ID3D11Texture2D* blurredTex = 0;
    HR(device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    HR(device->CreateShaderResourceView(blurredTex, &srvDesc, &mBlurredOutputTexSRV));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    HR(device->CreateUnorderedAccessView(blurredTex, &uavDesc, &mBlurredOutputTexUAV));

    ReleaseCOM(blurredTex);

    if (!mHorzCS || !mVertCS)
    {
        wchar_t buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
        auto filename = std::wstring(buffer).substr(0, pos + 1).append(L"../../../Shaders/ComputeBlur.hlsl");
        
        ID3D10Blob* compiledShader = CompileShader(filename.c_str(), "HorzBlurCS", "cs_5_0", 0);
        HR(device->CreateComputeShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &mHorzCS));
        ReleaseCOM(compiledShader);

        compiledShader = CompileShader(filename.c_str(), "VertBlurCS", "cs_5_0", 0);
        HR(device->CreateComputeShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &mVertCS));
        ReleaseCOM(compiledShader);
    }
}

void BlurFilter::BlurInPlace(ID3D11DeviceContext* dc,
    ID3D11ShaderResourceView* inputSRV,
    ID3D11UnorderedAccessView* inputUAV,
    int blurCount)
{
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ID3D11UnorderedAccessView* nullUAV = nullptr;

    for (int i = 0; i < blurCount; ++i)
    {
        dc->CSSetShaderResources(0, 1, &inputSRV);
        dc->CSSetUnorderedAccessViews(0, 1, &mBlurredOutputTexUAV, 0);
        dc->CSSetShader(mHorzCS, 0, 0);

        UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
        dc->Dispatch(numGroupsX, mHeight, 1);

        dc->CSSetShaderResources(0, 1, &nullSRV);
        dc->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);

        dc->CSSetShaderResources(0, 1, &mBlurredOutputTexSRV);
        dc->CSSetUnorderedAccessViews(0, 1, &inputUAV, 0);
        dc->CSSetShader(mVertCS, 0, 0);

        UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
        dc->Dispatch(mWidth, numGroupsY, 1);

        dc->CSSetShaderResources(0, 1, &nullSRV);
        dc->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
    }

    dc->CSSetShader(0, 0, 0);
}

ID3DBlob* BlurFilter::CompileShader(LPCWSTR filename, LPCSTR entry, LPCSTR target, const D3D_SHADER_MACRO* defines)
{
    DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3D10_SHADER_DEBUG;
    shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* compiledShader = 0;
    ID3DBlob* compilationMsgs = 0;

    HRESULT hr = D3DCompileFromFile(filename, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, shaderFlags, 0, &compiledShader, &compilationMsgs);

    // compilationMsgs can store errors or warnings.
    if (compilationMsgs != 0)
    {
        wchar_t* error = new wchar_t[compilationMsgs->GetBufferSize() + 1];
        size_t out;
        mbstowcs_s(&out, error, compilationMsgs->GetBufferSize() + 1, (const char*)compilationMsgs->GetBufferPointer(), compilationMsgs->GetBufferSize());
        MessageBox(0, error, 0, 0);
    }

    // Even if there are no compilationsMsgs, check to make sure there were no other errors.
    if (FAILED(hr))
    {
        DXTrace(__FILEW__, (DWORD)__LINE__, hr, L"D3DCompileFromFile", true);
    }

    ReleaseCOM(compilationMsgs);

    return compiledShader;
}