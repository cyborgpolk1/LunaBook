#include "Terrain.h"
#include "Camera.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "DDSTextureLoader.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#include <DirectXPackedVector.h>

using namespace PackedVector;

Terrain::Terrain()
    : mLayerMapArraySRV(0), mBlendMapSRV(0), mHeightMapSRV(0),
    mQuadPatchVB(0), mQuadPatchIB(0), mSamplerLinear(0), mSamplerPoint(0),
    mVS(0), mHS(0), mDS(0), mPS(0), mInputLayout(0), 
    mPerFrameBuffer(0), mPerObjectBuffer(0),
    mNumPatchVertices(0), mNumPatchQuadFaces(0), mNumPatchVertRows(0), mNumPatchVertCols(0)
{
    XMStoreFloat4x4(&mWorld, XMMatrixIdentity());

    mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
    mMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

Terrain::~Terrain()
{
    ReleaseCOM(mLayerMapArraySRV);
    ReleaseCOM(mBlendMapSRV);
    ReleaseCOM(mHeightMapSRV);
    ReleaseCOM(mQuadPatchVB);
    ReleaseCOM(mQuadPatchIB);
    ReleaseCOM(mSamplerLinear);
    ReleaseCOM(mSamplerPoint);
    ReleaseCOM(mVS);
    ReleaseCOM(mHS);
    ReleaseCOM(mDS);
    ReleaseCOM(mPS);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mPerObjectBuffer);
}

float Terrain::GetWidth() const
{
    // Total terrain width.
    return (mInfo.HeightmapWidth - 1) * mInfo.CellSpacing;
}

float Terrain::GetDepth() const
{
    // Total terrain depth.
    return (mInfo.HeightmapHeight - 1) * mInfo.CellSpacing;
}

float Terrain::GetHeight(float x, float z) const
{
    // Transform from terrain local space to "cell" space.
    float c = (x + 0.5f * GetWidth()) / mInfo.CellSpacing;
    float d = (z - 0.5f * GetDepth()) / -mInfo.CellSpacing;

    // Get the row and column we are in.
    int row = (int)floorf(d);
    int col = (int)floorf(c);
    
    // Grab the heights of the cell we are in.
    // A*--*B
    //  | /|
    //  |/ |
    // C*--*D
    float A = mHeightmap[row * mInfo.HeightmapWidth + col];
    float B = mHeightmap[row * mInfo.HeightmapWidth + col + 1];
    float C = mHeightmap[(row + 1) * mInfo.HeightmapWidth + col];
    float D = mHeightmap[(row + 1) * mInfo.HeightmapWidth + col + 1];

    // Where we are relative to the cell.
    float s = c - (float)col;
    float t = d - (float)row;

    // If upper triangle ABC.
    if (s + t <= 1.0f)
    {
        float uy = B - A;
        float vy = C - A;
        return A + s * uy + t * vy;
    }
    else // lower triangle DCB.
    {
        float uy = C - D;
        float vy = B - D;
        return D + (1.0f - s)*uy + (1.0f - t)*vy;
    }
}

XMMATRIX Terrain::GetWorld() const
{
    return XMLoadFloat4x4(&mWorld);
}

void Terrain::SetWorld(CXMMATRIX M)
{
    XMStoreFloat4x4(&mWorld, M);
}

