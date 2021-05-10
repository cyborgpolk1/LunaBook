#ifndef PYRAMIDANDBOX_H
#define PYRAMIDANDBOX_H

#include "D3DApp.h"
#include "MathHelper.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct alignas(16) MatrixBuffer
{
    XMMATRIX WorldViewProj;
	float Time;
};

class PyramidAndBoxApp : public D3DApp
{
public:
	PyramidAndBoxApp(HINSTANCE hInstance);
	~PyramidAndBoxApp();

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
	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mMatrixBuffer;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif