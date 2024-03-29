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

struct alignas(16) PerObjectBuffer
{
	XMMATRIX World;
    XMMATRIX WorldInvTranspose;
    XMMATRIX WorldViewProj;
    XMMATRIX gTexTransform;
    Material Mat;
    int Options;
};

class DynamicCubeMap : public D3DApp
{
public:
	DynamicCubeMap(HINSTANCE hInstance);
	~DynamicCubeMap();

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
    void BuildCubeFaceCamera(float x, float y, float z);
    void BuildDynamicCubeMapViews();
    void DrawScene(const Camera& camera, bool drawCenterSphere);

private:
	ID3D11Buffer *mShapesVB, *mSkullVB;
	ID3D11Buffer *mShapesIB, *mSkullIB;
	ID3D11VertexShader *mTexVS;
	ID3D11PixelShader *mTexPS[4];

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
    XMFLOAT4X4 mCenterSphereWorld;

	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;
    Material mCenterSphereMat;

	ID3D11ShaderResourceView *mFloorTex, *mBrickTex, *mStoneTex;
	ID3D11SamplerState* mSampleState;

    ID3D11DepthStencilView* mDynamicCubeMapDSV;
    ID3D11RenderTargetView* mDynamicCubeMapRTV[6];
    ID3D11ShaderResourceView* mDynamicCubeMapSRV;
    D3D11_VIEWPORT mCubeMapViewport;

    static const int CubeMapSize = 256;

	int mLightCount;

    Camera mCamera;
    Camera mCubeMapCamera[6];

    Sky* mSky;

	POINT mLastMousePos;
};
