#pragma once

#include "d3dUtil.h"

class MeshGeometry
{
public:
    struct Subset
    {
        Subset() :
            Id(-1), VertexStart(0), VertexCount(0),
            FaceStart(0), FaceCount(0)
        {}

        UINT Id;
        UINT VertexStart;
        UINT VertexCount;
        UINT FaceStart;
        UINT FaceCount;
    };

public:
    MeshGeometry();
    ~MeshGeometry();

    template <typename VertexType>
    void SetVertices(ID3D11Device* device, const VertexType* vertices, UINT count);

    void SetIndices(ID3D11Device* device, const USHORT* indices, UINT count);

    void SetSubsetTable(std::vector<Subset>& subsetTable);

    void Draw(ID3D11DeviceContext* dc, UINT subsetId);

private:
    MeshGeometry(const MeshGeometry& rhs);
    MeshGeometry& operator=(const MeshGeometry& rhs);

private:
    ID3D11Buffer* mVB;
    ID3D11Buffer* mIB;

    DXGI_FORMAT mIndexBufferFormat; // Always 16-bit
    UINT mVertexStride;

    std::vector<Subset> mSubsetTable;
};

