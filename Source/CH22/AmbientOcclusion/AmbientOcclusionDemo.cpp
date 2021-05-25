#include "AmbientOcclusionDemo.h"
#include <d3dcompiler.h>
#include "MathHelper.h"
#include "Octree.h"
#include <fstream>
#include <string>
#include <vector>

D3DMAIN(AmbientOcclusionDemo);

AmbientOcclusionDemo::AmbientOcclusionDemo(HINSTANCE hInstance)
    : D3DApp(hInstance), mSkullVB(0), mSkullIB(0), mVS(0), mPS(0), mMatrixBuffer(0),
    mInputLayout(0), mSkullIndexCount(0),
    mTheta(1.5f*MathHelper::Pi), mPhi(0.1f*MathHelper::Pi), mRadius(20.0f)
{
    mMainWndCaption = L"Ambient Occlusion Demo";

    mLastMousePoint.x = 0;
    mLastMousePoint.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
    XMStoreFloat4x4(&mWorld, T);
}

AmbientOcclusionDemo::~AmbientOcclusionDemo()
{
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mVS);
    ReleaseCOM(mPS);
    ReleaseCOM(mMatrixBuffer);
    ReleaseCOM(mInputLayout);
}

bool AmbientOcclusionDemo::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildGeometryBuffer();
    BuildFX();

    return true;
}

void AmbientOcclusionDemo::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void AmbientOcclusionDemo::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void AmbientOcclusionDemo::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->VSSetShader(mVS, 0, 0);
    md3dImmediateContext->PSSetShader(mPS, 0, 0);

    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world * view * proj;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(md3dImmediateContext->Map(mMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    MatrixBuffer* dataPtr = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
    dataPtr->WorldViewProj = XMMatrixTranspose(worldViewProj);

    md3dImmediateContext->Unmap(mMatrixBuffer, 0);
    md3dImmediateContext->VSSetConstantBuffers(0, 1, &mMatrixBuffer);

    md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

    HR(mSwapChain->Present(0, 0));
}

void AmbientOcclusionDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePoint.x = x;
    mLastMousePoint.y = y;

    SetCapture(mhMainWnd);
}

void AmbientOcclusionDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void AmbientOcclusionDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePoint.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePoint.y));

        mTheta += dx;
        mPhi += dy;

        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.05f*static_cast<float>(x - mLastMousePoint.x);
        float dy = 0.05f*static_cast<float>(y - mLastMousePoint.y);

        mRadius += dx - dy;

        mRadius = MathHelper::Clamp(mRadius, 5.0f, 50.0f);
    }

    mLastMousePoint.x = x;
    mLastMousePoint.y = y;
}

void AmbientOcclusionDemo::BuildGeometryBuffer()
{
    std::ifstream fin(ExePath().append(L"../../../Models/skull.txt").c_str());

    if (!fin)
    {
        MessageBox(0, L"Models/skull.txt not found.", 0, 0);
        return;
    }

    UINT vcount = 0;
    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> vcount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    std::vector<Vertex> vertices(vcount);
    for (UINT i = 0; i < vcount; ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
        fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
    }

    fin >> ignore >> ignore >> ignore;

    mSkullIndexCount = 3 * tcount;
    std::vector<UINT> indices(mSkullIndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    BuildVertexAmbientOcclusion(vertices, indices);

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices.data();
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

    vbd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    vbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = indices.data();
    HR(md3dDevice->CreateBuffer(&vbd, &initData, &mSkullIB));
}

void AmbientOcclusionDemo::BuildVertexAmbientOcclusion(std::vector<Vertex>& vertices, const std::vector<UINT>& indices)
{
    UINT vcount = vertices.size();
    UINT tcount = indices.size() / 3;

    std::vector<XMFLOAT3> positions(vcount);
    for (UINT i = 0; i < vcount; ++i)
        positions[i] = vertices[i].Pos;

    Octree octree;
    octree.Build(positions, indices);

    // For each vertex, count how many triangles contain the vertex.
    std::vector<int> vertexSharedCount(vcount);

    // Cast rays for each triangle, and average triangle occlusion
    // with the vertices that share this triangle.
    for (UINT i = 0; i < tcount; ++i)
    {
        UINT i0 = indices[i * 3 + 0];
        UINT i1 = indices[i * 3 + 1];
        UINT i2 = indices[i * 3 + 2];

        XMVECTOR v0 = XMLoadFloat3(&vertices[i0].Pos);
        XMVECTOR v1 = XMLoadFloat3(&vertices[i1].Pos);
        XMVECTOR v2 = XMLoadFloat3(&vertices[i2].Pos);

        XMVECTOR edge0 = v1 - v0;
        XMVECTOR edge1 = v2 - v0;

        XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

        XMVECTOR centroid = (v0 + v1 + v2) / 3.0f;

        // Offset to avoid self intersection.
        centroid += 0.001f*normal;

        const int NumSampleRays = 32;
        float numUnoccluded = 0.0f;
        for (int j = 0; j < NumSampleRays; ++j)
        {
            XMVECTOR randomDir = MathHelper::RandHemisphereUnitVec3(normal);

            // Test if the random ray intersects the scene mesh.
            //
            // TODO: Technically we should not count intersections
            // that are far away as occluding the triangle, but
            // this is OK for demo.
            if (!octree.RayOctreeIntersect(centroid, randomDir))
            {
                numUnoccluded++;
            }
        }

        float ambientAccess = numUnoccluded / NumSampleRays;

        // Average with vertices that share this face.
        vertices[i0].AmbientOcc += ambientAccess;
        vertices[i1].AmbientOcc += ambientAccess;
        vertices[i2].AmbientOcc += ambientAccess;

        vertexSharedCount[i0]++;
        vertexSharedCount[i1]++;
        vertexSharedCount[i2]++;
    }

    // Finish average by dividing by the number of samples we added
    // and store in the vertex attributes.
    for (UINT i = 0; i < vcount; ++i)
    {
        vertices[i].AmbientOcc /= vertexSharedCount[i];
    }
}

void AmbientOcclusionDemo::BuildFX()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"AMBIENT", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    auto filename = ExePath().append(L"../../../Shaders/AmbientOcclusion.hlsl");
    auto filestring = filename.c_str();

    ShaderHelper::CreateShader(md3dDevice, &mVS, filestring, "VS", 0, &mInputLayout, vertexDesc, 3);
    ShaderHelper::CreateShader(md3dDevice, &mPS, filestring, "PS", 0);

    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    
    HR(md3dDevice->CreateBuffer(&matrixBufferDesc, 0, &mMatrixBuffer));
}