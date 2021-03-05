#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "ComputeWaves.h"
#include "LightHelper.h"
#include "DDSTextureLoader.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

struct TreeVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Size;
};

struct PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX WorldViewProj;
	XMMATRIX TextureTransform;
	Material Mat;
};

struct PerTreeBuffer
{
	XMMATRIX ViewProj;
	Material Mat;
};

#pragma pack 4
struct PerFrameBuffer
{
	DirectionalLight DirLights[3];
	XMFLOAT3 EyePosW;

	float FogStart;
	XMFLOAT4 FogColor;
	float FogRange;

	XMFLOAT3 pad;
};

struct PerWave
{
    XMFLOAT2 DisplacementTexelSize;
    float GridSpatialStep;
    float pad;
};

class WavesDemo : public D3DApp
{
public:
	WavesDemo(HINSTANCE hInstance);
	~WavesDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildCrateGeometryBuffers();
	void BuildTreeGeometryBuffers();
	void BuildFX();
	void BuildTex();

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;
	ID3D11Buffer* mCrateVB;
	ID3D11Buffer* mCrateIB;

	ID3D11Buffer* mTreeVB;

	static const UINT TreeCount  = 16;

	ID3D11VertexShader* mVS;
    ID3D11VertexShader* mWavesVS;
	ID3D11PixelShader* mPS;

	ID3D11VertexShader* mBillVS;
	ID3D11GeometryShader* mBillGS;
	ID3D11PixelShader* mBillPS;

    ID3D11ComputeShader* mWavesCS;

	ID3D11Buffer* mPerObjectBuffer;
	ID3D11Buffer* mPerFrameBuffer;
	ID3D11Buffer* mTreeBuffer;
    ID3D11Buffer* mWaveBuffer;

	ID3D11InputLayout* mInputLayout;
	ID3D11InputLayout* mTreeLayout;

	ID3D11RasterizerState* mNoCullRS;

	ID3D11ShaderResourceView* mLandTexture;
	ID3D11ShaderResourceView* mWaterTexture;
	ID3D11ShaderResourceView* mCrateTexture;
	ID3D11ShaderResourceView* mTreeTextureArray;

	ID3D11SamplerState* mSampleState;
    ID3D11SamplerState* mClampSampler;

	ID3D11BlendState* mTransparentBlend;
	ID3D11BlendState* mAlphaToCoverageBS;

	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mCrateWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT2 mWaterTexOffset;

    UINT mGridIndexCount, mCrateIndexCount, mWavesIndexCount;

	ComputeWaves mWaves;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	DirectionalLight mDirLights[3];

	Material mLandMat;
	Material mWavesMat;
	Material mCrateMat;
	Material mTreeMat;

	XMFLOAT3 mEyePosW;
};
