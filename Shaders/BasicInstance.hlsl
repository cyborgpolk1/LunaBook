#include "LightHelper.hlsl"

cbuffer cbPerFrame : register(b0)
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gViewProj;
    Material gMaterial;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    row_major float4x4 World : WORLD;
    float4 Color : COLOR;
    uint InstanceId : SV_InstanceID;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), vin.World).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) vin.World);
    
    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);
    
    // Output vertex attributes for interpolation across triangle.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);

    float3 toEye = normalize(gEyePosW - pin.PosW);

    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float4 A, D, S;
        ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);

        ambient += A * pin.Color;
        diffuse += D * pin.Color;
        spec += S;
    }
    
    float4 litColor = ambient + diffuse + spec;
    litColor.a = gMaterial.Diffuse.a;
    
    return litColor;
}