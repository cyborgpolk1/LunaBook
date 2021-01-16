#ifndef LITSKULL_H
#define LITSKULL_H

#include "CommonLibs.h"
#include "D3DApp.h"
#include "LightHelper.h"

struct Vertex
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
	XMMATRIX fTexTransform;
	Material Mat;
};

class LitSkullDemo : public D3DApp
{
public:
	LitSkullDemo(HINSTANCE hInstance);
	~LitSkullDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffer();
	void BuildFX();

private:
	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;
	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS[4];

	ID3D11Buffer* mPerFrameBuffer;
	ID3D11Buffer* mPerObjectBuffer;

	ID3D11InputLayout* mInputLayout;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;
	int mSkullVertexOffset;

	int mBoxIndexCount;
	int mGridIndexCount;
	int mSphereIndexCount;
	int mCylinderIndexCount;
	int mSkullIndexCount;

	int mBoxIndexOffset;
	int mGridIndexOffset;
	int mSphereIndexOffset;
	int mCylinderIndexOffset;
	int mSkullIndexOffset;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT3 mEyePosW;

	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	int mLightCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif
