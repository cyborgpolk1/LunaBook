#include "ComputeAddVector.h"
#include <d3dcompiler.h>
#include <vector>
#include <fstream>

D3DMAIN(ComputeAddVector);

ComputeAddVector::ComputeAddVector(HINSTANCE hInstance)
    :D3DApp(hInstance), mCS(0), mInputA(0), mInputB(0), mOutput(0), mOutputBuffer(0), mOutputDebugBuffer(0)
{
    mMainWndCaption = L"Compute Add Vector";
}

ComputeAddVector::~ComputeAddVector()
{
    ReleaseCOM(mCS);
    ReleaseCOM(mInputA);
    ReleaseCOM(mInputB);
    ReleaseCOM(mOutput);
    ReleaseCOM(mOutputBuffer);
    ReleaseCOM(mOutputDebugBuffer);
}

bool ComputeAddVector::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildData();
    BuildFX();
    DoComputeWork();

    return true;
}

void ComputeAddVector::OnResize()
{
    D3DApp::OnResize();
}

void ComputeAddVector::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    HR(mSwapChain->Present(0, 0));
}

void ComputeAddVector::BuildData()
{
    std::vector<Data> dataA(mNumElements);
    std::vector<Data> dataB(mNumElements);
    for (int i = 0; i < mNumElements; ++i)
    {
        dataA[i].v1 = XMFLOAT3(i, i, i);
        dataA[i].v2 = XMFLOAT2(i, 0);

        dataB[i].v1 = XMFLOAT3(-i, i, 0);
        dataB[i].v2 = XMFLOAT2(0, -i);
    }

    D3D11_BUFFER_DESC inputDesc;
    inputDesc.Usage = D3D11_USAGE_DEFAULT;
    inputDesc.ByteWidth = sizeof(Data) * mNumElements;
    inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    inputDesc.CPUAccessFlags = 0;
    inputDesc.StructureByteStride = sizeof(Data);
    inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA vinitDataA;
    vinitDataA.pSysMem = dataA.data();
    ID3D11Buffer* bufferA = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vinitDataA, &bufferA));

    D3D11_SUBRESOURCE_DATA vinitDataB;
    vinitDataB.pSysMem = dataB.data();
    ID3D11Buffer* bufferB = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vinitDataB, &bufferB));

    D3D11_BUFFER_DESC outputDesc;
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth = sizeof(Data) * mNumElements;
    outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags = 0;
    outputDesc.StructureByteStride = sizeof(Data);
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mOutputBuffer));

    outputDesc.Usage = D3D11_USAGE_STAGING;
    outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    outputDesc.BindFlags = 0;

    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mOutputDebugBuffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement = 0;
    srvDesc.BufferEx.Flags = 0;
    srvDesc.BufferEx.NumElements = mNumElements;

    HR(md3dDevice->CreateShaderResourceView(bufferA, &srvDesc, &mInputA));
    HR(md3dDevice->CreateShaderResourceView(bufferB, &srvDesc, &mInputB));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = mNumElements;

    HR(md3dDevice->CreateUnorderedAccessView(mOutputBuffer, &uavDesc, &mOutput));

    ReleaseCOM(bufferA);
    ReleaseCOM(bufferB);
}

void ComputeAddVector::BuildFX()
{
    auto filename = ExePath().append(L"../../../Shaders/ComputeAddVector.hlsl");
    
    ShaderHelper::CreateShader(md3dDevice, &mCS, filename.c_str(), "CS", 0);
}

void ComputeAddVector::DoComputeWork()
{
    md3dImmediateContext->CSSetShaderResources(0, 1, &mInputA);
    md3dImmediateContext->CSSetShaderResources(1, 1, &mInputB);
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &mOutput, 0);

    md3dImmediateContext->CSSetShader(mCS, 0, 0);

    md3dImmediateContext->Dispatch(1, 1, 1);

    md3dImmediateContext->CSSetShader(0, 0, 0);

    //Unbind
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    md3dImmediateContext->CSSetShaderResources(0, 1, &nullSRV);
    md3dImmediateContext->CSSetShaderResources(1, 1, &nullSRV);

    // Copy and write
    CreateDirectory(ExePath().append(L"../../../OutputFiles").c_str(), NULL);
    auto outfile = ExePath().append(L"../../../OutputFiles/AddVector.txt");
    
    std::ofstream fout(outfile.c_str());

    md3dImmediateContext->CopyResource(mOutputDebugBuffer, mOutputBuffer);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    md3dImmediateContext->Map(mOutputDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedData);

    Data* dataView = reinterpret_cast<Data*>(mappedData.pData);
    
    for (UINT i = 0; i < mNumElements; ++i)
    {
        fout << "(" << dataView[i].v1.x << ", " <<
            dataView[i].v1.y << ", " <<
            dataView[i].v1.z << ", " <<
            dataView[i].v2.x << ", " <<
            dataView[i].v2.y << ")" << std::endl;
    }

    md3dImmediateContext->Unmap(mOutputDebugBuffer, 0);

    fout.close();
}