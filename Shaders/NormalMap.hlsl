#include "LightHelper.hlsl"

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
}

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
    Material gMaterial;
    
    // Multiple options
    // Use Texture = 0x01
    // Use Alpha Clipping = 0x02
    // Enable Reflections (env mapping) = 0x03
    int gOptions = 0x01;
}

Texture2D gDiffuseMap : register(t0);
TextureCube gCubeMap : register(t1);
Texture2D gNormalMap : register(t2);

SamplerState gSample : register(s0);

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float2 Tex      : TEXCOORD;
    float3 TangentL : TANGENT;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex      : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3) gWorld);
    
    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    
    // Output vertex attributes for interpolation across triangle.
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    if (gOptions & 0x01)
    {
        texColor = gDiffuseMap.Sample(gSample, pin.Tex);

        if (gOptions & 0x02)
            clip(texColor.a - 0.1f);
    }
    
    // Interpolation normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);
    
    // The toEye vector is used in lighting.
    float3 toEye = gEyePosW - pin.PosW;
    
    // Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);
    
    // Normalize.
    toEye /= distToEye;

    //
    // Normal mapping
    //
    float3 normalMapSample = gNormalMap.Sample(gSample, pin.Tex).rgb;
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
    
    //
    // Lighting
    //
    float4 litColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (NUM_LIGHTS > 0)
    {
        // Start with a sum of zero.
        float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
        
        // Sum the light contribution from each light source.
        [unroll]
        for (int i = 0; i < NUM_LIGHTS; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], bumpedNormalW, toEye, A, D, S);

            ambient += A;
            diffuse += D;
            spec += S;
        }

        litColor = texColor * (ambient + diffuse) + spec;
        
        // Environment Mapping
        if (gOptions & 0x04)
        {
            float3 incident = -toEye;
            float3 reflectionVector = reflect(incident, bumpedNormalW);
            float4 reflectionColor = gCubeMap.Sample(gSample, reflectionVector);

            litColor += gMaterial.Reflect * reflectionColor;
        }
    }
    
#ifdef FOG
    float fogLerp = saturate((distToEye - gFogStart) / gFogRange);
    
    litColor = lerp(litColor, gFogColor, fogLerp);
#endif
    
#ifdef OVERDRAW
    litColor = float4(0.1f, 0.1f, 0.1f, 1.0f);
#else
    // Common to take alpha from diffuse material.
    litColor.a = gMaterial.Diffuse.a * texColor.a;
#endif
    
    return litColor;
}