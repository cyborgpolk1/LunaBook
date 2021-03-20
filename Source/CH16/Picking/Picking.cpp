#include "Picking.h"
#include "MathHelper.h"
#include <fstream>
#include <string>

D3DMAIN(PickingDemo);

PickingDemo::PickingDemo(HINSTANCE hInstance)
    : D3DApp(hInstance), mCarVB(0), mCarIB(0), mVS(0), mPS(0),
    mInputLayout(0), mPerFrameBuffer(0), mPerObjectBuffer(0),
    mCarIndexCount(0), mPickedTriangle(-1), mLessEqualDS(0), 
    mWireframeRS(0), mWireframe(false), heldDown(false)
{
    mMainWndCaption = L"Picking Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCamera.SetPosition(0.0f, 2.0f, -15.0f);

    XMMATRIX CarScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX CarOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mCarWorld, XMMatrixMultiply(CarScale, CarOffset));

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
    
    mCarMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mCarMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mCarMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    mPickedMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
    mPickedMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
    mPickedMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

PickingDemo::~PickingDemo()
{
    ReleaseCOM(mCarVB);
    ReleaseCOM(mCarIB);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mVS);
    ReleaseCOM(mPS);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mPerObjectBuffer);
    ReleaseCOM(mLessEqualDS);
    ReleaseCOM(mWireframeRS);
}

bool PickingDemo::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildCarGeometryBuffer();
    BuildFX();

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    HR(md3dDevice->CreateDepthStencilState(&dsDesc, &mLessEqualDS));

    D3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    HR(md3dDevice->CreateRasterizerState(&rsDesc, &mWireframeRS));

    return true;
}

void PickingDemo::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void PickingDemo::UpdateScene(float dt)
{
    if (GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(10.0f*dt);

    if (GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-10.0f*dt);

    if (GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-10.0f*dt);

    if (GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(10.0f*dt);

    if (GetAsyncKeyState('1') & 0x8000)
    {
        if (!heldDown) {
            mWireframe = !mWireframe;
            heldDown = true;
        }
    }
    else
    {
        heldDown = false;
    }


    mCamera.UpdateViewMatrix();
}

void PickingDemo::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    md3dImmediateContext->IASetInputLayout(mInputLayout);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mCarVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mCarIB, DXGI_FORMAT_R32_UINT, 0);

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

    XMMATRIX world = XMLoadFloat4x4(&mCarWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerObjectBuffer* objPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
    objPtr->World = XMMatrixTranspose(world);
    objPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    objPtr->WorldViewProj = XMMatrixTranspose(world * mCamera.ViewProj());
    objPtr->TexTransform = XMMatrixIdentity();
    objPtr->Mat = mCarMat;

    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

    if (mWireframe)
        md3dImmediateContext->RSSetState(mWireframeRS);

    md3dImmediateContext->DrawIndexed(mCarIndexCount, 0, 0);

    md3dImmediateContext->RSSetState(0);

    // Draw just the picked triangle again with a different material to hightlight it.
    if (mPickedTriangle != -1)
    {
        // Change depth test from < to <= so that if we draw the same
        // triangle twice, it will sitll pass the depth test. This
        // is because we redraw the picked triangle with a different
        // material to highlight it. If we do not use <=, the triangle
        // will fall the depth test the 2nd time we try and draw it.
        md3dImmediateContext->OMSetDepthStencilState(mLessEqualDS, 0);

        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

        objPtr = reinterpret_cast<PerObjectBuffer*>(mappedResource.pData);
        objPtr->World = XMMatrixTranspose(world);
        objPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        objPtr->WorldViewProj = XMMatrixTranspose(world * mCamera.ViewProj());
        objPtr->TexTransform = XMMatrixIdentity();
        objPtr->Mat = mPickedMat;

        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);

        // Just draw one triangle-3 indices. Offset to the picked
        // triangle in the mesh index buffer.
        md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

        md3dImmediateContext->OMSetDepthStencilState(0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}

void PickingDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mhMainWnd);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        Pick(x, y);
    }
}

void PickingDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void PickingDemo::OnMouseMove(WPARAM btnState, int x, int y)
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

