#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX ViewProj;
};

class CircleToCylinder : public D3DApp
{
public:
	CircleToCylinder(HINSTANCE hInstance);
	~CircleToCylinder();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildCircleBuffer();
	void BuildFX();

private:
	ID3D11Buffer* mCircleVB;
	ID3D11Buffer* mCircleIB;

	static const UINT mCircleVertexCount = 8;
	UINT mCircleIndexCount;

	ID3D11VertexShader* mVS;
	ID3D11GeometryShader* mGS;
	ID3D11PixelShader* mPS;

	ID3D11InputLayout* mInputLayout;

	ID3D11Buffer* mConstantBuffer;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	ID3D11RasterizerState* mRS;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};