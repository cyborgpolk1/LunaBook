#pragma once

#include "MeshGeometry.h"
#include "LightHelper.h"
#include "AnimationHelper.h"
#include <fstream>
#include <map>

struct M3dMaterial
{
    Material Mat;
    bool AlphaClip;
    std::wstring EffectTypeName;
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

struct SkinnedVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 Tex;
    XMFLOAT4 TangentU;
    XMFLOAT3 Weights;
    XMUINT4 BoneIndices;
};

class M3DLoader
{
public:
    bool LoadM3d(const std::wstring& filename,
        std::vector<M3dVertex>& vertices,
        std::vector<USHORT>& indices,
        std::vector<MeshGeometry::Subset>& subsets,
        std::vector<M3dMaterial>& mats);

    bool LoadM3d(const std::wstring& filename,
        std::vector<SkinnedVertex>& vertices,
        std::vector<USHORT>& indices,
        std::vector<MeshGeometry::Subset>& subsets,
        std::vector<M3dMaterial>& mats,
        SkinnedData& skinInfo);

private:
    void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
    void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets);
    void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<M3dVertex>& vertices);
    void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);

    void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices);
    void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets);
    void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
    void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT animationClips, std::map<std::string, AnimationClip>& animations);
    void ReadBoneKeyframes(std::ifstream& fin, BoneAnimation& boneAnimation);
};