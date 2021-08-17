#include "QuatDemo.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include <vector>
#include <fstream>
#include <string>

D3DMAIN(QuatDemo);

QuatDemo::QuatDemo(HINSTANCE hInstance)
    : D3DApp(hInstance), mShapesVB(0), mShapesIB(0), mShapeInputLayout(0), mTexVS(0), mPerObjectBuffer(0), mPerFrameBuffer(0),
    mBoxVertexOffset(0), mGridVertexOffset(0), mSphereVertexOffset(0), mCylinderVertexOffset(0), mTexPS(0),
    mBoxIndexCount(0), mGridIndexCount(0), mSphereIndexCount(0), mCylinderIndexCount(0), mSkullIndexCount(0),
    mBoxIndexOffset(0), mGridIndexOffset(0), mSphereIndexOffset(0), mCylinderIndexOffset(0),
    mTheta(1.5f*MathHelper::Pi), mPhi(0.1f*MathHelper::Pi), mRadius(20.0f), mLightCount(0), mSkullVB(0), mSkullIB(0)
{
    mMainWndCaption = L"Quaternion Rotation Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    XMStoreFloat4x4(&mGridWorld, I);

    XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    // We create 5 rows of 2 cylinders and spheres per row
    for (int i = 0; i < 5; ++i)
    {
        XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(5.0f, 1.5f, -10.0f + i * 5.0f));

        XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(5.0f, 3.5f, -10.0f + i * 5.0f));
    }

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

    mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mGridMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    mSphereMat.Ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
    mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
    mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

    mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    XMVECTOR q0 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(30.0f));
    XMVECTOR q1 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 1.0f, 2.0f, 0.0f), XMConvertToRadians(45.0f));
    XMVECTOR q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(-30.0f));
    XMVECTOR q3 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(70.0f));

    mSkullAnimation.Keyframes.resize(5);

    mSkullAnimation.Keyframes[0].TimePos = 0.0f;
    mSkullAnimation.Keyframes[0].Translation = XMFLOAT3(-7.0f, 0.0f, 0.0f);
    mSkullAnimation.Keyframes[0].Scale = XMFLOAT3(0.25f, 0.25f, 0.25f);
    XMStoreFloat4(&mSkullAnimation.Keyframes[0].RotationQuat, q0);

    mSkullAnimation.Keyframes[1].TimePos = 2.0f;
    mSkullAnimation.Keyframes[1].Translation = XMFLOAT3(0.0f, 2.0f, 10.0f);
    mSkullAnimation.Keyframes[1].Scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
    XMStoreFloat4(&mSkullAnimation.Keyframes[1].RotationQuat, q1);

    mSkullAnimation.Keyframes[2].TimePos = 4.0f;
    mSkullAnimation.Keyframes[2].Translation = XMFLOAT3(7.0f, 0.0f, 0.0f);
    mSkullAnimation.Keyframes[2].Scale = XMFLOAT3(0.25f, 0.25f, 0.25f);
    XMStoreFloat4(&mSkullAnimation.Keyframes[2].RotationQuat, q2);

    mSkullAnimation.Keyframes[3].TimePos = 6.0f;
    mSkullAnimation.Keyframes[3].Translation = XMFLOAT3(0.0f, 1.0f, -10.0f);
    mSkullAnimation.Keyframes[3].Scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
    XMStoreFloat4(&mSkullAnimation.Keyframes[3].RotationQuat, q3);

    mSkullAnimation.Keyframes[4].TimePos = 8.0f;
    mSkullAnimation.Keyframes[4].Translation = XMFLOAT3(-7.0f, 0.0f, 0.0f);
    mSkullAnimation.Keyframes[4].Scale = XMFLOAT3(0.25f, 0.25f, 0.25f);
    XMStoreFloat4(&mSkullAnimation.Keyframes[4].RotationQuat, q0);
}

QuatDemo::~QuatDemo()
{
    ReleaseCOM(mShapesVB);
    ReleaseCOM(mShapesIB);
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mShapeInputLayout);
    ReleaseCOM(mTexVS);
    ReleaseCOM(mTexPS);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mPerObjectBuffer);
    ReleaseCOM(mFloorTex);
    ReleaseCOM(mBrickTex);
    ReleaseCOM(mStoneTex);
    ReleaseCOM(mSampleState);
}

