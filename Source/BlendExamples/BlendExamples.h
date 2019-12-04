#ifndef BLENDEXAMPLES_H
#define BLENDEXAMPLES_H

#include "D3DApp.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct MatrixBuffer
{
	XMMATRIX WorldViewProj;
};

class BlendExamplesApp : public D3DApp
{
public:
	BlendExamplesApp(HINSTANCE hInstance);
	~BlendExamplesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildTex();
	void BuildBlendStates();

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	ID3D11Buffer* mMatrixBuffer;
	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;

	UINT mIndexCount;

	ID3D11InputLayout* mInputLayout;

	ID3D11ShaderResourceView* mTexture[2];
	ID3D11SamplerState* mSampleState;

	ID3D11BlendState* mStates[4];
	ID3D11DepthStencilState* mDS;

	XMMATRIX mTransforms[5];
};



#endif
