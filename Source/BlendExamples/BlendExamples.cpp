#include "BlendExamples.h"
#include <d3dcompiler.h>
#include <vector>

D3DMAIN(BlendExamplesApp);

BlendExamplesApp::BlendExamplesApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mVB(0), mIB(0), mInputLayout(0), mVS(0), mPS(0),
	mSampleState(0), mMatrixBuffer(0), mIndexCount(0), mDS(0)
{
	mMainWndCaption = L"Blend Examples Demo";

	for (int i = 0; i < 5; i++)
		mTransforms[i] = XMMatrixIdentity();

	for (int i = 0; i < 2; i++)
		mTexture[i] = 0;

	for (int i = 0; i < 4; i++)
		mStates[i] = 0;
}

BlendExamplesApp::~BlendExamplesApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mMatrixBuffer);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mVS);
	ReleaseCOM(mPS);
	for (int i = 0; i < 2; i++)
		ReleaseCOM(mTexture[i]);
	ReleaseCOM(mSampleState);
	for (int i = 0; i < 4; i++)
		ReleaseCOM(mStates[i]);
	ReleaseCOM(mDS);
}

bool BlendExamplesApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildTex();
	BuildBlendStates();

	return true;
}

void BlendExamplesApp::OnResize() 
{
	D3DApp::OnResize();
}

void BlendExamplesApp::UpdateScene(float dt) {}

void BlendExamplesApp::DrawScene() 
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->VSSetShader(mVS, 0, 0);
	md3dImmediateContext->PSSetShader(mPS, 0, 0);

	for (int i = 0; i < 8; i++)
	{
		int j = i;

		if (i >= 2)
			j = i / 2 + 1;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HR(md3dImmediateContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

		MatrixBuffer* dataPtr = (MatrixBuffer*)mappedResource.pData;
		dataPtr->WorldViewProj = mTransforms[j];

		md3dImmediateContext->Unmap(mMatrixBuffer, 0);
		md3dImmediateContext->VSSetConstantBuffers(0, 1, &mMatrixBuffer);

		md3dImmediateContext->PSSetShaderResources(0, 1, &mTexture[i % 2]);
		md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);

		md3dImmediateContext->OMSetDepthStencilState(mDS, 0);

		float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		if (i % 2 == 1)
			md3dImmediateContext->OMSetBlendState(mStates[i/2], blendFactors, 0xffffffff);
		else
			md3dImmediateContext->OMSetBlendState(mStates[0], blendFactors, 0xffffffff);

		md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void BlendExamplesApp::BuildGeometryBuffers() 
{
	std::vector<Vertex> vertices
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices.data();
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	std::vector<UINT> indices
	{
		0, 1, 2,
		1, 3, 2
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices.data();
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));

	mIndexCount = indices.size();
}

void BlendExamplesApp::BuildFX() 
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	CreateShader(&mVS, ExePath().append(L"../../../Shaders/BasicTexture.hlsl").c_str(), "VS", 0, &mInputLayout, vertexDesc.data(), vertexDesc.size());
	CreateShader(&mPS, ExePath().append(L"../../../Shaders/BasicTexture.hlsl").c_str(), "PS", 0);

	mTransforms[0] = XMMatrixTranspose(XMMatrixScaling(0.3f, 0.3f, 1.0f) * XMMatrixTranslation(-0.495f, 0.66f, 0.0f));
	mTransforms[1] = XMMatrixTranspose(XMMatrixScaling(0.3f, 0.3f, 1.0f) * XMMatrixTranslation(0.495f, 0.66f, 0.0f));
	mTransforms[2] = XMMatrixTranspose(XMMatrixScaling(0.3f, 0.3f, 1.0f) * XMMatrixTranslation(-0.66f, -0.33f, 0.0f));
	mTransforms[3] = XMMatrixTranspose(XMMatrixScaling(0.3f, 0.3f, 1.0f) * XMMatrixTranslation(0.0f, -0.33f, 0.0f));
	mTransforms[4] = XMMatrixTranspose(XMMatrixScaling(0.3f, 0.3f, 1.0f) * XMMatrixTranslation(0.66f, -0.33f, 0.0f));


	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mMatrixBuffer));
}

void BlendExamplesApp::BuildTex() 
{
	ID3D11Resource* textureResource[2];
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/darkbrickdxt1.dds").c_str(), &textureResource[0], &mTexture[0]));
	HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/flarealpha.dds").c_str(), &textureResource[1], &mTexture[1]));

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState));

	for (int i = 0; i < 2; i++)
		ReleaseCOM(textureResource[i]);
}

void BlendExamplesApp::BuildBlendStates()
{
	// None
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(md3dDevice->CreateBlendState(&blendDesc, &mStates[0]));

	// Add
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	HR(md3dDevice->CreateBlendState(&blendDesc, &mStates[1]));

	// Subtract
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;

	HR(md3dDevice->CreateBlendState(&blendDesc, &mStates[2]));

	// Multiply
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	HR(md3dDevice->CreateBlendState(&blendDesc, &mStates[3]));

	D3D11_DEPTH_STENCIL_DESC depthDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	HR(md3dDevice->CreateDepthStencilState(&depthDesc, &mDS));
}


