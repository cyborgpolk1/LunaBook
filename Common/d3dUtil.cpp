#include "d3dUtil.h"

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

void ShaderHelper::CreateShader(ID3D11Device* device, ID3D11VertexShader** shader, LPCWSTR filename, LPCSTR entry, const D3D_SHADER_MACRO* defines, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* vertexDesc, UINT vertexDescSize)
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