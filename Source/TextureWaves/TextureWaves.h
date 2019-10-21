#pragma once

#include "CommonLibs.h"
#include "D3DApp.h"
#include "Waves.h"
#include "LightHelper.h"
#include "DDSTextureLoader.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct PerObjectBuffer
{
	XMMATRIX World;
	XMMATRIX WorldInvTranspose;
	XMMATRIX WorldViewProj;
	Material Mat;
};

struct PerFrameBuffer
{
	DirectionalLight DirLight;
	PointLight PLight;
	SpotLight SLight;
	XMFLOAT3 EyePosW;
	float pad;
};

class TextureWavesApp : public D3DApp
{
public:
	TextureWavesApp(HINSTANCE hInstance);
	~TextureWavesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildFX();
	void BuildTex();

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;
	ID3D11Buffer* mPerObjectBuffer;
	ID3D11Buffer* mPerFrameBuffer;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mWireframeRS;

	ID3D11ShaderResourceView* mTexture;
	ID3D11SamplerState* mSampleState;

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

	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	Material mLandMat;
	Material mWavesMat;

	XMFLOAT3 mEyePosW;
};