bool QuatDemo::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildShapeGeometryBuffer();
    BuildSkullGeometryBuffer();
    BuildFX();
    BuildTex();

    return true;
}

void QuatDemo::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void QuatDemo::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    mEyePosW = XMFLOAT3(x, y, z);

    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);

    // Increase the time position.
    mAnimTimePos += dt;
    if (mAnimTimePos >= mSkullAnimation.GetEndTime())
    {
        // Loop animation back to beginning.
        mAnimTimePos = 0.0f;
    }

    // Get the skull's world matrix at this time instant.
    mSkullAnimation.Interpolate(mAnimTimePos, mSkullWorld);
}

void QuatDemo::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(mShapeInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(TexVertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
    md3dImmediateContext->PSSetShader(mTexPS, 0, 0);

    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX viewProj = view * proj;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* frameDataPtr = (PerFrameBuffer*)mappedResource.pData;
    frameDataPtr->EyePosW = mEyePosW;
    frameDataPtr->DirLights[0] = mDirLights[0];
    frameDataPtr->DirLights[1] = mDirLights[1];
    frameDataPtr->DirLights[2] = mDirLights[2];

    md3dImmediateContext->Unmap(mPerFrameBuffer, 0);
    md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    // Draw the grid
    XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    PerObjectBuffer* dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixScaling(5.0f, 5.0f, 0.0f);
    dataPtr->Mat = mGridMat;
    dataPtr->Options = USE_TEXTURES;
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(0, 1, &mFloorTex);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

    // Draw the box
    world = XMLoadFloat4x4(&mBoxWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixIdentity();
    dataPtr->Mat = mBoxMat;
    dataPtr->Options = USE_TEXTURES;
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
    md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
    md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

    // Draw the cylinders
    for (int i = 0; i < 10; ++i)
    {
        world = XMLoadFloat4x4(&mCylWorld[i]);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        dataPtr->Mat = mCylinderMat;
        dataPtr->Options = USE_TEXTURES;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mBrickTex);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
    }

    // Draw the spheres
    for (int i = 0; i < 10; ++i)
    {
        world = XMLoadFloat4x4(&mSphereWorld[i]);
        HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        dataPtr = (PerObjectBuffer*)mappedResource.pData;
        dataPtr->World = XMMatrixTranspose(world);
        dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
        dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
        dataPtr->gTexTransform = XMMatrixIdentity();
        dataPtr->Mat = mSphereMat;
        dataPtr->Options = USE_TEXTURES;
        md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
        md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
        md3dImmediateContext->PSSetShaderResources(0, 1, &mStoneTex);
        md3dImmediateContext->PSSetSamplers(0, 1, &mSampleState);
        md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
    }

    md3dImmediateContext->IASetInputLayout(mShapeInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    stride = sizeof(BasicVertex);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->VSSetShader(mTexVS, 0, 0);
    md3dImmediateContext->PSSetShader(mTexPS, 0, 0);

    md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    // Draw skull
    world = XMLoadFloat4x4(&mSkullWorld);
    HR(md3dImmediateContext->Map(mPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    dataPtr = (PerObjectBuffer*)mappedResource.pData;
    dataPtr->World = XMMatrixTranspose(world);
    dataPtr->WorldViewProj = XMMatrixTranspose(world*view*proj);
    dataPtr->WorldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);
    dataPtr->gTexTransform = XMMatrixIdentity();
    dataPtr->Mat = mSkullMat;
    dataPtr->Options = STANDARD_LIGHTING;
    md3dImmediateContext->Unmap(mPerObjectBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectBuffer);
    md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

    HR(mSwapChain->Present(0, 0));
}

void QuatDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void QuatDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void QuatDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        mTheta += dx;
        mPhi += dy;

        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

        mRadius += dx - dy;

        mRadius = MathHelper::Clamp(mRadius, 3.0f, 200.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void QuatDemo::BuildShapeGeometryBuffer()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
    geoGen.CreateSphere(0.5f, 20, 20, sphere);
    geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;
    mGridVertexOffset = (UINT)box.Vertices.size();
    mSphereVertexOffset = mGridVertexOffset + (UINT)grid.Vertices.size();
    mCylinderVertexOffset = mSphereVertexOffset + (UINT)sphere.Vertices.size();

    // Cache the index count of each object.
    mBoxIndexCount = (UINT)box.Indices.size();
    mGridIndexCount = (UINT)grid.Indices.size();
    mSphereIndexCount = (UINT)sphere.Indices.size();
    mCylinderIndexCount = (UINT)cylinder.Indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;
    mGridIndexOffset = mBoxIndexCount;
    mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
    mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

    UINT totalVertexCount = (UINT)(box.Vertices.size() + grid.Vertices.size() + sphere.Vertices.size() + cylinder.Vertices.size());

    UINT totalIndexCount = mBoxIndexCount + mGridIndexCount + mSphereIndexCount + mCylinderIndexCount;

    // Extract the vertex elements we are interested in and pack all
    // the vertices of all meshes into one vertex buffer
    std::vector<TexVertex> vertices(totalVertexCount);

    XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = box.Vertices[i].Position;
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].Tex = box.Vertices[i].TexC;
    }

    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = grid.Vertices[i].Position;
        vertices[k].Normal = grid.Vertices[i].Normal;
        vertices[k].Tex = grid.Vertices[i].TexC;
    }

    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = sphere.Vertices[i].Position;
        vertices[k].Normal = sphere.Vertices[i].Normal;
        vertices[k].Tex = sphere.Vertices[i].TexC;
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = cylinder.Vertices[i].Position;
        vertices[k].Normal = cylinder.Vertices[i].Normal;
        vertices[k].Tex = cylinder.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(TexVertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

    // Pack the indices of all the meshes into one index buffer.
    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
    indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
    indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
    indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &initData, &mShapesIB));
}

