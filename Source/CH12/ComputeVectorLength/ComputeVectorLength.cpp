#include "ComputeVectorLength.h"
#include <d3dcompiler.h>
#include <vector>
#include <fstream>
#include <stdlib.h>

D3DMAIN(ComputeVectorLength);

ComputeVectorLength::ComputeVectorLength(HINSTANCE hInstance)
    :D3DApp(hInstance), 
    mStructCS(0), mStructOutputUAV(0), mStructOutputBuffer(0), mStructInputSRV(0), mStructDebugBuffer(0),
    mTypedCS(0), mTypedOutputUAV(0), mTypedOutputBuffer(0), mTypedInputSRV(0), mTypedDebugBuffer(0),
    mAppendCS(0), mAppendOutputUAV(0), mAppendOutputBuffer(0), mAppendInputUAV(0), mAppendDebugBuffer(0)
{
    mMainWndCaption = L"Compute Vector Length";
}

ComputeVectorLength::~ComputeVectorLength()
{
    ReleaseCOM(mStructCS);
    ReleaseCOM(mStructOutputUAV);
    ReleaseCOM(mStructOutputBuffer);
    ReleaseCOM(mStructInputSRV);
    ReleaseCOM(mStructDebugBuffer);

    ReleaseCOM(mTypedCS);
    ReleaseCOM(mTypedOutputUAV);
    ReleaseCOM(mTypedOutputBuffer);
    ReleaseCOM(mTypedInputSRV);
    ReleaseCOM(mTypedDebugBuffer);

    ReleaseCOM(mAppendCS);
    ReleaseCOM(mAppendOutputUAV);
    ReleaseCOM(mAppendOutputBuffer);
    ReleaseCOM(mAppendInputUAV);
    ReleaseCOM(mAppendDebugBuffer);
}

bool ComputeVectorLength::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildData();
    BuildFX();
    DoComputeWork();

    return true;
}

void ComputeVectorLength::OnResize()
{
    D3DApp::OnResize();
}

void ComputeVectorLength::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    HR(mSwapChain->Present(0, 0));
}

void ComputeVectorLength::BuildData()
{
    std::vector<Data> data(mNumElements);
    std::vector<XMFLOAT3> data2(mNumElements);
    for (UINT i = 0; i < mNumElements; ++i)
    {
        data[i].vec = XMFLOAT3(rand() % 100 + 1, rand() % 100 + 1, rand() % 100 + 1);
        XMVECTOR randomVec = XMLoadFloat3(&data[i].vec);
        randomVec = XMVectorScale(XMVector3Normalize(randomVec), rand() % 10 + 1);
        XMStoreFloat3(&data[i].vec, randomVec);
        XMStoreFloat3(&data2[i], randomVec);
    }

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = data.data();

    D3D11_BUFFER_DESC inputDesc;
    inputDesc.Usage = D3D11_USAGE_DEFAULT;
    inputDesc.ByteWidth = sizeof(Data) * mNumElements;
    inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    inputDesc.CPUAccessFlags = 0;
    inputDesc.StructureByteStride = sizeof(Data);
    inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    ID3D11Buffer* structInputBuffer = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vinitData, &structInputBuffer));

    inputDesc.ByteWidth = sizeof(XMFLOAT3) * mNumElements;
    inputDesc.StructureByteStride = sizeof(XMFLOAT3);
    inputDesc.MiscFlags = 0;
    vinitData.pSysMem = data2.data();
    
    ID3D11Buffer* typedInputBuffer = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vinitData, &typedInputBuffer));

    inputDesc.ByteWidth = sizeof(Data) * mNumElements;
    inputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    inputDesc.StructureByteStride = sizeof(Data);
    inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    vinitData.pSysMem = data.data();

    ID3D11Buffer* appendInputBuffer = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vinitData, &appendInputBuffer));

    D3D11_BUFFER_DESC outputDesc;
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth = sizeof(float) * mNumElements;
    outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags = 0;
    outputDesc.StructureByteStride = sizeof(float);
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mStructOutputBuffer));
    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mAppendOutputBuffer));

    outputDesc.MiscFlags = 0;
    
    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mTypedOutputBuffer));

    outputDesc.Usage = D3D11_USAGE_STAGING;
    outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    outputDesc.BindFlags = 0;
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mStructDebugBuffer));
    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mAppendDebugBuffer));

    outputDesc.MiscFlags = 0;

    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mTypedDebugBuffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement = 0;
    srvDesc.BufferEx.Flags = 0;
    srvDesc.BufferEx.NumElements = mNumElements;

    HR(md3dDevice->CreateShaderResourceView(structInputBuffer, &srvDesc, &mStructInputSRV));

    srvDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

    HR(md3dDevice->CreateShaderResourceView(typedInputBuffer, &srvDesc, &mTypedInputSRV));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = mNumElements;

    HR(md3dDevice->CreateUnorderedAccessView(mStructOutputBuffer, &uavDesc, &mStructOutputUAV));

    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

    HR(md3dDevice->CreateUnorderedAccessView(mAppendOutputBuffer, &uavDesc, &mAppendOutputUAV));
    HR(md3dDevice->CreateUnorderedAccessView(appendInputBuffer, &uavDesc, &mAppendInputUAV));

    uavDesc.Buffer.Flags = 0;
    uavDesc.Format = DXGI_FORMAT_R32_FLOAT;

    HR(md3dDevice->CreateUnorderedAccessView(mTypedOutputBuffer, &uavDesc, &mTypedOutputUAV));

    ReleaseCOM(structInputBuffer);
    ReleaseCOM(typedInputBuffer);
    ReleaseCOM(appendInputBuffer);
}

