cbuffer cbPerObject
{
    float4x4 gWorldViewProj;
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
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 16> patch, uint patchId : SV_PrimitiveID)
{
    PatchTess pt;
    
    // Uniform tessellation for this demo.
    
    pt.EdgeTess[0] = 25;
    pt.EdgeTess[1] = 25;
    pt.EdgeTess[2] = 25;
    pt.EdgeTess[3] = 25;
    
    pt.InsideTess[0] = 25;
    pt.InsideTess[1] = 25;
    
    return pt;
}

struct HullOut
{
    float3 PosL : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 16> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    hout.PosL = p[i].PosL;
    
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_Position;
};

float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float4(invT * invT * invT,  // (1 - t)^3
                  3.0f * t * invT * invT, // 3t(1 - t)^2
                  3.0f * t * t * invT, // 3(t^2)(1 - t)
                  t * t * t); // t^3

}

float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float4(-3.0f * invT * invT, // -3(1 - t)^2
                  3.0f * invT * invT - 6.0f * t * invT, // 3(1 - t)^2 - 6t(1 - t)
                  6.0f * t * invT - 3.0f * t * t, // 6t(1 - t) - 3(t^2)
                  3.0f * t * t); // 3(t^2)
}

float3 CubicBezierSum(const OutputPatch<HullOut, 16> bezpatch, float4 basisU, float4 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    
    sum += basisV.x * (basisU.x * bezpatch[0].PosL +
                       basisU.y * bezpatch[1].PosL +
                       basisU.z * bezpatch[2].PosL +
                       basisU.w * bezpatch[3].PosL);
    
    sum += basisV.y * (basisU.x * bezpatch[4].PosL +
                       basisU.y * bezpatch[5].PosL +
                       basisU.z * bezpatch[6].PosL +
                       basisU.w * bezpatch[7].PosL);
    
    sum += basisV.z * (basisU.x * bezpatch[8].PosL +
                       basisU.y * bezpatch[9].PosL +
                       basisU.z * bezpatch[10].PosL +
                       basisU.w * bezpatch[11].PosL);
    
    sum += basisV.w * (basisU.x * bezpatch[12].PosL +
                       basisU.y * bezpatch[13].PosL +
                       basisU.z * bezpatch[14].PosL +
                       basisU.w * bezpatch[15].PosL);
    
    
    return sum;
}

[domain("quad")]
DomainOut DS(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 16> bezPatch)
{
    DomainOut dout;
    
    float4 basisU = BernsteinBasis(uv.x);
    float4 basisV = BernsteinBasis(uv.y);
    
    float3 p = CubicBezierSum(bezPatch, basisU, basisV);
    
    dout.PosH = mul(float4(p, 1.0f), gWorldViewProj);
    
    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}