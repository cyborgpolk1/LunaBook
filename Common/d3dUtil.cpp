#include "d3dUtil.h"
#include "MathHelper.h"
#include "DDSTextureLoader.h"

void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M)
{
    //
    // Left
    //
    planes[0].x = M.m[0][3] + M.m[0][0];
    planes[0].y = M.m[1][3] + M.m[1][0];
    planes[0].z = M.m[2][3] + M.m[2][0];
    planes[0].w = M.m[3][3] + M.m[3][0];

    //
    // Right
    //
    planes[1].x = M.m[0][3] - M.m[0][0];
    planes[1].y = M.m[1][3] - M.m[1][0];
    planes[1].z = M.m[2][3] - M.m[2][0];
    planes[1].w = M.m[3][3] - M.m[3][0];

    //
    // Bottom
    //
    planes[2].x = M.m[0][3] + M.m[0][1];
    planes[2].y = M.m[1][3] + M.m[1][1];
    planes[2].z = M.m[2][3] + M.m[2][1];
    planes[2].w = M.m[3][3] + M.m[3][1];

    //
    // Top
    //
    planes[3].x = M.m[0][3] - M.m[0][1];
    planes[3].y = M.m[1][3] - M.m[1][1];
    planes[3].z = M.m[2][3] - M.m[2][1];
    planes[3].w = M.m[3][3] - M.m[3][1];

    //
    // Near
    //
    planes[4].x = M.m[0][2];
    planes[4].y = M.m[1][2];
    planes[4].z = M.m[2][2];
    planes[4].w = M.m[3][2];

    //
    // Far
    //
    planes[5].x = M.m[0][3] - M.m[0][2];
    planes[5].y = M.m[1][3] - M.m[1][2];
    planes[5].z = M.m[2][3] - M.m[2][2];
    planes[5].w = M.m[3][3] - M.m[3][2];

    // Normalize the plane equations.
    for (int i = 0; i < 6; ++i)
    {
        XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
        XMStoreFloat4(&planes[i], v);
    }
}

ID3DBlob* ShaderHelper::CompileShader(LPCWSTR filename, LPCSTR entry, LPCSTR target, const D3D_SHADER_MACRO* defines)
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

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11VertexShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines, ID3D11InputLayout** inputLayout, const D3D11_INPUT_ELEMENT_DESC* vertexDesc, UINT vertexDescSize)
{
    ID3DBlob* compiledShader = CompileShader(filename, entry, "vs_5_0", defines);

    HR(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    HR(device->CreateInputLayout(vertexDesc, vertexDescSize, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), inputLayout));

    ReleaseCOM(compiledShader);
}

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11VertexShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines)
{
    ID3DBlob* compiledShader = CompileShader(filename, entry, "vs_5_0", defines);

    HR(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    ReleaseCOM(compiledShader);
}

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11PixelShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines)
{
    ID3DBlob* compiledShader = CompileShader(filename, entry, "ps_5_0", defines);

    HR(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    ReleaseCOM(compiledShader);
}

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11GeometryShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines)
{
    ID3D10Blob* compiledShader = CompileShader(filename, entry, "gs_5_0", defines);

    HR(device->CreateGeometryShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    ReleaseCOM(compiledShader);
}

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11HullShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines)
{
    ID3D10Blob* compiledShader = CompileShader(filename, entry, "hs_5_0", defines);

    HR(device->CreateHullShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    ReleaseCOM(compiledShader);
}

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11DomainShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines)
{
    ID3D10Blob* compiledShader = CompileShader(filename, entry, "ds_5_0", defines);

    HR(device->CreateDomainShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    ReleaseCOM(compiledShader);
}

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11ComputeShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines)
{
    ID3D10Blob* compiledShader = CompileShader(filename, entry, "cs_5_0", defines);

    HR(device->CreateComputeShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, shader));

    ReleaseCOM(compiledShader);
}

std::wstring ExePath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos + 1);
}

ID3D11ShaderResourceView* D3DHelper::CreateTexture2DArraySRV(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<std::wstring>& filenames)
{
    UINT size = filenames.size();

    std::vector<ID3D11Texture2D*> srcTex(size);
    for (UINT i = 0; i < size; ++i)
    {
        HR(CreateDDSTextureFromFileEx(device, filenames[i].c_str(),
            0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0, false,
            (ID3D11Resource**)&srcTex[i], 0));
    }

    D3D11_TEXTURE2D_DESC texElementsDesc;
    srcTex[0]->GetDesc(&texElementsDesc);

    D3D11_TEXTURE2D_DESC texArrayDesc;
    texArrayDesc.Width = texElementsDesc.Width;
    texArrayDesc.Height = texElementsDesc.Height;
    texArrayDesc.MipLevels = texElementsDesc.MipLevels;
    texArrayDesc.ArraySize = filenames.size();
    texArrayDesc.Format = texElementsDesc.Format;
    texArrayDesc.SampleDesc.Count = 1;
    texArrayDesc.SampleDesc.Quality = 0;
    texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags = 0;
    texArrayDesc.MiscFlags = 0;

    ID3D11Texture2D* texArray = 0;
    HR(device->CreateTexture2D(&texArrayDesc, 0, &texArray));

    for (UINT texElement = 0; texElement < size; ++texElement)
    {
        for (UINT mipLevel = 0; mipLevel < texElementsDesc.MipLevels; ++mipLevel)
        {
            D3D11_MAPPED_SUBRESOURCE mappedTex2D;
            HR(deviceContext->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

            deviceContext->UpdateSubresource(
                texArray,
                D3D11CalcSubresource(mipLevel, texElement, texElementsDesc.MipLevels),
                0,
                mappedTex2D.pData,
                mappedTex2D.RowPitch,
                mappedTex2D.DepthPitch);

            deviceContext->Unmap(srcTex[texElement], mipLevel);
        }
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = texArrayDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MostDetailedMip = 0;
    viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
    viewDesc.Texture2DArray.FirstArraySlice = 0;
    viewDesc.Texture2DArray.ArraySize = size;

    ID3D11ShaderResourceView* srv;
    HR(device->CreateShaderResourceView(texArray, &viewDesc, &srv));

    ReleaseCOM(texArray);

    for (UINT i = 0; i < size; ++i)
        ReleaseCOM(srcTex[i]);

    return srv;
}

ID3D11ShaderResourceView* D3DHelper::CreateRandomTexture1DSRV(ID3D11Device* device)
{
    //
    // Create the randam data.
    //
    XMFLOAT4 randomValues[1024];

    for (int i = 0; i < 1024; ++i)
    {
        randomValues[i].x = MathHelper::RandF(-1.0f, 1.0f);
        randomValues[i].y = MathHelper::RandF(-1.0f, 1.0f);
        randomValues[i].z = MathHelper::RandF(-1.0f, 1.0f);
        randomValues[i].w = MathHelper::RandF(-1.0f, 1.0f);
    }

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
    initData.SysMemPitch = 1024 * sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

    //
    // Create the texture.
    //
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

    ID3D11Texture1D* randomTex = 0;
    HR(device->CreateTexture1D(&texDesc, &initData, &randomTex));

    //
    // Create the resource view.
    //
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
    viewDesc.Texture1D.MostDetailedMip = 0;

    ID3D11ShaderResourceView* randomTexSRV = 0;
    HR(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

    return randomTexSRV;
}