void ComputeVectorLength::BuildFX()
{
    auto filename = ExePath().append(L"../../../Shaders/ComputeVectorLength.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mStructCS, cstr, "StructCS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mTypedCS, cstr, "TypedCS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mAppendCS, cstr, "AppendCS", 0);
}

void ComputeVectorLength::DoComputeWork()
{
    md3dImmediateContext->CSSetShaderResources(0, 1, &mStructInputSRV);
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &mStructOutputUAV, 0);

    md3dImmediateContext->CSSetShaderResources(1, 1, &mTypedInputSRV);
    md3dImmediateContext->CSSetUnorderedAccessViews(1, 1, &mTypedOutputUAV, 0);

    md3dImmediateContext->CSSetUnorderedAccessViews(2, 1, &mAppendInputUAV, &mNumElements);
    md3dImmediateContext->CSSetUnorderedAccessViews(3, 1, &mAppendOutputUAV, 0);

    md3dImmediateContext->CSSetShader(mStructCS, 0, 0);
    md3dImmediateContext->Dispatch(1, 1, 1);

    md3dImmediateContext->CSSetShader(mTypedCS, 0, 0);
    md3dImmediateContext->Dispatch(1, 1, 1);

    md3dImmediateContext->CSSetShader(mAppendCS, 0, 0);
    md3dImmediateContext->Dispatch(1, 1, 1);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
    md3dImmediateContext->CSSetUnorderedAccessViews(1, 1, &nullUAV, 0);
    md3dImmediateContext->CSSetUnorderedAccessViews(2, 1, &nullUAV, 0);
    md3dImmediateContext->CSSetUnorderedAccessViews(3, 1, &nullUAV, 0);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    md3dImmediateContext->CSSetShaderResources(0, 1, &nullSRV);
    md3dImmediateContext->CSSetShaderResources(1, 1, &nullSRV);
    md3dImmediateContext->CSSetShader(0, 0, 0);

    CreateDirectory(ExePath().append(L"../../../OutputFiles").c_str(), nullptr);
    auto outfile = ExePath().append(L"../../../OutputFiles/VectorLengths.txt");

    std::ofstream fout(outfile.c_str());

    md3dImmediateContext->CopyResource(mStructDebugBuffer, mStructOutputBuffer);
    md3dImmediateContext->CopyResource(mTypedDebugBuffer, mTypedOutputBuffer);
    md3dImmediateContext->CopyResource(mAppendDebugBuffer, mAppendOutputBuffer);

    D3D11_MAPPED_SUBRESOURCE mappedResourceStruct, mappedResourceTyped, mappedResourceAppend;
    md3dImmediateContext->Map(mStructDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedResourceStruct);
    md3dImmediateContext->Map(mTypedDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedResourceTyped);
    md3dImmediateContext->Map(mAppendDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedResourceAppend);

    float* dataViewStruct = reinterpret_cast<float*>(mappedResourceStruct.pData);
    float* dataViewTyped = reinterpret_cast<float*>(mappedResourceTyped.pData);
    float* dataViewAppend = reinterpret_cast<float*>(mappedResourceAppend.pData);

    for (UINT i = 0; i < mNumElements; ++i)
    {
        fout << dataViewStruct[i] << "\t" << 
            dataViewTyped[i] << "\t" <<
            dataViewAppend[i] << std::endl;
    }

    md3dImmediateContext->Unmap(mStructDebugBuffer, 0);
    md3dImmediateContext->Unmap(mTypedDebugBuffer, 0);
    md3dImmediateContext->Unmap(mAppendDebugBuffer, 0);

    fout.close();
}