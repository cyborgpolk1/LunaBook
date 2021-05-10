#pragma once

#include "d3dUtil.h"
#include "Camera.h"

class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem();

    enum SystemType {
        FIRE,
        RAIN
    };

    // Time elapsed since the system was reset.
    float GetAge() const;

    void SetEyePos(const XMFLOAT3& eyePosW);
    void SetEmitPos(const XMFLOAT3& emitPosW);
    void SetEmitDir(const XMFLOAT3& emitDirW);

    void Init(ID3D11Device* device,
        SystemType type,
        ID3D11ShaderResourceView* texArraySRV,
        ID3D11ShaderResourceView* randomTexSRV,
        UINT maxParticles);

    void Reset();
    void Update(float dt, float gameTime);
    void Draw(ID3D11DeviceContext* dc, const Camera& cam);

private:
    void BuildVB(ID3D11Device* device);

    ParticleSystem(const ParticleSystem& rhs);
    ParticleSystem& operator=(const ParticleSystem& rhs);

private:
    struct Vertex
    {
        XMFLOAT3 InitialPosW;
        XMFLOAT3 InitialVelW;
        XMFLOAT2 SizeW;
        float Age;
        UINT Type;
    };

    struct PerFrameBuffer
    {
        alignas(16) XMFLOAT3 EyePoseW;
        alignas(16) XMFLOAT3 EmitPosW;
        alignas(16) XMFLOAT3 EmitDirW;
        float GameTime;
        float TimeStep;
        alignas(16) XMMATRIX ViewProj;
    };

    UINT mMaxParticles;
    bool mFirstRun;

    float mGameTime;
    float mTimeStep;
    float mAge;

    XMFLOAT3 mEyePosW;
    XMFLOAT3 mEmitPosW;
    XMFLOAT3 mEmitDirW;

    ID3D11Buffer* mInitVB;
    ID3D11Buffer* mDrawVB;
    ID3D11Buffer* mStreamOutVB;

    ID3D11Buffer* mPerFrameBuffer;

    ID3D11ShaderResourceView* mTexArraySRV;
    ID3D11ShaderResourceView* mRandomTexSRV;

    ID3D11SamplerState* mSamLinear;

    ID3D11InputLayout* mInputLayout;

    ID3D11VertexShader* mStreamVS;
    ID3D11GeometryShader* mStreamGS;
    ID3D11VertexShader* mDrawVS;
    ID3D11GeometryShader* mDrawGS;
    ID3D11PixelShader* mPS;

    ID3D11DepthStencilState* mDisableDepthDS;
    ID3D11DepthStencilState* mNoDepthWritesDS;

    ID3D11BlendState* mAdditiveBlendState;
    bool mPerformBlend;
};