void Terrain::Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo)
{
    mInfo = initInfo;

    // Divide heightmap into patches such that each patch has CellsPerPath.
    mNumPatchVertRows = ((mInfo.HeightmapHeight - 1) / CellsPerPatch) + 1;
    mNumPatchVertCols = ((mInfo.HeightmapWidth - 1) / CellsPerPatch) + 1;

    mNumPatchVertices = mNumPatchVertRows * mNumPatchVertCols;
    mNumPatchQuadFaces = (mNumPatchVertRows - 1) * (mNumPatchVertCols - 1);

    LoadHeightmap();
    Smooth();
    CalcAllPatchBoundsY();

    BuildQuadPatchVB(device);
    BuildQuadPatchIB(device);
    BuildHeightmapSRV(device);

    std::vector<std::wstring> layerFilenames;
    layerFilenames.push_back(mInfo.LayerMapFilename0);
    layerFilenames.push_back(mInfo.LayerMapFilename1);
    layerFilenames.push_back(mInfo.LayerMapFilename2);
    layerFilenames.push_back(mInfo.LayerMapFilename3);
    layerFilenames.push_back(mInfo.LayerMapFilename4);
    mLayerMapArraySRV = D3DHelper::CreateTexture2DArraySRV(device, dc, layerFilenames);

    HR(CreateDDSTextureFromFile(device, mInfo.BlendMapFilename.c_str(), nullptr, &mBlendMapSRV));

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

    HR(device->CreateSamplerState(&samplerDesc, &mSamplerLinear));

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    HR(device->CreateSamplerState(&samplerDesc, &mSamplerPoint));

    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    auto fxFile = ExePath().append(L"../../../Shaders/Terrain.hlsl");

    ShaderHelper::CreateShader(device, &mVS, fxFile.c_str(), "VS", 0, &mInputLayout, vertexDesc, 3);
    ShaderHelper::CreateShader(device, &mHS, fxFile.c_str(), "HS", 0);
    ShaderHelper::CreateShader(device, &mDS, fxFile.c_str(), "DS", 0);
    ShaderHelper::CreateShader(device, &mPS, fxFile.c_str(), "PS", 0);

    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    HR(device->CreateBuffer(&matrixBufferDesc, 0, &mPerFrameBuffer));

    matrixBufferDesc.ByteWidth = sizeof(PerObjectBuffer);

    HR(device->CreateBuffer(&matrixBufferDesc, 0, &mPerObjectBuffer));
}

void Terrain::Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3])
{
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    dc->IASetInputLayout(mInputLayout);

    UINT stride = sizeof(TerrainVertex);
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
    dc->IASetIndexBuffer(mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0);

    XMMATRIX viewProj = cam.ViewProj();
    
    XMFLOAT4 worldPlanes[6];
    ExtractFrustumPlanes(worldPlanes, viewProj);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(dc->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* frameDataPtr = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);
    frameDataPtr->DirLights[0] = lights[0];
    frameDataPtr->DirLights[1] = lights[1];
    frameDataPtr->DirLights[2] = lights[2];
    frameDataPtr->EyePosW = cam.GetPosition();
    
    frameDataPtr->FogStart = 0.0f;
    frameDataPtr->FogRange = 0.0f;
    //XMStoreFloat4(&frameDataPtr->FogColor, Colors::Silver);
    frameDataPtr->FogColor = XMFLOAT4(0.75f, 1.0f, 1.0f, 1.0f);

    frameDataPtr->MinDist = 20.0f;
    frameDataPtr->MaxDist = 500.0f;

    frameDataPtr->MinTess = 0.0f;
    frameDataPtr->MaxTess = 6.0f;

    frameDataPtr->TexelCellSpaceU = 1.0f / mInfo.HeightmapWidth;
    frameDataPtr->TexelCellSpaceV = 1.0f / mInfo.HeightmapHeight;

    frameDataPtr->WorldCellSpace = mInfo.CellSpacing;

    frameDataPtr->TexScale = XMFLOAT2(50.0f, 50.0f);

    frameDataPtr->WorldFrustumPlanes[0] = worldPlanes[0];
    frameDataPtr->WorldFrustumPlanes[1] = worldPlanes[1];
    frameDataPtr->WorldFrustumPlanes[2] = worldPlanes[2];
    frameDataPtr->WorldFrustumPlanes[3] = worldPlanes[3];
    frameDataPtr->WorldFrustumPlanes[4] = worldPlanes[4];
    frameDataPtr->WorldFrustumPlanes[5] = worldPlanes[5];

    dc->Unmap(mPerFrameBuffer, 0);

    HR(dc->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerObjectBuffer* objectDataPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);

    objectDataPtr->ViewProj = XMMatrixTranspose(viewProj);
    objectDataPtr->Mat = mMat;

    dc->Unmap(mPerObjectBuffer, 0);

    dc->VSSetShaderResources(2, 1, &mHeightMapSRV);
    dc->VSSetSamplers(1, 1, &mSamplerPoint);
    dc->VSSetShader(mVS, 0, 0);

    dc->HSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    dc->HSSetShader(mHS, 0, 0);

    dc->DSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    dc->DSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    dc->DSSetShaderResources(2, 1, &mHeightMapSRV);
    dc->DSSetSamplers(1, 1, &mSamplerPoint);
    dc->DSSetShader(mDS, 0, 0);

    dc->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);
    dc->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    dc->PSSetShaderResources(0, 1, &mLayerMapArraySRV);
    dc->PSSetShaderResources(1, 1, &mBlendMapSRV);
    dc->PSSetShaderResources(2, 1, &mHeightMapSRV);
    dc->PSSetSamplers(0, 1, &mSamplerLinear);
    dc->PSSetSamplers(1, 1, &mSamplerPoint);
    dc->PSSetShader(mPS, 0, 0);

    dc->DrawIndexed(mNumPatchQuadFaces * 4, 0, 0);

    dc->VSSetShader(0, 0, 0);
    dc->HSSetShader(0, 0, 0);
    dc->DSSetShader(0, 0, 0);
    dc->PSSetShader(0, 0, 0);
}

