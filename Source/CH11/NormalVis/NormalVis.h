#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct ConstantBuffer
{
    XMMATRIX World;
	XMMATRIX ViewProj;
	XMMATRIX WorldInvTranspose;
};

class NormalVis : public D3DApp
{
public:
	NormalVis(HINSTANCE hInstance);
	~NormalVis();

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

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	UINT mIndexCount;

	ID3D11VertexShader* mNormalVS;
	ID3D11GeometryShader* mFaceNormalGS;
	ID3D11GeometryShader* mVertexNormalGS;
	ID3D11PixelShader* mNormalPS;

	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;

	ID3D11InputLayout* mInputLayout;

	ID3D11Buffer* mConstantBuffer;

    ID3D11RasterizerState* mWireframeRS;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};