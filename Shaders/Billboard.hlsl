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
	float4x4 gViewProj;
	Material gMaterial;
};

cbuffer cbFixed
{
    static const float2 gTexC[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };
};



Texture2DArray gTreeMapArray : register(t0);

SamplerState gSample : register(s0);

struct VertexIn
{
    float3 PosW     : POSITION;
    float2 SizeW    : SIZE;
};

struct VertexOut
{
    float3 CenterW  : POSITION;
    float2 SizeW    : SIZE;
};

struct GeoOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float2 Tex      : TEXCOORD;
    uint   PrimID   : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;
    
    return vout;
}

// We expand each point into a quad (4 vertices), so the maximum number
// of vertices we output per geometry shader invocation is 4.
[maxvertexcount(4)]
void GS(point VertexOut gin[1], uint primID : SV_PrimitiveID, inout TriangleStream<GeoOut> triStream)
{
    //
    // Compute the local coordinate system of the sprite relative to the
    // world space such  that the billboard is aligned with the y-axis
    // and faces the eye
    //
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f; // y-axis aligned, so project to xz-plane
    look = normalize(look);
    float3 right = cross(up, look);

    //
    // Compute triangle strip vertices (quad) in world space.
    //
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);
    
    //
    // Transform quad vertices to world space and output them as a triangle strip
    //
    GeoOut gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.Tex = gTexC[i];
        gout.PrimID = primID;
        
        triStream.Append(gout);
    }
}

float4 PS(GeoOut pin) : SV_Target
{
    // Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

    // The toEye vector is used in lightning.
    float3 toEye = gEyePosW - pin.PosW;
    
    // Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

    // Normalize.
    toEye /= distToEye;
    
    // Sample texture.
    float3 uvw = float3(pin.Tex, pin.PrimID % 4);
    float4 texColor = gTreeMapArray.Sample(gSample, uvw);
    
    clip(texColor.a - 0.1f);

    // Lighting.
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

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
    
#ifdef OVERDRAW
    litColor = float4(0.1f, 0.1f, 0.1f, 1.0f);
#else
    litColor.a = gMaterial.Diffuse.a * texColor.a;
#endif
    
    return litColor;
}