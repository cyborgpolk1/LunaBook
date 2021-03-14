cbuffer cbPerObject
{
    float4x4 gWorldInvTranspose;
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

PatchTess QuadConstantHS(InputPatch<VertexOut, 9> patch, uint patchId : SV_PrimitiveID)
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


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("QuadConstantHS")]
[maxtessfactor(64.0f)]
HullOut QuadHS(InputPatch<VertexOut, 9> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    
    hout.PosL = p[i].PosL;
    
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_Position;
    float3 NormalH : NORMAL;
};

float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float4(invT * invT * invT,  // (1 - t)^3
                  3.0f * t * invT * invT, // 3t(1 - t)^2
                  3.0f * t * t * invT, // 3(t^2)(1 - t)
                  t * t * t); // t^3

}

float3 QuadBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float3(invT * invT, // (1 - t)^2
                  2.0f * t * invT, // 2t(1 - t)
                  t * t); // t^2
}

float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float4(-3.0f * invT * invT, // -3(1 - t)^2
                  3.0f * invT * invT - 6.0f * t * invT, // 3(1 - t)^2 - 6t(1 - t)
                  6.0f * t * invT - 3.0f * t * t, // 6t(1 - t) - 3(t^2)
                  3.0f * t * t); // 3(t^2)
}

float3 dQuadBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float3(-2.0f * (1 - t), // -2(1 - t)
                  2.0f * (1 - 2.0f * t), // 2(1 - 2t)
                  2.0f * t); // 2t
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

float3 QuadBezierSum(const OutputPatch<HullOut, 9> bezpatch, float3 basisU, float3 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);

    sum += basisV.x * (basisU.x * bezpatch[0].PosL +
                       basisU.y * bezpatch[1].PosL +
                       basisU.z * bezpatch[2].PosL);
    
    sum += basisV.y * (basisU.x * bezpatch[3].PosL +
                       basisU.y * bezpatch[4].PosL +
                       basisU.z * bezpatch[5].PosL);
    
    sum += basisV.z * (basisU.x * bezpatch[6].PosL +
                       basisU.y * bezpatch[7].PosL +
                       basisU.z * bezpatch[8].PosL);
    
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
    
    float4 dBasisU = dBernsteinBasis(uv.x);
    float4 dBasisV = dBernsteinBasis(uv.y);
    
    float3 dpdu = CubicBezierSum(bezPatch, dBasisU, basisV);
    float3 dpdv = CubicBezierSum(bezPatch, basisU, dBasisV);
    
    dout.NormalH = mul(cross(dpdu, dpdv), (float3x3) gWorldInvTranspose);
    
    return dout;
}

[domain("quad")]
DomainOut QuadDS(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 9> bezPatch)
{
    DomainOut dout;
    
    float3 basisU = QuadBernsteinBasis(uv.x);
    float3 basisV = QuadBernsteinBasis(uv.y);
    
    float3 p = QuadBezierSum(bezPatch, basisU, basisV);
    
    dout.PosH = mul(float4(p, 1.0f), gWorldViewProj);
    
    float3 dBasisU = dQuadBernsteinBasis(uv.x);
    float3 dBasisV = dQuadBernsteinBasis(uv.y);
    
    float3 dpdu = QuadBezierSum(bezPatch, dBasisU, basisV);
    float3 dpdv = QuadBezierSum(bezPatch, basisU, dBasisV);
    
    dout.NormalH = mul(cross(dpdu, dpdv), (float3x3) gWorldInvTranspose);
    
    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    float ndotl = dot(normalize(pin.NormalH), -float3(-1.0f, -1.0f, 0.0));
    return float4(ndotl, ndotl, ndotl, 1.0f);
}