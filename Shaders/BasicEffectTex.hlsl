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
	float4 gFogColor;
    float gFogRange;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
    Material gMaterial;
};

Texture2D gTex : register(t0);
SamplerState gSample : register(s0);

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
	float2 TexC		: TEXCOORD;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float2 TexC		: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);

    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 texColor = gTex.Sample(gSample, pin.TexC);
#ifdef CLIP
	clip(texColor.a - 0.1f);
#endif

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

	float4 litColor = texColor * (ambient + diffuse) + spec;

#ifdef FOG
	float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

	litColor = lerp(litColor, gFogColor, fogLerp);
#endif

    // Common to take alpha from diffuse material.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}