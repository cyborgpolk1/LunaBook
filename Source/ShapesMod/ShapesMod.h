#ifndef SHAPESMODDEMO_H
#define SHAPESMODDEMO_H

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

class ShapesDemo : public D3DApp
{
public:
	ShapesDemo(HINSTANCE hInstance);
	~ShapesDemo();

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
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mMatrixBuffer;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mWireframeRS;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	int mBoxIndexCount;
	int mGridIndexCount;
	int mSphereIndexCount;
	int mCylinderIndexCount;

	int mBoxIndexOffset;
	int mGridIndexOffset;
	int mSphereIndexOffset;
	int mCylinderIndexOffset;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif