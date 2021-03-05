#include "ComputeWaves.h"
#include <cassert>

ComputeWaves::ComputeWaves()
    : mNumRows(0), mNumCols(0), mVertexCount(0), mTriangleCount(0),
    mTimeStep(0.0f), mSpatialStep(0.0f),
    mUpdateCS(0), mDisturbCS(0), 
    mPrevSRV(0), mCurrSRV(1), mCurrUAV(1), mNextUAV(2),
    mUpdateSettingsCB(0), mDisturbSettingsCB(0)
{
    for (UINT i = 0; i < 3; ++i)
    {
        mSRVs[i] = 0;
        mUAVs[i] = 0;
        mK[i] = 0.0f;
    }
}

ComputeWaves::~ComputeWaves()
{
    ReleaseCOM(mUpdateCS);
    ReleaseCOM(mDisturbCS);

    for (UINT i = 0; i < 3; ++i)
    {
        ReleaseCOM(mSRVs[i]);
        ReleaseCOM(mUAVs[i]);
    }

    ReleaseCOM(mUpdateSettingsCB);
    ReleaseCOM(mDisturbSettingsCB);
}

UINT ComputeWaves::RowCount() const { return mNumRows; }

UINT ComputeWaves::ColumnCount() const { return mNumCols; }

UINT ComputeWaves::VertexCount() const { return mVertexCount; }

UINT ComputeWaves::TriangleCount() const { return mTriangleCount; }

float ComputeWaves::Width() const { return mNumCols * mSpatialStep; }

float ComputeWaves::Depth() const { return mNumRows * mSpatialStep; }

float ComputeWaves::SpatialStep() const { return mSpatialStep; }

ID3D11ShaderResourceView* ComputeWaves::GetDisplacementMap() const { return mSRVs[mCurrSRV]; }

