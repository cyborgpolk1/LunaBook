#include "ParticleSystem.h"

ParticleSystem::ParticleSystem() 
    : mInitVB(0), mDrawVB(0), mStreamOutVB(0), mTexArraySRV(0), mRandomTexSRV(0),
    mFirstRun(true), mGameTime(0.0f), mTimeStep(0.0f), mAge(0.0f), mPerFrameBuffer(0),
    mSamLinear(0), mStreamVS(0), mStreamGS(0), mDrawVS(0), mDrawGS(0), mPS(0), mPerformBlend(false)
{
    mEyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mEmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

ParticleSystem::~ParticleSystem() 
{
    ReleaseCOM(mInitVB);
    ReleaseCOM(mDrawVB);
    ReleaseCOM(mStreamOutVB);
    ReleaseCOM(mPerFrameBuffer);
    ReleaseCOM(mSamLinear);
    ReleaseCOM(mStreamVS);
    ReleaseCOM(mStreamGS);
    ReleaseCOM(mDrawVS);
    ReleaseCOM(mDrawGS);
    ReleaseCOM(mPS);
    ReleaseCOM(mDisableDepthDS);
    ReleaseCOM(mNoDepthWritesDS);
    ReleaseCOM(mAdditiveBlendState);
}

float ParticleSystem::GetAge() const {
    return mAge;
}

void ParticleSystem::SetEyePos(const XMFLOAT3& eyePosW) 
{
    mEyePosW = eyePosW;
}

void ParticleSystem::SetEmitPos(const XMFLOAT3& emitPosW) 
{
    mEmitPosW = emitPosW;
}

void ParticleSystem::SetEmitDir(const XMFLOAT3& emitDirW) 
{
    mEmitDirW = emitDirW;
}

void ParticleSystem::Reset() 
{
    mFirstRun = true;
    mAge = 0.0f;
}

void ParticleSystem::Update(float dt, float gameTime) 
{
    mGameTime = gameTime;
    mTimeStep = dt;

    mAge += dt;
}

void ParticleSystem::Init(ID3D11Device* device,
    ParticleSystem::SystemType type,
    ID3D11ShaderResourceView* texArraySRV, 
    ID3D11ShaderResourceView* randomTexSRV, 
    UINT maxParticles)
{
    mMaxParticles = maxParticles;

    mTexArraySRV = texArraySRV;
    mRandomTexSRV = randomTexSRV;

    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    D3D11_SO_DECLARATION_ENTRY streamDesc[] =
    {
        { 0, "POSITION", 0, 0, 3, 0},
        { 0, "VELOCITY", 0, 0, 3, 0},
        { 0, "SIZE", 0, 0, 2, 0},
        { 0, "AGE", 0, 0, 1, 0},
        { 0, "TYPE", 0, 0, 1, 0},
    };

    auto fxFile = ExePath().append(L"../../../Shaders/");

    switch (type) 
    {
        case FIRE:
            fxFile.append(L"Fire.hlsl");
            mPerformBlend = true;
            break;

        case RAIN:
            fxFile.append(L"Rain.hlsl");
            mPerformBlend = false;
            break;

        default:
            fxFile.append(L"Fire.hlsl");
            mPerformBlend = false;
            break;
    }

    ShaderHelper::CreateShader(device, &mStreamVS, fxFile.c_str(), "StreamOutVS", 0, &mInputLayout, vertexDesc, 5);
    ShaderHelper::CreateShader(device, &mStreamGS, fxFile.c_str(), "StreamOutGS", 0, streamDesc, 5);

    ShaderHelper::CreateShader(device, &mDrawVS, fxFile.c_str(), "DrawVS", 0);
    ShaderHelper::CreateShader(device, &mDrawGS, fxFile.c_str(), "DrawGS", 0);
    ShaderHelper::CreateShader(device, &mPS, fxFile.c_str(), "DrawPS", 0);

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    HR(device->CreateBuffer(&bufferDesc, 0, &mPerFrameBuffer));

    D3D11_DEPTH_STENCIL_DESC dsDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
    dsDesc.DepthEnable = false;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    HR(device->CreateDepthStencilState(&dsDesc, &mDisableDepthDS));

    dsDesc.DepthEnable = true;

    HR(device->CreateDepthStencilState(&dsDesc, &mNoDepthWritesDS));

    D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

    HR(device->CreateBlendState(&blendDesc, &mAdditiveBlendState));

    D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

    HR(device->CreateSamplerState(&samplerDesc, &mSamLinear));

    BuildVB(device);
}

void ParticleSystem::Draw(ID3D11DeviceContext* dc, const Camera& cam) 
{
    XMMATRIX VP = cam.ViewProj();

    dc->IASetInputLayout(mInputLayout);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    // On the first pass, use the initialization VB. Otherwise, use
    // the VB that contains the current particle list.
    if (mFirstRun)
        dc->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);
    else
        dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(dc->Map(mPerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    PerFrameBuffer* bufferData = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);
    bufferData->EyePoseW = mEyePosW;
    bufferData->EmitPosW = mEmitPosW;
    bufferData->EmitDirW = mEmitDirW;
    bufferData->GameTime = mGameTime;
    bufferData->TimeStep = mTimeStep;
    bufferData->ViewProj = XMMatrixTranspose(VP);

    dc->Unmap(mPerFrameBuffer, 0);
    dc->GSSetConstantBuffers(0, 1, &mPerFrameBuffer);

    dc->GSSetShaderResources(1, 1, &mRandomTexSRV);
    dc->GSSetSamplers(0, 1, &mSamLinear);

    dc->PSSetShaderResources(0, 1, &mTexArraySRV);
    dc->PSSetSamplers(0, 1, &mSamLinear);

    //
    // Draw the current particle list using stream-out only to update them.
    // The updated vertices are streamed-out to the target.
    //
    dc->SOSetTargets(1, &mStreamOutVB, &offset);

    dc->VSSetShader(mStreamVS, 0, 0);
    dc->GSSetShader(mStreamGS, 0, 0);
    dc->PSSetShader(0, 0, 0);

    dc->OMSetDepthStencilState(mDisableDepthDS, 0);


    if (mFirstRun)
    {
        dc->Draw(1, 0);
        mFirstRun = false;
    }
    else
        dc->DrawAuto();

    // done streaming-out--undbind the vertex buffer
    ID3D11Buffer* bufferArray[1] = { 0 };
    dc->SOSetTargets(1, bufferArray, &offset);

    // ping-pong the vertex buffers
    std::swap(mDrawVB, mStreamOutVB);

    //
    // Draw the updated particle system we just streamed-out
    //
    dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

    dc->VSSetShader(mDrawVS, 0, 0);
    dc->GSSetShader(mDrawGS, 0, 0);
    dc->PSSetShader(mPS, 0, 0);

    dc->OMSetDepthStencilState(mNoDepthWritesDS, 0);

    float blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (mPerformBlend)
        dc->OMSetBlendState(mAdditiveBlendState, blendConstants, 0xffffffff);

    dc->DrawAuto();

    // Clear resources
    dc->GSSetConstantBuffers(0, 1, bufferArray);

    ID3D11ShaderResourceView* nullSRV = { 0 };
    dc->GSSetShaderResources(1, 1, &nullSRV);
    dc->PSSetShaderResources(0, 1, &nullSRV);

    ID3D11SamplerState* nullSampler = { 0 };
    dc->GSSetSamplers(0, 1, &nullSampler);
    dc->PSSetSamplers(0, 1, &nullSampler);

    dc->VSSetShader(0, 0, 0);
    dc->GSSetShader(0, 0, 0);
    dc->PSSetShader(0, 0, 0);

    dc->OMSetDepthStencilState(0, 0);
    dc->OMSetBlendState(0, blendConstants, 0xffffffff);
}

void ParticleSystem::BuildVB(ID3D11Device* device) 
{
    //
    // Create the buffer to kick-off the particle system.
    //
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(Vertex) * 1;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    // The initial particle emitter has type 0 and age 0. The rest
    // of the particle attributes do not apply to an emitter.
    Vertex p;
    ZeroMemory(&p, sizeof(Vertex));
    p.Age = 0.0f;
    p.Type = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &p;

    HR(device->CreateBuffer(&vbd, &vinitData, &mInitVB));

    //
    // Create the ping-pong buffers for stream-out and drawing.
    //
    vbd.ByteWidth = sizeof(Vertex) * mMaxParticles;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

    HR(device->CreateBuffer(&vbd, 0, &mDrawVB));
    HR(device->CreateBuffer(&vbd, 0, &mStreamOutVB));
}