void Terrain::LoadHeightmap()
{
    // A height for each vertex
    std::vector<unsigned char> in(mInfo.HeightmapWidth * mInfo.HeightmapHeight);

    // Open the file.
    std::ifstream inFile;
    inFile.open(mInfo.HeightMapFilename.c_str(), std::ios_base::binary);

    if (inFile)
    {
        // Read the RAW bytes.
        inFile.read((char*)&in[0], (std::streamsize)in.size());

        // Done with file.
        inFile.close();
    }

    // Copy the array data into a float array and scale it.
    mHeightmap.resize(mInfo.HeightmapHeight * mInfo.HeightmapWidth, 0);
    for (UINT i = 0; i < mInfo.HeightmapHeight * mInfo.HeightmapWidth; ++i)
    {
        mHeightmap[i] = (in[i] / 255.0f) * mInfo.HeightScale;
    }
}

bool Terrain::InBounds(int i, int j)
{
    // True if ij are valid indices; false otherwise.
    return
        i >= 0 && i < (int)mInfo.HeightmapHeight &&
        j >= 0 && j < (int)mInfo.HeightmapWidth;
}

float Terrain::Average(int i, int j)
{
    // Function computes the average height of the ij element.
    // It averages itself with its eight neighbor pixels.
    // Note that if a pixel is missing neighbor, we just don't
    // include it in the average--that is, edge pixels don't 
    // have a neighbor pixel.
    //
    // ----------
    // | 1| 2| 3|
    // ----------
    // |4 |ij| 6|
    // ----------
    // | 7| 8| 9|
    // ----------

    float avg = 0.0f;
    float num = 0.0f;

    // Use int to allow negatives.
    for (int m = i - 1; m <= i + 1; ++m)
    {
        for (int n = j - 1; n <= j + 1; ++n)
        {
            if (InBounds(m, n))
            {
                avg += mHeightmap[m * mInfo.HeightmapWidth + n];
                num += 1.0f;
            }
        }
    }

    return avg / num;
}

void Terrain::Smooth()
{
    std::vector<float> dest(mHeightmap.size());

    for (UINT i = 0; i < mInfo.HeightmapHeight; ++i)
    {
        for (UINT j = 0; j < mInfo.HeightmapWidth; ++j)
        {
            dest[i * mInfo.HeightmapWidth + j] = Average(i, j);
        }
    }

    // Replace the old heightmap with the filtered one.
    mHeightmap = dest;
}

