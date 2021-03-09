#include "TessQuad.h"
#include "MathHelper.h"

D3DMAIN(TessQuad);

TessQuad::TessQuad(HINSTANCE hInstance)
    : D3DApp(hInstance), mQuadPatchVB(0), mVS(0), mHS(0), mDS(0), mPS(0),
    mInputLayout(0), mMatrixBuffer(0), mWireframeRS(0), mEyePosW(0.0f, 0.0f, 0.0f),
    mTheta(1.3f*MathHelper::Pi), mPhi(0.2f*MathHelper::Pi), mRadius(80.0f)
{
    mMainWndCaption = L"Quad Tessellation";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

TessQuad::~TessQuad()
{
    ReleaseCOM(mQuadPatchVB);
    ReleaseCOM(mVS);
    ReleaseCOM(mHS);
    ReleaseCOM(mDS);
    ReleaseCOM(mPS);
    ReleaseCOM(mWireframeRS);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mMatrixBuffer);
    ReleaseCOM(mWireframeRS);
}

bool TessQuad::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildQuadPatchBuffer();
    BuildFX();

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    rsDesc.CullMode = D3D11_CULL_NONE;

    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mWireframeRS));

    return true;
}

void TessQuad::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void TessQuad::UpdateScene(float dt)
{
    // Convert Spherical to Cartesian coordinates.
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    mEyePosW = XMFLOAT3(x, y, z);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void TessQuad::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    md3dImmediateContext->IASetInputLayout(mInputLayout);

    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;

    md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);

    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    MatrixBuffer* dataPtr = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldViewProj = XMMatrixTranspose(world * view * proj);
    dataPtr->EyePosW = mEyePosW;

    md3dImmediateContext->Unmap(mMatrixBuffer, 0);

    md3dImmediateContext->HSSetConstantBuffers(0, 1, &mMatrixBuffer);
    md3dImmediateContext->DSSetConstantBuffers(0, 1, &mMatrixBuffer);

    md3dImmediateContext->VSSetShader(mVS, 0, 0);
    md3dImmediateContext->HSSetShader(mHS, 0, 0);
    md3dImmediateContext->DSSetShader(mDS, 0, 0);
    md3dImmediateContext->PSSetShader(mPS, 0, 0);

    md3dImmediateContext->RSSetState(mWireframeRS);

    md3dImmediateContext->Draw(4, 0);

    md3dImmediateContext->RSSetState(0);

    md3dImmediateContext->VSSetShader(0, 0, 0);
    md3dImmediateContext->HSSetShader(0, 0, 0);
    md3dImmediateContext->DSSetShader(0, 0, 0);
    md3dImmediateContext->PSSetShader(0, 0, 0);

    HR(mSwapChain->Present(0, 0));
}

void TessQuad::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void TessQuad::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void TessQuad::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 5.0f, 300.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void TessQuad::BuildQuadPatchBuffer()
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(XMFLOAT3) * 4;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    XMFLOAT3 vertices[4] = 
    {
        XMFLOAT3(-10.0f, 0.0f, 10.0f),
        XMFLOAT3(10.0f, 0.0f, 10.0f),
        XMFLOAT3(-10.0f, 0.0f, -10.0f),
        XMFLOAT3(10.0f, 0.0f, -10.0f)
    };

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
}

void TessQuad::BuildFX()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto filename = ExePath().append(L"../../../Shaders/TessQuad.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mVS, cstr, "VS", 0, &mInputLayout, vertexDesc, 1);
    ShaderHelper::CreateShader(md3dDevice, &mHS, cstr, "HS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mDS, cstr, "DS", 0);
    ShaderHelper::CreateShader(md3dDevice, &mPS, cstr, "PS", 0);

    // Create matrix buffer
    D3D11_BUFFER_DESC matrixDesc;
    matrixDesc.ByteWidth = sizeof(MatrixBuffer);
    matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixDesc.MiscFlags = 0;
    matrixDesc.StructureByteStride = 0;
    matrixDesc.Usage = D3D11_USAGE_DYNAMIC;

    HR(md3dDevice->CreateBuffer(&matrixDesc, 0, &mMatrixBuffer));
}