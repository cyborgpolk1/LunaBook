#include "ComputeAddTexture.h"
#include <d3dcompiler.h>
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include <vector>

D3DMAIN(ComputeAddTexture);

ComputeAddTexture::ComputeAddTexture(HINSTANCE hInstance)
    :D3DApp(hInstance), mQuadVB(0), mQuadIB(0), mQuadIndexCount(0),
    mVS(0), mPS(0), mCS(0), mInputLayout(0), mConstantBuffer(0),
    mTex1(0), mTex2(0), mTexOut(0), mTexWrite(0), mSampler(0)
{
    mMainWndCaption = L"Compute Add Texture Demo";
}

ComputeAddTexture::~ComputeAddTexture()
{
    ReleaseCOM(mQuadVB);
    ReleaseCOM(mQuadIB);
    ReleaseCOM(mVS);
    ReleaseCOM(mPS);
    ReleaseCOM(mCS);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mConstantBuffer);
    ReleaseCOM(mTex1);
    ReleaseCOM(mTex2);
    ReleaseCOM(mTexOut);
    ReleaseCOM(mTexWrite);
    ReleaseCOM(mSampler);
}

bool ComputeAddTexture::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildFullScreenQuad();
    BuildFX();
    BuildTex();
    DoComputeWork();

    return true;
}

void ComputeAddTexture::OnResize()
{
    D3DApp::OnResize();
}

void ComputeAddTexture::UpdateScene(float dt)
{

}

void ComputeAddTexture::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dImmediateContext->IASetInputLayout(mInputLayout);
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mQuadIB, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

    md3dImmediateContext->PSSetShaderResources(0, 1, &mTexOut);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampler);

    md3dImmediateContext->VSSetShader(mVS, 0, 0);
    md3dImmediateContext->PSSetShader(mPS, 0, 0);

    md3dImmediateContext->DrawIndexed(mQuadIndexCount, 0, 0);

    HR(mSwapChain->Present(0, 0));
}

void ComputeAddTexture::OnMouseDown(WPARAM btnState, int x, int y)
{

}

void ComputeAddTexture::OnMouseUp(WPARAM btnState, int x, int y)
{

}

void ComputeAddTexture::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void ComputeAddTexture::BuildFullScreenQuad()
{
    GeometryGenerator::MeshData quad;
    GeometryGenerator geoGen;
    geoGen.CreateFullScreenQuad(quad);

    std::vector<Vertex> vertices(quad.Vertices.size());

    for (UINT i = 0; i < vertices.size(); ++i)
    {
        vertices[i].Pos = quad.Vertices[i].Position;
        vertices[i].Tex = quad.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.StructureByteStride = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mQuadVB));

    mQuadIndexCount = quad.Indices.size();

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mQuadIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = quad.Indices.data();
    HR(md3dDevice->CreateBuffer(&ibd, &initData, &mQuadIB));
}

void ComputeAddTexture::BuildFX()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    auto filename = ExePath().append(L"../../../Shaders/BasicTexture.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mVS, cstr, "VS", 0, &mInputLayout, vertexDesc, 2);
    ShaderHelper::CreateShader(md3dDevice, &mPS, cstr, "PS", 0);

    filename = ExePath().append(L"../../../Shaders/ComputeAddTexture.hlsl");
    cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mCS, cstr, "CS", 0);

    struct ConstantBuffer {
        XMMATRIX gWorldViewProj;
    };

    ConstantBuffer cb;
    cb.gWorldViewProj = XMMatrixIdentity();

    D3D11_BUFFER_DESC constDesc;
    constDesc.Usage = D3D11_USAGE_IMMUTABLE;
    constDesc.ByteWidth = sizeof(ConstantBuffer);
    constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constDesc.CPUAccessFlags = 0;
    constDesc.StructureByteStride = 0;
    constDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA cinitData;
    cinitData.pSysMem = &cb;
    HR(md3dDevice->CreateBuffer(&constDesc, &cinitData, &mConstantBuffer));
}

void ComputeAddTexture::BuildTex()
{
    ID3D11Texture2D* tex1;
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/darkbrickdxt1.dds").c_str(), (ID3D11Resource**)&tex1, &mTex1));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/flarealpha.dds").c_str(), nullptr, &mTex2));

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampler));

    // RWTextures
    D3D11_TEXTURE2D_DESC texDesc;
    tex1->GetDesc(&texDesc);

    D3D11_TEXTURE2D_DESC outDesc;
    outDesc.Width = texDesc.Width;
    outDesc.Height = texDesc.Height;
    outDesc.MipLevels = 1;
    outDesc.ArraySize = 1;
    outDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    outDesc.SampleDesc.Count = 1;
    outDesc.SampleDesc.Quality = 0;
    outDesc.Usage = D3D11_USAGE_DEFAULT;
    outDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    outDesc.CPUAccessFlags = 0;
    outDesc.MiscFlags = 0;

    ID3D11Texture2D* outTex = 0;
    HR(md3dDevice->CreateTexture2D(&outDesc, 0, &outTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = outDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    HR(md3dDevice->CreateShaderResourceView(outTex, &srvDesc, &mTexOut));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = outDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    HR(md3dDevice->CreateUnorderedAccessView(outTex, &uavDesc, &mTexWrite));

    ReleaseCOM(outTex);
    ReleaseCOM(tex1);
}

void ComputeAddTexture::DoComputeWork()
{
    md3dImmediateContext->CSSetShaderResources(0, 1, &mTex1);
    md3dImmediateContext->CSSetShaderResources(1, 1, &mTex2);
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &mTexWrite, 0);

    md3dImmediateContext->CSSetShader(mCS, 0, 0);

    md3dImmediateContext->Dispatch(32, 32, 1);

    md3dImmediateContext->CSSetShader(0, 0, 0);

    //Unbind
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
}