#ifndef HILLSDEMO_H
#define HILLSDEMO_H

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

class HillsDemo : public D3DApp
{
public:
	HillsDemo(HINSTANCE hInstance);
	~HillsDemo();

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
	
	float GetHeight(float x, float z) const;

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;
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

	int mGridIndexCount;
};

#endif