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
    
    float gHeightScale;
    float gMaxTessDistance;
    float gMinTessDistance;
    float gMinTessFactor;
    float gMaxTessFactor;
}

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gViewProj;
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
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    float3 TangentL : TANGENT;
};

struct VertexOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
    float TessFactor : TESS;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
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

PatchTess PatchHS(InputPatch<VertexOut, 3> patch, uint PatchID : SV_PrimitiveID)
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
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<VertexOut, 3> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    // Pass through shader.
    hout.PosW = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.TangentW = p[i].TangentW;
    hout.Tex = p[i].Tex;
    
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
    float3 TangentW : TANGENT;
};

// The domain shader is called for every vertex created by the tessellator.
// Is is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;
    
    // Interpolate patch attributes to generated vertices.
    dout.PosW     = bary.x * tri[0].PosW     + bary.y * tri[1].PosW     + bary.z * tri[2].PosW;
    dout.NormalW  = bary.x * tri[0].NormalW  + bary.y * tri[1].NormalW  + bary.z * tri[2].NormalW;
    dout.TangentW = bary.x * tri[0].TangentW + bary.y * tri[1].TangentW + bary.z * tri[2].TangentW;
    dout.Tex      = bary.x * tri[0].Tex      + bary.y * tri[1].Tex      + bary.z * tri[2].Tex;
    
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
    float h = gNormalMap.SampleLevel(gSample, dout.Tex, mipLevel).a;
    
    // Offset vertex along normal.
    dout.PosW += (gHeightScale * (h - 1.0f)) * dout.NormalW;
    
    // Project to homogenous clip space.
    dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
    
    return dout;
}