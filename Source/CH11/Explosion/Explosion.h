#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"

struct Vertex
{
	XMFLOAT3 Pos;
};

struct alignas(16) ConstantBuffer
{
	XMMATRIX WorldViewProj;
	float Time;
};

class Explosion : public D3DApp
{
public:
	Explosion(HINSTANCE hInstance);
	~Explosion();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildIcosahedron();
	void BuildFX();

private:
	ID3D11Buffer* mIcosVB;
	ID3D11Buffer* mIcosIB;

	UINT mIcosIndexCount;

	ID3D11VertexShader* mVS;
	ID3D11GeometryShader* mGS;
	ID3D11PixelShader* mPS;

	ID3D11InputLayout* mInputLayout;

	ID3D11Buffer* mConstantBuffer;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	ID3D11RasterizerState* mNoCullRS;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};