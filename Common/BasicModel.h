#pragma once

#include "d3dUtil.h"
#include "TextuerMgr.h"
#include "M3DLoader.h"
#include "MeshGeometry.h"

class BasicModel
{
public:
    BasicModel(ID3D11Device* device,
        TextureMgr& texMgr,
        const std::wstring& modelFilename,
        const std::wstring& texturePath);
    ~BasicModel();

    UINT subsetCount;

    std::vector<Material> Mat;
    std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
    std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

    // Keep CPU copies of the mesh data to read from.
    std::vector<M3dVertex> Vertices;
    std::vector<USHORT> Indices;
    std::vector<MeshGeometry::Subset> Subsets;

    MeshGeometry ModelMesh;
};

struct BasicModelInstance
{
    BasicModel* Model;
    XMFLOAT4X4 World;
};