#pragma once

#include "MeshGeometry.h"
#include "LightHelper.h"
#include <fstream>

struct M3dMaterial
{
    Material Mat;
    bool AlphaClip;
    std::string EffectTypeName;
    std::wstring DiffuseMapName;
    std::wstring NormalMapName;
};

struct M3dVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 Tex;
    XMFLOAT4 TangentU;
};

class M3DLoader
{
public:
    bool LoadM3d(const std::string& filename,
        std::vector<M3dVertex>& vertices,
        std::vector<USHORT>& indices,
        std::vector<MeshGeometry::Subset>& subsets,
        std::vector<M3dMaterial>& mats);

private:
    void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
    void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets);
    void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<M3dVertex>& vertices);
    void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
};