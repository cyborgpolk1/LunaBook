#include "LightHelper.hlsl"

// Macros that can be redefined by the CPU need to be in guards to prevent redefinition
#ifndef NUM_LIGHTS
#define NUM_LIGHTS 3
#endif

cbuffer cbPerFrame : register(b0)
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;

    float gFogStart;
    float gFogRange;
    float4 gFogColor;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    float4x4 fTexTransform;
    Material gMaterial;
};

Texture2D gDiffuseMap;

SamplerState samAnisotropic;

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);

    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

    // The toEye vector is used in lighting.
    float3 toEye = gEyePosW - pin.PosW;

    // Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

    // Normalize.
    toEye /= distToEye;

    //
    // Lighting.
    //


    // Start with a sum of zero.
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Sum the light contribution from each light source.
    [unroll]
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        float4 A, D, S;
        ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);

        ambient += A;
        diffuse += D;
        spec += S;
    }

    float4 litColor = ambient + diffuse + spec;

    // Common to take alpha from diffuse material.
    litColor.a = gMaterial.Diffuse.a;

    return litColor;
}