#pragma once

#include "d3dUtil.h"
#include "LightHelper.h"
#include <vector>

class Camera;
struct DirectionalLight;

class Terrain
{
public:
    struct InitInfo
    {
        // Filename of RAW heightmap data.
        std::wstring HeightMapFilename;

        // Texture filenames used for texturing the terrain.
        std::wstring LayerMapFilename0;
        std::wstring LayerMapFilename1;
        std::wstring LayerMapFilename2;
        std::wstring LayerMapFilename3;
        std::wstring LayerMapFilename4;
        std::wstring BlendMapFilename;

        // Scale to apply to heights after they have been
        // loaded from the heightmap.
        float HeightScale;

        // Dimensions of the heightmap.
        UINT HeightmapWidth;
        UINT HeightmapHeight;

        // The cell spacing along the x- and z-axes.
        float CellSpacing;
    };

public:
    Terrain();
    ~Terrain();

    float GetWidth() const;
    float GetDepth() const;
    float GetHeight(float x, float z) const;

    XMMATRIX GetWorld() const;
    void SetWorld(CXMMATRIX M);
    
    void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);

    void Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3]);

private:
    void LoadHeightmap();
    bool InBounds(int i, int j);
    float Average(int i, int j);
    void Smooth();
    void BuildHeightmapSRV(ID3D11Device* device);
    void BuildQuadPatchVB(ID3D11Device* device);
    void BuildQuadPatchIB(ID3D11Device* device);
    void CalcAllPatchBoundsY();
    void CalcPatchBoundsY(UINT i, UINT j);

private:
    struct TerrainVertex
    {
        XMFLOAT3 Pos;
        XMFLOAT2 Tex;
        XMFLOAT2 BoundsY;
    };

    struct alignas(16) PerFrameBuffer
    {
        DirectionalLight DirLights[3];
        XMFLOAT3 EyePosW;

        float FogStart;
        XMFLOAT4 FogColor;
        float FogRange;

        float MinDist;
        float MaxDist;

        float MinTess;
        float MaxTess;

        float TexelCellSpaceU;
        float TexelCellSpaceV;

        float WorldCellSpace;

        XMFLOAT2 TexScale;

        XMFLOAT2 pad;

        XMFLOAT4 WorldFrustumPlanes[6];
    };

    struct alignas(16) PerObjectBuffer
    {
        XMMATRIX ViewProj;
        Material Mat;
    };

    static const int CellsPerPatch = 64;

    ID3D11Buffer* mQuadPatchVB;
    ID3D11Buffer* mQuadPatchIB;

    ID3D11ShaderResourceView* mLayerMapArraySRV;
    ID3D11ShaderResourceView* mBlendMapSRV;
    ID3D11ShaderResourceView* mHeightMapSRV;

    ID3D11SamplerState* mSamplerLinear;
    ID3D11SamplerState* mSamplerPoint;

    ID3D11InputLayout* mInputLayout;

    ID3D11VertexShader* mVS;
    ID3D11HullShader* mHS;
    ID3D11DomainShader* mDS;
    ID3D11PixelShader* mPS;

    ID3D11Buffer* mPerFrameBuffer;
    ID3D11Buffer* mPerObjectBuffer;

    InitInfo mInfo;

    UINT mNumPatchVertices;
    UINT mNumPatchQuadFaces;

    UINT mNumPatchVertRows;
    UINT mNumPatchVertCols;

    XMFLOAT4X4 mWorld;

    Material mMat;

    std::vector<XMFLOAT2> mPatchBoundsY;
    std::vector<float> mHeightmap;
};