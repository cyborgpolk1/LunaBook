cbuffer PerObjectBuffer
{
    float4x4 gWorld;
    float4x4 gViewProj;
    float4x4 gWorldInvTranspose;
};

struct VertexIn
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct VertexOut
{
    float4 Pos : SV_POSITION;
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.Pos = mul(float4(vin.Pos, 1.0f), mul(gWorld, gViewProj));

    return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

// FACE NORMALS
struct NormalVertexOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
};

struct GeoOut
{
    float4 Pos : SV_POSITION;
    float3 Color : COLOR;
};

NormalVertexOut NormalVS(VertexIn vin)
{
    NormalVertexOut vout;
    
    vout.PosW = mul(float4(vin.Pos, 1.0f), gWorld).xyz;
    vout.NormalW = normalize(mul(vin.Normal, (float3x3) gWorldInvTranspose));

    return vout;
}

[maxvertexcount(2)]
void FaceNormalGS(triangle NormalVertexOut gin[3], inout LineStream<GeoOut> stream)
{
    GeoOut gout;
    
    gout.Color = float3(1.0f, 0.0f, 1.0f);
    
    float3 center = (1.0 / 3.0f) * (gin[0].PosW + gin[1].PosW + gin[2].PosW);
    
    float3 faceNormal = normalize((1.0f / 3.0f) * (gin[0].NormalW + gin[1].NormalW + gin[2].NormalW));
    
    gout.Pos = mul(float4(center, 1.0f), gViewProj);
    stream.Append(gout);

    gout.Pos = mul(float4(center + faceNormal, 1.0f), gViewProj);
    stream.Append(gout);
}

[maxvertexcount(2)]
void VertexNormalGS(point NormalVertexOut gin[1], inout LineStream<GeoOut> stream)
{
    GeoOut gout;
    
    gout.Color = float3(0.0f, 1.0f, 1.0f);
    
    gout.Pos = mul(float4(gin[0].PosW, 1.0f), gViewProj);
    stream.Append(gout);

    gout.Pos = mul(float4(gin[0].PosW + gin[0].NormalW, 1.0f), gViewProj);
    stream.Append(gout);
}

float4 NormalPS(GeoOut pin) : SV_TARGET
{
    return float4(pin.Color, 1.0f);
}
