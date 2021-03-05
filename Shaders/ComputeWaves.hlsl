#include "BasicEffectTex.hlsl"

cbuffer cbUpdateSettings : register(b0)
{
    float4 gWaveConstants;
};

cbuffer cbDisturbSettings : register(b1)
{
    int2 gDisturbCoord;
    float gMagnitude;
};

Texture2D gPrevSolInput : register(t0);
Texture2D gCurrSolInput : register(t1);
RWTexture2D<float> gCurrSolOutput : register(u0);
RWTexture2D<float> gNextSolOutput : register(u1);

[numthreads(16, 16, 1)]
void UpdateCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = dispatchThreadID.x;
    int y = dispatchThreadID.y;
    
    gNextSolOutput[int2(x, y)] =
        gWaveConstants.x * gPrevSolInput[int2(x, y)].r +
        gWaveConstants.y * gCurrSolInput[int2(x, y)].r +
        gWaveConstants.z * (
            gCurrSolInput[int2(x, y + 1)].r +
            gCurrSolInput[int2(x, y - 1)].r +
            gCurrSolInput[int2(x + 1, y)].r +
            gCurrSolInput[int2(x - 1, y)].r);
}

[numthreads(16, 16, 1)]
void DisturbCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = dispatchThreadID.x;
    int y = dispatchThreadID.y;
    
    float dist = clamp(ceil(length(float2(gDisturbCoord - float2(x, y)))), 0, 2);
    
    gCurrSolOutput[int2(x, y)] = (2 - dist) * 0.5f * gMagnitude + gCurrSolOutput[int2(x, y)];
}

cbuffer cbPerWave : register(b2)
{
    float2 gDisplacmentTexelSize;
    float gGridSpatialStep;
    float pad;
};

Texture2D gDisplacementMap : register(t2);
SamplerState samDisplacement : register(s1);

VertexOut WavesVS(VertexIn vin)
{
    VertexOut vout;
    
    // Sample the displacement map using non-transformed
    // [0, 1]^2 tex-coords.
    vin.PosL.y = gDisplacementMap.SampleLevel(samDisplacement, vin.TexC, 0.0f).r;

    // Estimate normal using finite difference.
    float du = 1.0f / 512.0f;
    float dv = 1.0f / 512.0f;
    float l = gDisplacementMap.SampleLevel(samDisplacement, vin.TexC - float2(du, 0.0f), 0.0f).r;
    float r = gDisplacementMap.SampleLevel(samDisplacement, vin.TexC + float2(du, 0.0f), 0.0f).r;
    float t = gDisplacementMap.SampleLevel(samDisplacement, vin.TexC - float2(0.0f, dv), 0.0f).r;
    float b = gDisplacementMap.SampleLevel(samDisplacement, vin.TexC + float2(0.0f, dv), 0.0f).r;
    vin.NormalL = normalize(float3(-r + l, 2.0f * gGridSpatialStep, b - t));
    
    return VS(vin);
}