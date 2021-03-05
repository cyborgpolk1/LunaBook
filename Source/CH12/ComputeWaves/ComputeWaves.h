#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include "d3dUtil.h"

using namespace DirectX;

class ComputeWaves
{
public:
    ComputeWaves();
    ~ComputeWaves();

    UINT RowCount() const;
    UINT ColumnCount() const;
    UINT VertexCount() const;
    UINT TriangleCount() const;
    float Width() const;
    float Depth() const;
    float SpatialStep() const;
    ID3D11ShaderResourceView* GetDisplacementMap() const;

    void Init(ID3D11Device* device, UINT m, UINT n, float dx, float dy, float speed, float damping);
    void Update(ID3D11DeviceContext* dc, float dt);
    void Disturb(ID3D11DeviceContext* dc, UINT i, UINT j, float magnitude);

private:
    UINT mNumRows;
    UINT mNumCols;

    UINT mVertexCount;
    UINT mTriangleCount;

    float mK[4];

    float mTimeStep;
    float mSpatialStep;

    ID3D11ComputeShader* mUpdateCS;
    ID3D11ComputeShader* mDisturbCS;

    ID3D11ShaderResourceView* mSRVs[3];
    ID3D11UnorderedAccessView* mUAVs[3];

    int mPrevSRV, mCurrSRV;
    int mCurrUAV, mNextUAV;

    ID3D11Buffer* mUpdateSettingsCB;
    ID3D11Buffer* mDisturbSettingsCB;

    struct alignas(16) DisturbSettings
    {
        XMINT2 coords;
        float magnitude;
    };
};