#include "InstancingAndCulling.h"
#include "MathHelper.h"
#include <fstream>
#include <string>

D3DMAIN(InstancingAndCulling);

InstancingAndCulling::InstancingAndCulling(HINSTANCE hInstance)
    : D3DApp(hInstance), mSkullVB(0), mSkullIB(0), mInstanceBuffer(0),
    mSkullIndexCount(0), mInputLayout(0), mVS(0), mPS(0), mVisibleObjectCount(0),
    mPerFrameBuffer(0), mPerObjectBuffer(0), mEnableFrustumCulling(true)
{
    mMainWndCaption = L"Instancing and Culling Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCamera.SetPosition(0.0f, 2.0f, -15.0f);

    XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

    mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

    mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
}

InstancingAndCulling::~InstancingAndCulling()
{
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mInstanceBuffer);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mVS);
    ReleaseCOM(mPS);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mPerObjectBuffer);
}

bool InstancingAndCulling::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildSkullGeometryBuffer();
    BuildInstanceBuffer();
    BuildFX();

    return true;
}

void InstancingAndCulling::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

    mCameraFrustum.CreateFromMatrix(mCameraFrustum, mCamera.Proj());
}

void InstancingAndCulling::UpdateScene(float dt)
{
    //
    // Control the camera.
    //
    if (GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(10.0f*dt);

    if (GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-10.0f*dt);

    if (GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-10.0f*dt);

    if (GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(10.0f*dt);

    if (GetAsyncKeyState('1') & 0x8000)
        mEnableFrustumCulling = true;

    if (GetAsyncKeyState('2') & 0x8000)
        mEnableFrustumCulling = false;

    mCamera.UpdateViewMatrix();

    mVisibleObjectCount = 0;

    if (mEnableFrustumCulling)
    {
        XMVECTOR detView = XMMatrixDeterminant(mCamera.View());
        XMMATRIX invView = XMMatrixInverse(&detView, mCamera.View());

        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(md3dImmediateContext->Map(mInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

        InstanceData* dataPtr = reinterpret_cast<InstanceData*>(mappedData.pData);

        for (UINT i = 0; i < mInstanceData.size(); ++i)
        {
            XMMATRIX W = XMLoadFloat4x4(&mInstanceData[i].World);
            XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

            // View space to the object's local space.
            XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

            // Decompose the matrix into its individual parts.
            XMVECTOR scale, rotQuat, translation;
            XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

            // Transform the camera frustum from view space to the object's local space.
            BoundingFrustum localSpaceFrustum;
            mCameraFrustum.Transform(localSpaceFrustum, XMVectorGetX(scale), rotQuat, translation);

            // Perform the box/frustum intersection test in local space.
            if (localSpaceFrustum.Contains(mSkullBox) != DISJOINT)
            {
                // Write the instance data to dynamic VB of the visible objects.
                dataPtr[mVisibleObjectCount++] = mInstanceData[i];
            }
        }

        md3dImmediateContext->Unmap(mInstanceBuffer, 0);
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(md3dImmediateContext->Map(mInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

        InstanceData* dataPtr = reinterpret_cast<InstanceData*>(mappedData.pData);

        for (UINT i = 0; i < mInstanceData.size(); ++i)
        {
            dataPtr[mVisibleObjectCount++] = mInstanceData[i];
        }

        md3dImmediateContext->Unmap(mInstanceBuffer, 0);
    }

    std::wostringstream outs;
    outs.precision(6);
    outs << L"Instancing and Culling Demo" <<
        L"    " << mVisibleObjectCount <<
        L" objects visible out of " << mInstanceData.size();
    mMainWndCaption = outs.str();
}

void InstancingAndCulling::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dImmediateContext->IASetInputLayout(mInputLayout);

    UINT stride[2] = { sizeof(Vertex), sizeof(InstanceData) };
    UINT offset[2] = { 0, 0 };
    ID3D11Buffer* vbs[2] = { mSkullVB, mInstanceBuffer };
    md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
    md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->VSSetShader(mVS, 0, 0);
    md3dImmediateContext->PSSetShader(mPS, 0, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* framePtr = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);
    framePtr->DirLights[0] = mDirLights[0];
    framePtr->DirLights[1] = mDirLights[1];
    framePtr->DirLights[2] = mDirLights[2];
    framePtr->EyePosW = mCamera.GetPosition();

    md3dImmediateContext->Unmap(mPerFrameBuffer, 0);
    md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    XMMATRIX skullWorld = XMLoadFloat4x4(&mSkullWorld);
    PerObjectBuffer* objectPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
    objectPtr->World = XMMatrixTranspose(skullWorld);
    objectPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(skullWorld), skullWorld);
    objectPtr->ViewProj = XMMatrixTranspose(mCamera.ViewProj());
    objectPtr->Mat = mSkullMat;

    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

    md3dImmediateContext->DrawIndexedInstanced(mSkullIndexCount, mVisibleObjectCount, 0, 0, 0);

    HR(mSwapChain->Present(0, 0));
}

void InstancingAndCulling::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void InstancingAndCulling::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void InstancingAndCulling::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mCamera.Pitch(dy);
        mCamera.RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void InstancingAndCulling::BuildSkullGeometryBuffer()
{
    std::ifstream fin(ExePath().append(L"../../../Models/skull.txt").c_str());

    if (!fin)
    {
        MessageBox(0, L"skull.txt not found.", 0, 0);
        return;
    }

    UINT vcount = 0;
    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> vcount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    XMFLOAT3 vMinf3(MathHelper::Infinity, MathHelper::Infinity, MathHelper::Infinity);
    XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
    std::vector<Vertex> vertices(vcount);
    for (UINT i = 0; i < vertices.size(); ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
        fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

        XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

        vMin = XMVectorMin(vMin, P);
        vMax = XMVectorMax(vMax, P);
    }
    XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin + vMax));
    XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax - vMin));

    fin >> ignore >> ignore >> ignore;

    mSkullIndexCount = 3 * tcount;
    std::vector<UINT> indices(mSkullIndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> indices[i * 3] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.StructureByteStride = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = indices.data();
    HR(md3dDevice->CreateBuffer(&ibd, &initData, &mSkullIB));
}

void InstancingAndCulling::BuildInstanceBuffer()
{
    const int n = 5;
    mInstanceData.resize(n*n*n);

    float width = 200.0f;
    float height = 200.0f;
    float depth = 200.0f;

    float x = -0.5f * width;
    float y = -0.5f * height;
    float z = -0.5f * depth;
    float dx = width / (n - 1);
    float dy = height / (n - 1);
    float dz = depth / (n - 1);
    for (int k = 0; k < n; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                // Position instanced along a 3D grid.
                mInstanceData[k * n * n + i * n + j].World = XMFLOAT4X4(
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    x + j * dx, y + i * dy, z + k * dz, 1.0f);

                mInstanceData[k*n*n + i * n + j].Color.x = MathHelper::RandF(0.0f, 1.0f);
                mInstanceData[k*n*n + i * n + j].Color.y = MathHelper::RandF(0.0f, 1.0f);
                mInstanceData[k*n*n + i * n + j].Color.z = MathHelper::RandF(0.0f, 1.0f);
                mInstanceData[k*n*n + i * n + j].Color.w = 1.0f;
            }
        }
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(InstanceData) * mInstanceData.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.StructureByteStride = 0;
    vbd.MiscFlags = 0;

    HR(md3dDevice->CreateBuffer(&vbd, 0, &mInstanceBuffer));
}

void InstancingAndCulling::BuildFX()
{
    const D3D11_INPUT_ELEMENT_DESC vertexDesc[7] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };

    auto filename = ExePath().append(L"../../../Shaders/BasicInstance.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mVS, cstr, "VS", 0, &mInputLayout, vertexDesc, 7);
    ShaderHelper::CreateShader(md3dDevice, &mPS, cstr, "PS", 0);

    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.StructureByteStride = 0;
    matrixBufferDesc.MiscFlags = 0;

    HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerFrameBuffer));

    matrixBufferDesc.ByteWidth = sizeof(PerObjectBuffer);

    HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerObjectBuffer));
}