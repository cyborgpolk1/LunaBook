#include "TessFactorExamples.h"
#include <vector>

D3DMAIN(TessFactorExamples);

TessFactorExamples::TessFactorExamples(HINSTANCE hInstance)
    : D3DApp(hInstance), mTriPatchVB(0), mQuadPatchVB(0), mInputLayout(0),
    mVS(0), mTriHS(0), mQuadHS(0), mTriDS(0), mQuadDS(0), mPS(0), mWireframeRS(0),
    mTriVertexCount(0), mQuadVertexCount(0)
{
    mMainWndCaption = L"Tess Factor Examples";
}

TessFactorExamples::~TessFactorExamples()
{
    ReleaseCOM(mTriPatchVB);
    ReleaseCOM(mQuadPatchVB);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mVS);
    ReleaseCOM(mTriHS);
    ReleaseCOM(mQuadHS);
    ReleaseCOM(mTriDS);
    ReleaseCOM(mQuadDS);
    ReleaseCOM(mPS);
    ReleaseCOM(mWireframeRS);
}

bool TessFactorExamples::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildPatchBuffers();
    BuildFX();

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mWireframeRS));

    return true;
}

void TessFactorExamples::OnResize()
{
    D3DApp::OnResize();
}

void TessFactorExamples::UpdateScene(float dt) {}

void TessFactorExamples::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    md3dImmediateContext->IASetInputLayout(mInputLayout);

    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mTriPatchVB, &stride, &offset);

    md3dImmediateContext->VSSetShader(mVS, 0, 0);
    md3dImmediateContext->HSSetShader(mTriHS, 0, 0);
    md3dImmediateContext->DSSetShader(mTriDS, 0, 0);
    md3dImmediateContext->PSSetShader(mPS, 0, 0);

    md3dImmediateContext->RSSetState(mWireframeRS);

    md3dImmediateContext->Draw(mTriVertexCount, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);

    md3dImmediateContext->HSSetShader(mQuadHS, 0, 0);
    md3dImmediateContext->DSSetShader(mQuadDS, 0, 0);

    md3dImmediateContext->Draw(mQuadVertexCount, 0);

    md3dImmediateContext->RSSetState(0);

    md3dImmediateContext->VSSetShader(0, 0, 0);
    md3dImmediateContext->HSSetShader(0, 0, 0);
    md3dImmediateContext->DSSetShader(0, 0, 0);
    md3dImmediateContext->PSSetShader(0, 0, 0);

    HR(mSwapChain->Present(0, 0));
}

void TessFactorExamples::OnMouseDown(WPARAM btnState, int x, int y) {}

void TessFactorExamples::OnMouseUp(WPARAM btnState, int x, int y) {}

void TessFactorExamples::OnMouseMove(WPARAM btnState, int x, int y) {}

void TessFactorExamples::BuildPatchBuffers()
{
    std::vector<XMFLOAT3> tris =
    {
        XMFLOAT3(-0.95f, 0.1f, 0.0f),
        XMFLOAT3(-0.75f, 0.9f, 0.0f),
        XMFLOAT3(-0.55f, 0.1f, 0.0f),

        XMFLOAT3(-0.45f, 0.1f, 0.0f),
        XMFLOAT3(-0.25f, 0.9f, 0.0f),
        XMFLOAT3(-0.05f, 0.1f, 0.0f),

        XMFLOAT3(0.05f, 0.1f, 0.0f),
        XMFLOAT3(0.25f, 0.9f, 0.0f),
        XMFLOAT3(0.45f, 0.1f, 0.0f),

        XMFLOAT3(0.55f, 0.1f, 0.0f),
        XMFLOAT3(0.75f, 0.9f, 0.0f),
        XMFLOAT3(0.95f, 0.1f, 0.0f)
    };

    mTriVertexCount = tris.size();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(XMFLOAT3) * mTriVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.StructureByteStride = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = tris.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mTriPatchVB));

    std::vector<XMFLOAT3> quads =
    {
        XMFLOAT3(-0.95f, -0.1f, 0.0f),
        XMFLOAT3(-0.55f, -0.1f, 0.0f),
        XMFLOAT3(-0.95f, -0.9f, 0.0f),
        XMFLOAT3(-0.55f, -0.9f, 0.0f),

        XMFLOAT3(-0.45f, -0.1f, 0.0f),
        XMFLOAT3(-0.05f, -0.1f, 0.0f),
        XMFLOAT3(-0.45f, -0.9f, 0.0f),
        XMFLOAT3(-0.05f, -0.9f, 0.0f),

        XMFLOAT3(0.05f, -0.1f, 0.0f),
        XMFLOAT3(0.45f, -0.1f, 0.0f),
        XMFLOAT3(0.05f, -0.9f, 0.0f),
        XMFLOAT3(0.45f, -0.9f, 0.0f),

        XMFLOAT3(0.55f, -0.1f, 0.0f),
        XMFLOAT3(0.95f, -0.1f, 0.0f),
        XMFLOAT3(0.55f, -0.9f, 0.0f),
        XMFLOAT3(0.95f, -0.9f, 0.0f),
    };

    mQuadVertexCount = quads.size();

    vbd.ByteWidth = sizeof(XMFLOAT3) * mQuadVertexCount;
    vinitData.pSysMem = quads.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
}

void TessFactorExamples::BuildFX()
{
    D3D11_INPUT_ELEMENT_DESC patchDesc[1] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto filename = ExePath().append(L"../../../Shaders/TessExamples.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mVS, cstr, "VS", 0, &mInputLayout, patchDesc, 1);
    ShaderHelper::CreateShader(md3dDevice, &mTriHS, cstr, "TriHS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mQuadHS, cstr, "QuadHS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mTriDS, cstr, "TriDS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mQuadDS, cstr, "QuadDS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mPS, cstr, "PS", 0);
}