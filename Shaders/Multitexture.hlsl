cbuffer PerFrame
{
    float4x4 gWorldViewProj;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

Texture2D gTex1 : register(t0);
Texture2D gTex2 : register(t1);
SamplerState gSample;

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    vout.TexC = vin.TexC;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return gTex1.Sample(gSample, pin.TexC) * gTex2.Sample(gSample, pin.TexC);
}