void ComputeWaves::Init(ID3D11Device* device, UINT m, UINT n, float dx, float dt, float speed, float damping)
{
    mNumRows = m;
    mNumCols = n;

    mVertexCount = m * n;
    mTriangleCount = (m - 1) * (n - 1) * 2;

    mTimeStep = dt;
    mSpatialStep = dx;
    
    float d = damping * dt + 2.0f;
    float e = (speed * speed) * (dt * dt) / (dx * dx);
    mK[0] = (damping * dt - 2.0f) / d;
    mK[1] = (4.0f - 8.0f * e) / d;
    mK[2] = (2.0f * e) / d;
    mK[3] = 0;

    ReleaseCOM(mUpdateSettingsCB);

    D3D11_BUFFER_DESC updateDesc;
    updateDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    updateDesc.ByteWidth = sizeof(float) * 4; // padding
    updateDesc.CPUAccessFlags = 0;
    updateDesc.MiscFlags = 0;
    updateDesc.StructureByteStride = 0;
    updateDesc.Usage = D3D11_USAGE_IMMUTABLE;

    D3D11_SUBRESOURCE_DATA updateInit;
    updateInit.pSysMem = mK;
    HR(device->CreateBuffer(&updateDesc, &updateInit, &mUpdateSettingsCB));

    if (!mDisturbSettingsCB)
    {
        updateDesc.ByteWidth = sizeof(DisturbSettings);
        updateDesc.Usage = D3D11_USAGE_DYNAMIC;
        updateDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HR(device->CreateBuffer(&updateDesc, 0, &mDisturbSettingsCB));
    }


    auto filename = ExePath().append(L"../../../Shaders/ComputeWaves.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(device, &mUpdateCS, cstr, "UpdateCS", 0);
    ShaderHelper::CreateShader(device, &mDisturbCS, cstr, "DisturbCS", 0);

    D3D11_TEXTURE2D_DESC wavesTexDesc;
    wavesTexDesc.Width = mNumRows;
    wavesTexDesc.Height = mNumCols;
    wavesTexDesc.MipLevels = 1;
    wavesTexDesc.ArraySize = 1;
    wavesTexDesc.Format = DXGI_FORMAT_R32_FLOAT;
    wavesTexDesc.SampleDesc.Count = 1;
    wavesTexDesc.SampleDesc.Quality = 0;
    wavesTexDesc.Usage = D3D11_USAGE_DEFAULT;
    wavesTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    wavesTexDesc.CPUAccessFlags = 0;
    wavesTexDesc.MiscFlags = 0;

    for (UINT i = 0; i < 3; ++i)
    {
        ReleaseCOM(mSRVs[i]);
        ReleaseCOM(mUAVs[i]);

        ID3D11Texture2D* wavesTex = 0;
        HR(device->CreateTexture2D(&wavesTexDesc, 0, &wavesTex));

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        HR(device->CreateShaderResourceView(wavesTex, 0, &mSRVs[i]));

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;

        HR(device->CreateUnorderedAccessView(wavesTex, 0, &mUAVs[i]));

        ReleaseCOM(wavesTex);
    }

    mPrevSRV = 0;
    mCurrSRV = 1;

    mCurrSRV = 1;
    mNextUAV = 2;
}

void ComputeWaves::Update(ID3D11DeviceContext* dc, float dt)
{
    static float t = 0;

    t += dt;

    if (t >= mTimeStep)
    {
        ID3D11UnorderedAccessView* nullUAV = nullptr;
        ID3D11ShaderResourceView* nullSRV = nullptr;
        ID3D11Buffer* nullBuffer = nullptr;

        dc->CSSetConstantBuffers(0, 1, &mUpdateSettingsCB);
        dc->CSSetShaderResources(0, 1, &mSRVs[mPrevSRV]);
        dc->CSSetShaderResources(1, 1, &mSRVs[mCurrSRV]);
        dc->CSSetUnorderedAccessViews(1, 1, &mUAVs[mNextUAV], 0);
        dc->CSSetShader(mUpdateCS, 0, 0);

        dc->Dispatch((UINT)ceilf(mNumRows / 16.0f), (UINT)ceilf(mNumCols / 16.0f), 1);

        dc->CSSetConstantBuffers(0, 1, &nullBuffer);
        dc->CSSetShaderResources(0, 1, &nullSRV);
        dc->CSSetShaderResources(1, 1, &nullSRV);
        dc->CSSetUnorderedAccessViews(1, 1, &nullUAV, 0);
        dc->CSSetShader(0, 0, 0);

        mPrevSRV = mCurrSRV;
        mCurrSRV = mNextUAV;
        mCurrUAV = mCurrSRV;
        mNextUAV = (mNextUAV + 1) % 3;

        t = 0.0f;
    }
}

void ComputeWaves::Disturb(ID3D11DeviceContext* dc, UINT i, UINT j, float magnitude)
{
    // Don't disturb boundaries
    assert(i > 1 && i < mNumRows - 2);
    assert(j > 1 && j < mNumCols - 2);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(dc->Map(mDisturbSettingsCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    DisturbSettings* dataPtr = reinterpret_cast<DisturbSettings*>(mappedResource.pData);
    dataPtr->coords = XMINT2(i, j);
    dataPtr->magnitude = magnitude;

    dc->Unmap(mDisturbSettingsCB, 0);

    ID3D11UnorderedAccessView* nullUAV = nullptr;
    ID3D11Buffer* nullBuffer = nullptr;

    dc->CSSetConstantBuffers(1, 1, &mDisturbSettingsCB);
    dc->CSSetUnorderedAccessViews(0, 1, &mUAVs[mCurrUAV], 0);
    dc->CSSetShader(mDisturbCS, 0, 0);

    dc->Dispatch((UINT)ceilf(mNumRows / 16.0f), (UINT)ceilf(mNumCols / 16.0f), 1);

    dc->CSSetConstantBuffers(1, 1, &nullBuffer);
    dc->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
    dc->CSSetShader(0, 0, 0);
}