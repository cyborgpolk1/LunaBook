#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include <array>

struct Vertex
{
	XMFLOAT3 Pos;
};

struct ConstantBuffer
{
	XMMATRIX View;
	XMMATRIX ViewProj;
};

class SubdivideIcosahedron : public D3DApp
{
public:
	SubdivideIcosahedron(HINSTANCE hInstance);
	~SubdivideIcosahedron();

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
	void BuildViewports();

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
	XMFLOAT4X4 mNoZoomView;
	XMFLOAT4X4 mProj;

	std::array<D3D11_VIEWPORT, 2> mViewports;

	ID3D11RasterizerState* mWireframeRS;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};