void Terrain::BuildHeightmapSRV(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = mInfo.HeightmapWidth;
    texDesc.Height = mInfo.HeightmapHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    std::vector<HALF> hmap(mHeightmap.size());
    std::transform(mHeightmap.begin(), mHeightmap.end(), hmap.begin(), XMConvertFloatToHalf);

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = hmap.data();
    data.SysMemPitch = mInfo.HeightmapWidth * sizeof(HALF);
    data.SysMemSlicePitch = 0;

    ID3D11Texture2D* hmapTex = 0;
    HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;
    HR(device->CreateShaderResourceView(hmapTex, &srvDesc, &mHeightMapSRV));

    ReleaseCOM(hmapTex);
}

void Terrain::BuildQuadPatchVB(ID3D11Device* device)
{
    std::vector<TerrainVertex> patchVertices(mNumPatchVertRows * mNumPatchVertCols);

    float halfWidth = 0.5f * GetWidth();
    float halfDepth = 0.5f * GetDepth();

    float patchWidth = GetWidth() / (mNumPatchVertCols - 1);
    float patchDepth = GetDepth() / (mNumPatchVertRows - 1);
    float du = 1.0f / (mNumPatchVertCols - 1);
    float dv = 1.0f / (mNumPatchVertRows - 1);

    for (UINT i = 0; i < mNumPatchVertRows; ++i)
    {
        float z = halfDepth - i * patchDepth;
        for (UINT j = 0; j < mNumPatchVertCols; ++j)
        {
            float x = -halfWidth + j * patchWidth;

            patchVertices[i*mNumPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);

            // Stretch texture over grid.
            patchVertices[i*mNumPatchVertCols + j].Tex.x = j * du;
            patchVertices[i*mNumPatchVertCols + j].Tex.y = i * dv;
        }
    }

    // Store axis-aligned bounding box y-bounds in upper-left patch corner.
    for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
    {
        for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
        {
            UINT patchID = i * (mNumPatchVertCols - 1) + j;
            patchVertices[i*mNumPatchVertCols + j].BoundsY = mPatchBoundsY[patchID];
        }
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(TerrainVertex) * patchVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.StructureByteStride = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = patchVertices.data();
    HR(device->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
}

void Terrain::BuildQuadPatchIB(ID3D11Device* device)
{
    std::vector<USHORT> indices(mNumPatchQuadFaces * 4);
    
    int k = 0;
    for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
    {
        for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
        {
            // Top row of 2x2 quad patch
            indices[k] = i * mNumPatchVertCols + j;
            indices[k + 1] = i * mNumPatchVertCols + j + 1;

            // Bottom row of 2x2 quad patch
            indices[k + 2] = (i + 1) * mNumPatchVertCols + j;
            indices[k + 3] = (i + 1) * mNumPatchVertCols + j + 1;

            k += 4;
        }
    }

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(USHORT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = indices.data();
    HR(device->CreateBuffer(&ibd, &initData, &mQuadPatchIB));
}

void Terrain::CalcAllPatchBoundsY()
{
    mPatchBoundsY.resize(mNumPatchQuadFaces);

    // For each patch
    for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
    {
        for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
        {
            CalcPatchBoundsY(i, j);
        }
    }
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j)
{
    // Scan the heightmap values this patch covers and
    // compute the min/max height.

    UINT x0 = j * CellsPerPatch;
    UINT x1 = (j + 1) * CellsPerPatch;

    UINT y0 = i * CellsPerPatch;
    UINT y1 = (i + 1) * CellsPerPatch;

    float minY = MathHelper::Infinity;
    float maxY = -MathHelper::Infinity;
    for (UINT y = y0; y <= y1; ++y)
    {
        for (UINT x = x0; x <= x1; ++x)
        {
            UINT k = y * mInfo.HeightmapWidth + x;
            minY = MathHelper::Min(minY, mHeightmap[k]);
            maxY = MathHelper::Max(maxY, mHeightmap[k]);
        }
    }

    UINT patchID = i * (mNumPatchVertCols - 1) + j;
    mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}