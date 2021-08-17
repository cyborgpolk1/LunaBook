#pragma once

#include "d3dUtil.h"
#include "TextuerMgr.h"
#include "M3DLoader.h"

class SkinnedModel
{
public:
    SkinnedModel(ID3D11Device* device, TextureMgr& texMgr,
        const std::wstring& modelFilename, const std::wstring& texturePath);
    ~SkinnedModel();

    UINT subsetCount;

    std::vector<Material> Mat;
    std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
    std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

    // Keep CPU copies of the mesh data to read from.
    std::vector<SkinnedVertex> Vertices;
    std::vector<USHORT> Indices;
    std::vector<MeshGeometry::Subset> Subsets;

    MeshGeometry ModelMesh;
    SkinnedData SkinnedData;
};

struct SkinnedModelInstance
{
    SkinnedModel* Model;
    float TimePos;
    std::string ClipName;
    XMFLOAT4X4 World;
    std::vector<XMFLOAT4X4> FinalTransforms;

    void Update(float dt);
};