cbuffer cbPerFrame
{
    float4x4 gWorld;
    float4x4 gWorldView;
    float4x4 gWorldViewProj;
    float3 gEyePosW;
};

struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float3 PosL : POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosL = vin.PosL;
    
    return vout;
}

struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    // Find center of patch in wrold space.
    float3 centerL = 0.333f * (patch[0].PosL + patch[1].PosL + patch[2].PosL);
    float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz;
    
    float d = distance(centerW, gEyePosW);
    
    const float d0 = 2.0f;
    const float d1 = 25.0f;
    float tess = 16.0f * saturate((d1 - d) / (d1 - d0));
    
    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
    
    pt.InsideTess = tess;
    
    return pt;
}

struct HullOut
{
    float3 PosL : POSITION;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(16.0f)]
HullOut HS(InputPatch<VertexOut, 3> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    hout.PosL = p[i].PosL;
    
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_Position;
};

[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;
    
    // Barycentric coordinates
    float3 v = bary.r * tri[0].PosL + bary.g * tri[1].PosL + bary.b * tri[2].PosL;
    
    dout.PosH = mul(float4(normalize(v), 1.0f), gWorldViewProj);
    
    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}