void QuatDemo::BuildSkullGeometryBuffer()
{
    // Load skull
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

    std::vector<BasicVertex> skullVertices(vcount);
    for (UINT i = 0; i < vcount; ++i)
    {
        fin >> skullVertices[i].Pos.x >> skullVertices[i].Pos.y >> skullVertices[i].Pos.z;

        fin >> skullVertices[i].Normal.x >> skullVertices[i].Normal.y >> skullVertices[i].Normal.z;
    }

    fin >> ignore >> ignore >> ignore;

    mSkullIndexCount = 3 * tcount;
    std::vector<UINT> skullIndices(mSkullIndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> skullIndices[i * 3 + 0] >> skullIndices[i * 3 + 1] >> skullIndices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(BasicVertex) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &skullVertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &skullIndices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &initData, &mSkullIB));
}

void QuatDemo::BuildFX()
{
    // Create the vertex input layout
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    D3D_SHADER_MACRO basicEffectDefines[] = {
        { "NUM_LIGHTS", "3" },
        { 0, 0 }
    };

    ShaderHelper::CreateShader(md3dDevice, &mTexVS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "VS", 0, &mShapeInputLayout, vertexDesc, 3);
    ShaderHelper::CreateShader(md3dDevice, &mTexPS, ExePath().append(L"../../../Shaders/BasicEffectTex.hlsl").c_str(), "PS", basicEffectDefines);

    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.ByteWidth = sizeof(PerObjectBuffer);
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mPerObjectBuffer));

    D3D11_BUFFER_DESC matrixBufferDesc2;
    matrixBufferDesc2.ByteWidth = sizeof(PerFrameBuffer);
    matrixBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc2.MiscFlags = 0;
    matrixBufferDesc2.StructureByteStride = 0;

    HR(md3dDevice->CreateBuffer(&matrixBufferDesc2, 0, &mPerFrameBuffer));
}

void QuatDemo::BuildTex()
{
    ID3D11Resource *floorResource, *brickResource, *stoneResource;

    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/floor.dds").c_str(), &floorResource, &mFloorTex));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/bricks.dds").c_str(), &brickResource, &mBrickTex));
    HR(CreateDDSTextureFromFile(md3dDevice, ExePath().append(L"../../../Textures/stone.dds").c_str(), &stoneResource, &mStoneTex));

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HR(md3dDevice->CreateSamplerState(&samplerDesc, &mSampleState));

    ReleaseCOM(floorResource);
    ReleaseCOM(brickResource);
    ReleaseCOM(stoneResource);
}