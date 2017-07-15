#ifndef LITWAVESDEMO_H
#define LITWAVESDEMO_H

#include "CommonLibs.h"
#include "D3DApp.h"
#include "Waves.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct MatrixBuffer
{
	XMMATRIX WorldViewProj;
};

class LitWavesDemo : public D3DApp
{
public:
	LitWavesDemo(HINSTANCE hInstance);
	~LitWavesDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHeight(float x, float z) const;
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildFX();

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mMatrixBuffer;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mWireframeRS;

	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mGridIndexCount;

	Waves mWaves;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif
