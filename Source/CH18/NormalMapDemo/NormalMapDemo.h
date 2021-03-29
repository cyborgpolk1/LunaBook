#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Sky.h"

struct TexVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
    XMFLOAT3 Tangent;
};

struct BasicVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct alignas(16) PerFrameBuffer
{
	DirectionalLight DirLights[3];
	XMFLOAT3 EyePosW;

	float FogStart;
    XMFLOAT4 FogColor;
	float FogRange;
	
    float HeightScale;
    float MaxTessDistance;
    float MinTessDistance;
    float MinTessFactor;
    float MaxTessFactor;
};

struct PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX WorldViewProj;
	XMMATRIX gTexTransform;
	Material Mat;
    int Options;
};

enum RenderOptions
{
    RenderOptionsBasic = 0,
    RenderOptionsNormalMap = 1,
    RenderOptionsDisplacementMap = 2
};

class NormalMapDemo : public D3DApp
{
public:
	NormalMapDemo(HINSTANCE hInstance);
	~NormalMapDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildShapeGeometryBuffer();
	void BuildSkullGeometryBuffer();
	void BuildFX();
	void BuildTex();

private:
	ID3D11Buffer *mShapesVB, *mSkullVB;
	ID3D11Buffer *mShapesIB, *mSkullIB;
    ID3D11VertexShader *mTexVS, *mNormalVS, *mDispVS;
    ID3D11PixelShader *mTexPS, *mNormalPS, *mDispPS;
    ID3D11HullShader* mDispHS;
    ID3D11DomainShader* mDispDS;

	ID3D11Buffer* mPerFrameBuffer;
	ID3D11Buffer* mPerObjectBuffer;

	ID3D11InputLayout *mShapeInputLayout;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	int mBoxIndexCount;
	int mGridIndexCount;
	int mSphereIndexCount;
	int mCylinderIndexCount;
	int mSkullIndexCount;

	int mBoxIndexOffset;
	int mGridIndexOffset;
	int mSphereIndexOffset;
	int mCylinderIndexOffset;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;

	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	ID3D11ShaderResourceView *mFloorTex, *mBrickTex, *mStoneTex;
    ID3D11ShaderResourceView *mFloorNormal, *mBrickNormal, *mStoneNormal;
	ID3D11SamplerState* mSampleState;

    bool mDoWireframe;
    ID3D11RasterizerState* mWireframeRS;

    Camera mCamera;

    Sky* mSky;

	POINT mLastMousePos;

    RenderOptions mRenderOptions;
};
