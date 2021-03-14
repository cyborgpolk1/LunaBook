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

struct TriPatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

TriPatchTess TriConstantHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
    TriPatchTess pt;
    
    switch (patchID)
    {
        case 0:
            pt.EdgeTess[0] = 4;
            pt.EdgeTess[1] = 4;
            pt.EdgeTess[2] = 4;
            pt.InsideTess = 4;
            break;
        
        case 1:
            pt.EdgeTess[0] = 1;
            pt.EdgeTess[1] = 2;
            pt.EdgeTess[2] = 3;
            pt.InsideTess = 4;
            break;
        
        case 2:
            pt.EdgeTess[0] = 6;
            pt.EdgeTess[1] = 6;
            pt.EdgeTess[2] = 6;
            pt.InsideTess = 3;
            break;
        
        default:
            pt.EdgeTess[0] = 6;
            pt.EdgeTess[1] = 12;
            pt.EdgeTess[2] = 3;
            pt.InsideTess = 1;
            break;  
    }
    
    return pt;
}

struct QuadPatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

QuadPatchTess QuadConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
    QuadPatchTess pt;
    
    switch (patchID)
    {
        case 0:
            pt.EdgeTess[0] = 4;
            pt.EdgeTess[1] = 4;
            pt.EdgeTess[2] = 4;
            pt.EdgeTess[3] = 4;
            pt.InsideTess[0] = 4;
            pt.InsideTess[1] = 4;
            break;
        
        case 1:
            pt.EdgeTess[0] = 1;
            pt.EdgeTess[1] = 2;
            pt.EdgeTess[2] = 3;
            pt.EdgeTess[3] = 4;
            pt.InsideTess[0] = 4;
            pt.InsideTess[1] = 4;
            break;
        
        case 2:
            pt.EdgeTess[0] = 2;
            pt.EdgeTess[1] = 2;
            pt.EdgeTess[2] = 4;
            pt.EdgeTess[3] = 4;
            pt.InsideTess[0] = 2;
            pt.InsideTess[1] = 4;
            break;
        
        default:
            pt.EdgeTess[0] = 4;
            pt.EdgeTess[1] = 4;
            pt.EdgeTess[2] = 4;
            pt.EdgeTess[3] = 4;
            pt.InsideTess[0] = 3;
            pt.InsideTess[1] = 3;
            break;
    }
    
    return pt;
}

struct HullOut
{
    float3 PosL : POSITION;
};

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("TriConstantHS")]
[maxtessfactor(64.0f)]
HullOut TriHS(InputPatch<VertexOut, 3> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    hout.PosL = p[i].PosL;
    
    return hout;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("QuadConstantHS")]
[maxtessfactor(64.0f)]
HullOut QuadHS(InputPatch<VertexOut, 4> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    hout.PosL = p[i].PosL;
    
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
};

[domain("tri")]
DomainOut TriDS(TriPatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;
    
    dout.PosH.xyz = bary.r * tri[0].PosL + bary.g * tri[1].PosL + bary.b * tri[2].PosL;
    dout.PosH.w = 1.0f;
    
    return dout;
}

[domain("quad")]
DomainOut QuadDS(QuadPatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;
    
    float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
    float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
    dout.PosH.xyz = lerp(v1, v2, uv.y);
    dout.PosH.w = 1.0f;
    
    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}