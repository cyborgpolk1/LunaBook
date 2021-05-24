#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Sky.h"
#include "ShadowMap.h"

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

struct QuadVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

struct alignas(16) PerFrameBuffer
{
	XMFLOAT3 EyePosW;
	
    float HeightScale;
    float MaxTessDistance;
    float MinTessDistance;
    float MinTessFactor;
    float MaxTessFactor;

    XMMATRIX ViewProj;

    DirectionalLight DirLights[3];
};

struct alignas(16) PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX gTexTransform;
    XMMATRIX ShadowMapTransform;
	Material Mat;
    int Options;
};

struct PerQuadBuffer
{
    XMMATRIX WorldViewProj;
};

enum RenderOptions
{
    RenderOptionsBasic = 0,
    RenderOptionsNormalMap = 1,
    RenderOptionsDisplacementMap = 2
};

struct BoundingSphere
{
    BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
    XMFLOAT3 Center;
    float Radius;
};

class ShadowMapDemo : public D3DApp
{
public:
	ShadowMapDemo(HINSTANCE hInstance);
	~ShadowMapDemo();

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
    void BuildScreenQuadGeometryBuffer();
    void BuildShadowTransform();
	void BuildFX();
	void BuildTex();
    void DrawSceneToShadowMap();
    void DrawScreenQuad();

private:
	ID3D11Buffer *mShapesVB, *mSkullVB, *mQuadVB;
	ID3D11Buffer *mShapesIB, *mSkullIB, *mQuadIB;
    ID3D11VertexShader *mTexVS, *mNormalVS, *mDispVS;
    ID3D11PixelShader *mTexPS, *mNormalPS, *mDispPS;
    ID3D11HullShader* mDispHS;
    ID3D11DomainShader* mDispDS;

	ID3D11Buffer* mPerFrameBuffer;
	ID3D11Buffer* mPerObjectBuffer;

    ID3D11InputLayout *mShapeInputLayout, *mQuadInputLayout;

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
	ID3D11SamplerState *mSampleState, *mComparisonState;

    bool mDoWireframe;
    ID3D11RasterizerState* mWireframeRS;

    Camera mCamera;

    Sky* mSky;

	POINT mLastMousePos;

    RenderOptions mRenderOptions;

    BoundingSphere mSceneBounds;

    static const int SMapSize = 2048;
    ShadowMap* mSmap;
    XMFLOAT4X4 mLightView;
    XMFLOAT4X4 mLightProj;
    XMFLOAT4X4 mShadowTransform;

    float mLightRotationAngle;
    XMFLOAT3 mOriginalLightDir[3];

    ID3D11VertexShader *mShadowVS, *mShadowTessVS;
    ID3D11PixelShader *mShadowPS;
    ID3D11HullShader* mShadowHS;
    ID3D11DomainShader* mShadowDS;

    ID3D11RasterizerState* mShadowRS;

    ID3D11VertexShader* mQuadVS;
    ID3D11PixelShader* mQuadPS;
    ID3D11Buffer* mPerQuadBuffer;
    ID3D11DepthStencilState* mQuadDS;
};
