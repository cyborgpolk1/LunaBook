#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "LightHelper.h"
#include <stack>

enum Constants {
	NONE,
	ROOM,
	SKULL
};

struct TexVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;

	TexVertex() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}

	TexVertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty)
		: Pos(px, py, pz), Normal(nx, ny, nz), Tex(tx, ty) {}
};

struct BasicVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct PerFrameBuffer
{
	DirectionalLight DirLights[3];
	XMFLOAT3 EyePosW;

	float FogStart;
	float FogRange;
	XMFLOAT4 FogColor;

	XMFLOAT3 pad;
};

struct PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX WorldViewProj;
	XMMATRIX gTexTransform;
	Material Mat;
};

class MirrorDemo : public D3DApp
{
public:
	MirrorDemo(HINSTANCE hInstance);
	~MirrorDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildRoomGeometryBuffer();
	void BuildSkullGeometryBuffer();
	void BuildFX();
	void BuildTex();
	void BuildRasterizerStates();
	void BuildDepthStencilStates();
	void BuildBlendStates();

	void SetRoomConstants();
	void SetSkullConstants();

	void DrawWall(bool drawBackWall);
	void DrawFloor();
	void DrawMirror();
	void DrawSkull(Material skullMat);

private:
	ID3D11Buffer* mRoomVB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;
	
	UINT mSkullIndexCount;

	Constants currentConstants;

	ID3D11VertexShader *mTexVS, *mLitVS;
	ID3D11PixelShader *mTexPS, *mLitPS;

	ID3D11Buffer* mPerFrameBuffer;
	ID3D11Buffer* mReflectedPerFrameBuffer;
	ID3D11Buffer* mCurrentPerFrameBuffer;
	ID3D11Buffer* mPerObjectBuffer;

	ID3D11InputLayout* mRoomInputLayout;
	ID3D11InputLayout* mSkullInputLayout;

	XMFLOAT3 mSkullTranslation;

	XMFLOAT4X4 mRoomWorld;
	XMFLOAT4X4 mSkullWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT3 mEyePosW;

	DirectionalLight mDirLights[3];
	Material mRoomMat;
	Material mMirrorMat;
	Material mShadowMat;
	Material mSkullMat;

	ID3D11ShaderResourceView* mFloorTexture;
	ID3D11ShaderResourceView* mWallTexture;
	ID3D11ShaderResourceView* mMirrorTexture;
	ID3D11SamplerState* mSampleState;

	ID3D11RasterizerState* mCullClockwiseRS;
	ID3D11RasterizerState* mNoCullRS;

	ID3D11DepthStencilState* mMarkMirrorDSS;
	ID3D11DepthStencilState* mMarkFloorDSS;
	ID3D11DepthStencilState* mDrawReflectionDSS;
	ID3D11DepthStencilState* mNoDoubleBlendDSS;

	ID3D11BlendState* mNoRenderTargetWriteBS;
	ID3D11BlendState* mTransparentBS;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	std::stack<XMMATRIX> matrixStack;
};