void PickingDemo::BuildCarGeometryBuffer()
{
    std::ifstream fin(ExePath().append(L"../../../Models/car.txt").c_str());

    if (!fin)
    {
        MessageBox(0, L"car.txt not found.", 0, 0);
        return;
    }

    UINT vcount = 0;
    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> vcount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    XMFLOAT3 vMin3f(MathHelper::Infinity, MathHelper::Infinity, MathHelper::Infinity);
    XMFLOAT3 vMax3f(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

    XMVECTOR vMin = XMLoadFloat3(&vMin3f);
    XMVECTOR vMax = XMLoadFloat3(&vMax3f);
    mCarVertices.resize(vcount);
    for (UINT i = 0; i < mCarVertices.size(); ++i)
    {
        fin >> mCarVertices[i].Pos.x >> mCarVertices[i].Pos.y >> mCarVertices[i].Pos.z;
        fin >> mCarVertices[i].Normal.x >> mCarVertices[i].Normal.y >> mCarVertices[i].Normal.z;

        XMVECTOR P = XMLoadFloat3(&mCarVertices[i].Pos);

        vMin = XMVectorMin(vMin, P);
        vMax = XMVectorMax(vMax, P);
    }
    XMStoreFloat3(&mCarBox.Center, 0.5f * (vMin + vMax));
    XMStoreFloat3(&mCarBox.Extents, 0.5f * (vMax - vMin));

    fin >> ignore >> ignore >> ignore;

    mCarIndexCount = 3 * tcount;
    mCarIndices.resize(mCarIndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> mCarIndices[i * 3] >> mCarIndices[i * 3 + 1] >> mCarIndices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * mCarVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.StructureByteStride = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = mCarVertices.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mCarVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mCarIndices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = mCarIndices.data();
    HR(md3dDevice->CreateBuffer(&ibd, &initData, &mCarIB));
}

void PickingDemo::BuildFX()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto filename = ExePath().append(L"../../../Shaders/BasicEffect.hlsl");
    auto cstr = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mVS, cstr, "VS", 0, &mInputLayout, vertexDesc, 2);
    ShaderHelper::CreateShader(md3dDevice, &mPS, cstr, "PS", 0);

    D3D11_BUFFER_DESC matrixDesc;
    matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixDesc.ByteWidth = sizeof(PerFrameBuffer);
    matrixDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixDesc.MiscFlags = 0;
    matrixDesc.StructureByteStride = 0;
    matrixDesc.Usage = D3D11_USAGE_DYNAMIC;

    HR(md3dDevice->CreateBuffer(&matrixDesc, 0, &mPerFrameBuffer));

    matrixDesc.ByteWidth = sizeof(PerObjectBuffer);

    HR(md3dDevice->CreateBuffer(&matrixDesc, 0, &mPerObjectBuffer));
}

void PickingDemo::Pick(int sx, int sy)
{
    XMFLOAT4X4 P;
    XMStoreFloat4x4(&P, mCamera.Proj());

    // Compute picking ray in view space.
    float vx = (2.0f * sx / mClientWidth - 1.0f) / P(0,0);
    float vy = (-2.0f * sy / mClientHeight + 1.0f) / P(1,1);

    XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

    // Transform ray to local space of Mesh.
    XMMATRIX V = mCamera.View();
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

    XMMATRIX W = XMLoadFloat4x4(&mCarWorld);
    XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

    XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

    rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
    rayDir = XMVector3TransformNormal(rayDir, toLocal);

    // Make the ray direction unit length for the intersection tests.
    rayDir = XMVector3Normalize(rayDir);

    // If we hit the bounding box of the Mesh, then we might have picked
    // a Mesh triangle, so do the ray/triangle tests.
    //
    // If we did not hit the bounding box, then it is impossible that we
    // hit the Mesh, so do not waste effort doing ray/triangle tests.
    mPickedTriangle = -1;
    float tmin = 0.0f;
    if (mCarBox.Intersects(rayOrigin, rayDir, tmin))
    {
        // Find the nearest ray/triangle intersection.
        tmin = MathHelper::Infinity;
        for (UINT i = 0; i < mCarIndices.size() / 3; ++i)
        {
            // Indices for this triangle
            UINT i0 = mCarIndices[i * 3];
            UINT i1 = mCarIndices[i * 3 + 1];
            UINT i2 = mCarIndices[i * 3 + 2];

            // Vertices for thsi triangle.
            XMVECTOR v0 = XMLoadFloat3(&mCarVertices[i0].Pos);
            XMVECTOR v1 = XMLoadFloat3(&mCarVertices[i1].Pos);
            XMVECTOR v2 = XMLoadFloat3(&mCarVertices[i2].Pos);

            // We have to iterate over all the triangles in order to find
            // the nearest intersection.
            float t = 0.0f;
            if (TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
            {
                if (t < tmin)
                {
                    // This is the new nearest picked triangle.
                    tmin = t;
                    mPickedTriangle = i;
                }
            }
        }
    }
}