#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "Waves.h"
#include "LightHelper.h"
#include "DDSTextureLoader.h"
#include "BlurFilter.h"

struct Vertex
{
	XMFLOAT3 Pos;
    XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};

struct TreeVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Size;
};

struct QuadVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

struct PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX WorldViewProj;
	XMMATRIX TextureTransform;
	Material Mat;
    int Options;
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

class BlurDemo : public D3DApp
{
public:
	BlurDemo(HINSTANCE hInstance);
	~BlurDemo();

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
    void BuildQuadGeometryBuffers();
	void BuildFX();
	void BuildTex();
    void DrawToTexture();
    void BuildOffScreenViews();

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;
	ID3D11Buffer* mCrateVB;
	ID3D11Buffer* mCrateIB;

	ID3D11Buffer* mTreeVB;

    ID3D11Buffer* mQuadVB;
    ID3D11Buffer* mQuadIB;

	static const UINT TreeCount  = 16;

	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;

	ID3D11VertexShader* mBillVS;
	ID3D11GeometryShader* mBillGS;
	ID3D11PixelShader* mBillPS;

    ID3D11VertexShader* mQuadVS;
    ID3D11PixelShader* mQuadPS;

	ID3D11Buffer* mPerObjectBuffer;
	ID3D11Buffer* mPerFrameBuffer;
	ID3D11Buffer* mTreeBuffer;
    ID3D11Buffer* mQuadBuffer;

	ID3D11InputLayout* mInputLayout;
	ID3D11InputLayout* mTreeLayout;
    ID3D11InputLayout* mQuadLayout;

	ID3D11RasterizerState* mNoCullRS;

	ID3D11ShaderResourceView* mLandTexture;
	ID3D11ShaderResourceView* mWaterTexture;
	ID3D11ShaderResourceView* mCrateTexture;
	ID3D11ShaderResourceView* mTreeTextureArray;
	ID3D11SamplerState* mSampleState;

    BlurFilter mBlur;

    ID3D11ShaderResourceView* mOffScreenSRV;
    ID3D11UnorderedAccessView* mOffScreenUAV;
    ID3D11RenderTargetView* mOffScreenRTV;

	ID3D11BlendState* mTransparentBlend;
	ID3D11BlendState* mAlphaToCoverageBS;

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
	Material mTreeMat;

	XMFLOAT3 mEyePosW;
};
