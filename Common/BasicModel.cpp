#include "BasicModel.h"

BasicModel::BasicModel(ID3D11Device* device,
    TextureMgr& texMgr,
    const std::wstring& modelFilename,
    const std::wstring& texturePath)
{
    std::vector<M3dMaterial> mats;
    M3DLoader m3dLoader;
    m3dLoader.LoadM3d(modelFilename, Vertices, Indices, Subsets, mats);

    ModelMesh.SetVertices(device, Vertices.data(), Vertices.size());
    ModelMesh.SetIndices(device, Indices.data(), Indices.size());
    ModelMesh.SetSubsetTable(Subsets);

    subsetCount = mats.size();

    for (UINT i = 0; i < subsetCount; ++i)
    {
        Mat.push_back(mats[i].Mat);
        ID3D11ShaderResourceView* diffuseMapSRV = texMgr.CreateTexture(
            texturePath + mats[i].DiffuseMapName);
        DiffuseMapSRV.push_back(diffuseMapSRV);

        ID3D11ShaderResourceView* normalMapSRV = texMgr.CreateTexture(
            texturePath + mats[i].NormalMapName);
        NormalMapSRV.push_back(normalMapSRV);
    }
}

BasicModel::~BasicModel() {}