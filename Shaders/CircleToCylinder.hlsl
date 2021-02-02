cbuffer cbPerObject
{
    float4x4 gWorldViewProj;
};

struct VertexIn
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float3 Pos : POSTION;
    float4 Color : COLOR;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.Pos = vin.Pos;
    vout.Color = vin.Color;
    
    return vout;
}

[maxvertexcount(6)]
void GS(line VertexOut gin[2], inout TriangleStream<GeoOut> stream)
{
    // Vertices will be appended in reverse order to preserve CW front faces
    GeoOut gout;
    [unroll]
    for (int i = 1; i >= 0; --i)
    {
        gout.PosH = mul(float4(gin[i].Pos, 1.0f), gWorldViewProj);
        gout.Color = gin[i].Color;
        
        stream.Append(gout);
    }

    
    //float4x4 translate = float4x4(1, 0, 0, 0,
    //                              0, 1, 0, 2,
    //                              0, 0, 1, 0,
    //                              0, 0, 0, 1);
    [unroll]
    for (int j = 0; j < 2; ++j)
    {
        for (i = 1; i >= 0; --i)
        {
            gout.PosH = mul(float4(gin[i].Pos, 1.0f) + float4(0.0f, 0.5f + 0.5f * j, 0.0f, 0.0f), gWorldViewProj);
            gout.Color = gin[i].Color + float4(0.0f, 0.0f, 0.5f + 0.5f * j, 0.0f);
        
            stream.Append(gout);
        }
    }
}


float4 PS(GeoOut pin) : SV_TARGET
{
    return pin.Color;
}