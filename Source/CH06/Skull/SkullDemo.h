#ifndef SKULLDEMO_H
#define SKULLDEMO_H

#include "CommonLibs.h"
#include "D3DApp.h"

struct Vertex 
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct MatrixBuffer
{
	XMMATRIX WorldViewProj;
};

class SkullDemo : public D3DApp
{
public:
	SkullDemo(HINSTANCE hInstance);
	~SkullDemo();

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
	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;
	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mMatrixBuffer;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mWireframeRS;

	int mSkullIndexCount;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif
