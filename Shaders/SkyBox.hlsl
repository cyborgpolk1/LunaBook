cbuffer cbPerFrame : register(b0)
{
    float4x4 gWorldViewProj;
};

TextureCube gCubeMap : register(t0);
SamplerState samTriLinear : register(s0);

struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Set z = w so that z/w = 1 (i.e. skydome always on far plane).
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
    
    // Use local vertex position as cubemap lookup vector.
    vout.PosL = vin.PosL;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{
    return gCubeMap.Sample(samTriLinear, pin.PosL);
}