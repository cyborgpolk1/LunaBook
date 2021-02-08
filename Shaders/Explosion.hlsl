cbuffer PerObjectBuffer
{
    float4x4 gWorldViewProj;
    float gTime;
};

struct VertexIn
{
    float3 Pos : POSITION;
};

struct VertexOut
{
    float3 Pos : POSITION;
};

struct GeoOut
{
    float4 Pos : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.Pos = vin.Pos;
    
    return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
    GeoOut gout;
    
    float3 e1 = gin[0].Pos - gin[1].Pos;
    float3 e2 = gin[0].Pos - gin[2].Pos;
    float3 normal = normalize(cross(e1, e2));
    
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float3 newPos = gin[i].Pos + 0.5f * gTime * normal;
        gout.Pos = mul(float4(newPos, 1.0f), gWorldViewProj);
        stream.Append(gout);
    }
}

float4 PS(GeoOut pin) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}