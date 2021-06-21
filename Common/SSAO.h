#pragma once

#include "d3dUtil.h"
#include "Camera.h"
#include <array>

class Ssao
{
public:
    Ssao(ID3D11Device* device, ID3D11DeviceContext* dc, int width, int height, float fovy, float farZ);
    ~Ssao();

    ID3D11ShaderResourceView* NormalDepthSRV();
    ID3D11ShaderResourceView* AmbientSRV();

    ///<summary>
    /// Call when the backbuffer is resized.
    ///</summary>
    void OnSize(int width, int height, float fovy, float farZ);

    ///<summary>
    /// Changes the render target to the NormalDepth render target. Pass the
    /// main depth buffer as the depth buffer to use when we render to the
    /// NormalDepth map. This pass lays down the scene depth so that there in
    /// no overdraw in the subsequent rendering pass.
    ///</summary>
    void SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv);


    ///<summary>
    /// Changes the render target to the Ambient render target and draws a fullscreen
    /// quad to kick off the pixel shader to compute the AmbientMap. We still keep the
    /// main depth buffer binded to the pipeline, but depth buffer read/writes
    /// are disabled, as we do not need the depth buffer computing the Ambient map.
    ///</summary>
    void ComputeSsao(const Camera& camera);

    ///<summary>
    /// Blurs the ambient map to smooth out the noise caused by only taking a
    /// few random samples per pixel. We use en edge preserving blur so that
    /// we do not blur across discontinuities--we want edges to remain edges.
    ///</summary>
    void BlurAmbientMap(int blurCount);

private:
    Ssao(const Ssao& rhs);
    Ssao& operator=(const Ssao& rhs);

    void BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur);

    void BuildFrustumFarCorners(float fovy, float farZ);

    void BuildFullScreenQuad();

    void BuildTextureViews();
    void ReleaseTextureViews();

    void BuildRandomVectorTexture();

    void BuildOffsetVectors();
    
    void BuildFX();

    //void DrawFullScreenQuad();

private:
    struct Vertex
    {
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT2 Tex;
    };
    
    struct alignas(16) BuildBuffer
    {
        XMMATRIX ViewToTexSpace;
        std::array<XMFLOAT4, 14> OffsetVectors;
        std::array<XMFLOAT4, 4> FrustumCorners;

        float OcclusionRadius;
        float OcclusionFadeStart;
        float OcclusionFadeEnd;
        float SurfaceEpsilon;
    };

    struct alignas(16) BlurBuffer
    {
        float TexelWidth;
        float TexelHeight;
        bool HorizontalBlur;
    };

    ID3D11Device* md3dDevice;
    ID3D11DeviceContext* mDC;

    ID3D11Buffer* mScreenQuadVB;
    ID3D11Buffer* mScreenQuadIB;

    ID3D11InputLayout* mInputLayout;

    ID3D11VertexShader* mSSAOVS;
    ID3D11PixelShader* mSSAOPS;
    ID3D11Buffer* mBuildBuffer;

    ID3D11SamplerState* mNormalDepthSampler;
    ID3D11SamplerState* mRandomVecSampler;

    ID3D11VertexShader* mBlurVS;
    ID3D11PixelShader* mBlurPS;
    ID3D11Buffer* mBlurBuffer;

    ID3D11SamplerState* mBlurSampler;

    ID3D11ShaderResourceView* mRandomVectorSRV;

    ID3D11RenderTargetView* mNormalDepthRTV;
    ID3D11ShaderResourceView* mNormalDepthSRV;

    // Need two for ping-ponging during blur.
    ID3D11RenderTargetView* mAmbientRTV0;
    ID3D11ShaderResourceView* mAmbientSRV0;

    ID3D11RenderTargetView* mAmbientRTV1;
    ID3D11ShaderResourceView* mAmbientSRV1;

    UINT mRenderTargetWidth;
    UINT mRenderTargetHeight;

    std::array<XMFLOAT4, 4> mFrustumCorner;
    
    std::array<XMFLOAT4, 14> mOffsets;

    D3D11_VIEWPORT mAmbientMapViewport;
};