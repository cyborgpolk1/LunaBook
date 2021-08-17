#include "LightHelper.hlsl"

// Macros that can be redefined by the CPU need to be in guards to prevent redefinition
#ifndef NUM_LIGHTS
#define NUM_LIGHTS 3
#endif

cbuffer cbPerFrame : register(b0)
{
    float3 gEyePosW;
    
    float gHeightScale;
    float gMaxTessDistance;
    float gMinTessDistance;
    float gMinTessFactor;
    float gMaxTessFactor;
    
    float4x4 gViewProj;
    
    DirectionalLight gDirLights[3];
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gTexTransform;
    float4x4 gShadowTransform;
    Material gMaterial;
    
    int gOptions;
};

cbuffer cbSkinned : register(b2)
{
    // Max support of 96 bones per character.
    float4x4 gBoneTransforms[96];
};

Texture2D gTex : register(t0);
TextureCube gCubeMap : register(t1);
Texture2D gNormalMap : register(t2);
Texture2D gShadowMap : register(t3);

SamplerState gSample : register(s0);
SamplerComparisonState samShadow : register(s1);

struct BasicVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct PixelIn
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD0;
    float4 TangentW : TANGENT;
    float4 ShadowPosH : TEXCOORD1;
};

PixelIn BasicVS(BasicVertexIn vin)
{
    PixelIn vout;
    
    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    
    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);
    
    vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform).xy;
    
    vout.TangentW = 0.0f;
    
    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(float4(vout.PosW, 1.0f), gShadowTransform);
    
    return vout;
}

struct NormalVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    float3 TangentL : TANGENT;
};

PixelIn NormalVS(NormalVertexIn vin)
{
    PixelIn vout;
    
    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = float4(mul(vin.TangentL, (float3x3) gWorld), 1.0f);
    
    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);
    
    // Output vertex attributes for interpolation accross triangle.
    vout.TexC = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(float4(vout.PosW, 1.0f), gShadowTransform);
    
    return vout;
}

struct SkinnedVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    float4 TangentL : TANGENT;
    float3 Weights : WEIGHTS;
    uint4 BoneIndices : BoneIndices;
};

PixelIn SkinnedVS(SkinnedVertexIn vin)
{
    PixelIn vout;
    
    // Init array or else we get strange warnings about SV_POSITION
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vin.Weights.x;
    weights[1] = vin.Weights.y;
    weights[2] = vin.Weights.z;
    weights[3] = 1.0f - weights[0] - weights[1] - weights[2];
    
    // Do vertex blending
    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i)
    {
        // Asume no nonuniform scaling when transforming normals, so
        // that we do not have to use the inverse-transpose.
        posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(vin.NormalL, (float3x3) gBoneTransforms[vin.BoneIndices[i]]);
        tangentL += weights[i] * mul(vin.TangentL.xyz, (float3x3) gBoneTransforms[vin.BoneIndices[i]]);
    }
    
    // Transform to world space.
    vout.PosW = mul(float4(posL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(normalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = float4(mul(tangentL, (float3x3) gWorld), vin.TangentL.w);
    
    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);
    
    // Output vertex attributes for interpolation accross triangle.
    vout.TexC = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(float4(vout.PosW, 1.0f), gShadowTransform);
    
    return vout;
}

struct TessVertexOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
    float TessFactor : TESS;
};

TessVertexOut DispVS(NormalVertexIn vin)
{
    TessVertexOut vout;
    
    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3) gWorld);
    
    // Output vertex attributes for interpolation across triangle.
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    float d = distance(vout.PosW, gEyePosW);
    
    // Normalize tessellation factor.
    // The tessellation is
    //   0 if d >= gMinTessDistance and
    //   1 if d <= gMaxTessDistance.
    float tess = saturate((gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance));
    
    // Rescale [0, 1] --> [gMinTessFactor, gMaxTessFactor].
    vout.TessFactor = gMinTessFactor + tess * (gMaxTessFactor - gMinTessFactor);
    
    return vout;
}

struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<TessVertexOut, 3> patch, uint PatchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    // Average vertex tessellation factors along edges.
    // It is important to do the tessellation factor
    // calculation based on the edge properties so that edges shared by
    // more than one triangle will have the same tessellation factor.
    // Otherwise, gaps can appear.
    pt.EdgeTess[0] = 0.5f * (patch[1].TessFactor + patch[2].TessFactor);
    pt.EdgeTess[1] = 0.5f * (patch[2].TessFactor + patch[0].TessFactor);
    pt.EdgeTess[2] = 0.5f * (patch[0].TessFactor + patch[1].TessFactor);
    
    // Pick an edge tessellation factor for the interior tessellation.
    pt.InsideTess = pt.EdgeTess[0];
    
    return pt;
}

struct HullOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float4 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<TessVertexOut, 3> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    // Pass through shader.
    hout.PosW = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.TangentW = float4(p[i].TangentW, 1.0f);
    hout.Tex = p[i].Tex;
    
    return hout;
}

// The domain shader is called for every vertex created by the tessellator.
// It is like the vertex shader after tessellation.
[domain("tri")]
PixelIn DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
    PixelIn dout;
    
    // Interpolate patch attributes to generated vertices.
    dout.PosW = bary.x * tri[0].PosW + bary.y * tri[1].PosW + bary.z * tri[2].PosW;
    dout.NormalW = bary.x * tri[0].NormalW + bary.y * tri[1].NormalW + bary.z * tri[2].NormalW;
    dout.TangentW = bary.x * tri[0].TangentW + bary.y * tri[1].TangentW + bary.z * tri[2].TangentW;
    dout.TexC = bary.x * tri[0].Tex + bary.y * tri[1].Tex + bary.z * tri[2].Tex;
    
    // Interpolating normal can unnormalize it, so normalize it.
    dout.NormalW = normalize(dout.NormalW);
    
    //
    // Displacement mapping.
    //
    
    // Choose the mipmap level based on distance to the eye;
    // specifically, choose the next miplevel every MipInterval uints,
    // and clamp the miplevel in [0, 6].
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
    
    // Sample height map (stored in alpha channel).
    float h = gNormalMap.SampleLevel(gSample, dout.TexC, mipLevel).a;
    
    // Offset vertex along normal.
    dout.PosW += (gHeightScale * (h - 1.0f)) * dout.NormalW;
    
    // Generate projective tex-coords to project shadow map onto scene.
    dout.ShadowPosH = mul(float4(dout.PosW, 1.0f), gShadowTransform);
    
    // Project to homogenous clip space.
    dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
    
    return dout;
}

float4 PS(PixelIn pin) : SV_TARGET
{
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    if (gOptions & 0x01)
    {
        texColor = gTex.Sample(gSample, pin.TexC);

        if (gOptions & 0x02)
            clip(texColor.a - 0.1f);
    }

    // Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

    // The toEye vector is used in lighting.
    float3 toEye = gEyePosW - pin.PosW;
    
    // Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

    // Normalize.
    toEye /= distToEye;
    
    float3 bumpedNormalW = pin.NormalW;
    if (gOptions & 0x08)
    {
        float3 normalMapSample = gNormalMap.Sample(gSample, pin.TexC).rgb;
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
    }

    //
    // Lighting.
    //
    float4 litColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (NUM_LIGHTS > 0)
    {
        // Start with a sum of zero.
        float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
        
        // Only the first light casts a shadow.
        float3 shadow = float3(1.0f, 1.0f, 1.0f);
        shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

        // Sum the light contribution from each light source.
        [unroll]
        for (int i = 0; i < NUM_LIGHTS; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], bumpedNormalW, toEye, A, D, S);

            ambient += A;
            diffuse += shadow[i] * D;
            spec += shadow[i] * S;
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
    
    litColor.a = gMaterial.Diffuse.a * texColor.a;
    
    return litColor;
}