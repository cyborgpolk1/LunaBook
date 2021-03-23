#include "Sky.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include <vector>

Sky::Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius)
{
    //
    // Create Shaders
    //
    D3D11_INPUT_ELEMENT_DESC vertexDesc[1] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ShaderHelper::CreateShader(device, &mVS, ExePath().append(L"../../../Shaders/SkyBox.hlsl").c_str(), "VS", 0, &mInputLayout, vertexDesc, 1);
    ShaderHelper::CreateShader(device, &mPS, ExePath().append(L"../../../Shaders/SkyBox.hlsl").c_str(), "PS", 0);

    //
    // Create Constant Buffer
    //
    D3D11_BUFFER_DESC matrixDesc;
    matrixDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixDesc.ByteWidth = sizeof(MatrixBuffer);
    matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixDesc.StructureByteStride = 0;
    matrixDesc.MiscFlags = 0;

    HR(device->CreateBuffer(&matrixDesc, 0, &mMatrixBuffer));

    //
    // Create Texture Resources
    //
    CreateDDSTextureFromFile(device, cubemapFilename.c_str(), 0, &mCubeMapSRV);

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HR(device->CreateSamplerState(&samplerDesc, &mCubeMapSS));

    //
    // Create Dome
    //
    GeometryGenerator::MeshData sphere;
    GeometryGenerator geoGen;
    geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

    std::vector<XMFLOAT3> vertices(sphere.Vertices.size());

    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i] = sphere.Vertices[i].Position;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices.data();
    HR(device->CreateBuffer(&vbd, &vinitData, &mVB));

    mIndexCount = sphere.Indices.size();

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(size_t) * mIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = sphere.Indices.data();
    HR(device->CreateBuffer(&ibd, &initData, &mIB));

    //
    // Create Rasterizer and Depth-Stencil States
    //
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    HR(device->CreateDepthStencilState(&dsDesc, &mDDS));

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.CullMode = D3D11_CULL_NONE;
    HR(device->CreateRasterizerState(&rsDesc, &mRS));
}

Sky::~Sky()
{
    ReleaseCOM(mVB);
    ReleaseCOM(mIB);
    ReleaseCOM(mMatrixBuffer);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mCubeMapSRV);
    ReleaseCOM(mCubeMapSS);
    ReleaseCOM(mVS);
    ReleaseCOM(mPS);
    ReleaseCOM(mRS);
    ReleaseCOM(mDDS);
}

ID3D11ShaderResourceView* Sky::CubeMapSRV()
{
    return mCubeMapSRV;
}

void Sky::Draw(ID3D11DeviceContext* dc, const Camera& camera)
{
    // center Sky about eye in world space
    XMFLOAT3 eyePos = camera.GetPosition();
    XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);

    XMMATRIX WVP = XMMatrixMultiply(T, camera.ViewProj());

    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
    dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
    dc->IASetInputLayout(mInputLayout);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    dc->VSSetShader(mVS, 0, 0);
    dc->PSSetShader(mPS, 0, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(dc->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    MatrixBuffer* dataPtr = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
    dataPtr->WorldViewProj = XMMatrixTranspose(WVP);

    dc->Unmap(mMatrixBuffer, 0);
    dc->VSSetConstantBuffers(0, 1, &mMatrixBuffer);

    dc->PSSetShaderResources(0, 1, &mCubeMapSRV);
    dc->PSSetSamplers(0, 1, &mCubeMapSS);

    dc->RSSetState(mRS);
    dc->OMSetDepthStencilState(mDDS, 0);

    dc->DrawIndexed(mIndexCount, 0, 0);

    dc->VSSetShader(0, 0, 0);
    dc->PSSetShader(0, 0, 0);
    
    dc->RSSetState(0);
    dc->OMSetDepthStencilState(0, 0);
}