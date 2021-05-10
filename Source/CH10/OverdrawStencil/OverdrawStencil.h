#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "Waves.h"
#include "LightHelper.h"
#include "DDSTextureLoader.h"
#include <array>

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

struct alignas(16) PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX WorldViewProj;
	XMMATRIX TextureTransform;
	Material Mat;
    int Options;
};

struct QuadVertex
{
	XMFLOAT3 Pos;
};

struct PerQuadBuffer
{
	XMFLOAT4 Color;
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

std::array<XMFLOAT4, 7> colors = 
{
	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
	XMFLOAT4(0.0f, 0.2f, 0.0f, 1.0f),
	XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f),
	XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
	XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
	XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)
};

class OverdrawStencilApp : public D3DApp
{
public:
	OverdrawStencilApp(HINSTANCE hInstance);
	~OverdrawStencilApp();

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
	void BuildQuadGeometryBuffers();
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildCrateGeometryBuffers();
	void BuildFX();
	void BuildTex();
	void BuildRasterizerStates();
	void BuildDepthStencilStates();
	void BuildBlendStates();

	void drawToStencil();

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;
	ID3D11Buffer* mCrateVB;
	ID3D11Buffer* mCrateIB;
	ID3D11Buffer* mQuadVB;
	ID3D11Buffer* mQuadIB;

	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mPerObjectBuffer;
	ID3D11Buffer* mPerFrameBuffer;

	ID3D11VertexShader* mQuadVS;
	ID3D11PixelShader* mQuadPS;
	ID3D11Buffer* mPerQuadBuffer;

	ID3D11InputLayout* mSceneLayout;
	ID3D11InputLayout* mQuadLayout;

	ID3D11RasterizerState* mNoCullRS;

	ID3D11DepthStencilState* mRenderToStencilDSS;
	ID3D11DepthStencilState* mRenderOverdrawDSS;

	ID3D11ShaderResourceView* mLandTexture;
	ID3D11ShaderResourceView* mWaterTexture;
	ID3D11ShaderResourceView* mCrateTexture;
	ID3D11SamplerState* mSampleState;

	ID3D11BlendState* mTransparentBlend;
	ID3D11BlendState* mNoRenderTargetWriteBS;

	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mCrateWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT2 mWaterTexOffset;

	UINT mGridIndexCount, mCrateIndexCount, mQuadIndexCount;

	Waves mWaves;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	DirectionalLight mDirLights[3];

	Material mLandMat;
	Material mWavesMat;
	Material mCrateMat;

	XMFLOAT3 mEyePosW;
};
