#pragma once

#include "D3DApp.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "WICTextureLoader.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct MatrixBuffer
{
	XMMATRIX WorldViewProj;
};

class FireCrateApp : public D3DApp
{
public:
	FireCrateApp(HINSTANCE hInstance);
	~FireCrateApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildTex();

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mMatrixBuffer;

	UINT mIndexCount;

	ID3D11InputLayout* mInputLayout;

	ID3D11ShaderResourceView* mTexture[120];
	ID3D11SamplerState* mSampleState;
	int currentFrame;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePoint;
};