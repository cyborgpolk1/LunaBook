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

Texture2D gTex;
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
    return gTex.Sample(gSample, pin.TexC);
}

float4 GrayscalePS(VertexOut pin) : SV_TARGET
{
    float4 c = gTex.Sample(gSample, pin.TexC).r;
    return float4(c.rrr, 1.0f);
}