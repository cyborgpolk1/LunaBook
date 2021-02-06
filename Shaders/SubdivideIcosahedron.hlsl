cbuffer PerObjectBuffer
{
    //float4x4 gWorldView;
	float4x4 gWorldViewProj;
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
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    vout.Pos = vin.Pos;

	return vout;
}

void Subdivide(VertexOut inVerts[3], out VertexOut outVerts[6])
{
    VertexOut m[3];
    
    m[0].Pos = 0.5f * (inVerts[0].Pos + inVerts[1].Pos);
    m[1].Pos = 0.5f * (inVerts[1].Pos + inVerts[2].Pos);
    m[2].Pos = 0.5f * (inVerts[2].Pos + inVerts[0].Pos);

    // Output from top to bottom
    outVerts[0] = inVerts[1];
    outVerts[1] = m[0];
    outVerts[2] = m[1];
    outVerts[3] = inVerts[0];
    outVerts[4] = m[2];
    outVerts[5] = inVerts[2];
}

void OutputSubdivision(VertexOut v[15], inout TriangleStream<GeoOut> triStream)
{
    GeoOut gout[15];
    float4 currentColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    
    [unroll]
    for (int i = 0; i < 15; ++i)
    {
        gout[i].Pos = mul(float4(normalize(v[i].Pos), 1.0f), gWorldViewProj);
        gout[i].Color = currentColor;
    }
    
    // Start with the top strip of size 1.
    // Each strip is two more triangles than the last.
    // There are 2^numSubdivisions strips
    int topIndex = 0;
    int bottomIndex = 1;
    for (unsigned int numInStrip = 1; bottomIndex < 15; numInStrip += 2)
    {
        gout[bottomIndex].Color = currentColor;
        currentColor = currentColor.brga;
        triStream.Append(gout[bottomIndex]);
        ++bottomIndex;
        
        gout[topIndex].Color = currentColor;
        currentColor = currentColor.brga;
        triStream.Append(gout[topIndex]);
        ++topIndex;
        
        for (unsigned int j = 0; j < numInStrip; ++j)
        {
            int currentIndex = (1 - j % 2) * bottomIndex + (j % 2) * topIndex;
            gout[currentIndex].Color = currentColor;
            currentColor = currentColor.brga;
            triStream.Append(gout[currentIndex]);
            bottomIndex = (1 - j % 2) * (bottomIndex + 1) + (j % 2) * bottomIndex;
            topIndex = (1 - j % 2) * topIndex + (j % 2) * (topIndex + 1);
        }
        
        triStream.RestartStrip();
    }

}

[maxvertexcount(24)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
    VertexOut v[6];
    Subdivide(gin, v);
    
    VertexOut tri0[3] = { v[1], v[0], v[2] };
    VertexOut tri1[3] = { v[3], v[1], v[4] };
    VertexOut tri3[3] = { v[4], v[2], v[5] };
    
    VertexOut bigv[15];
    
    Subdivide(tri0, v);
    bigv[0] = v[0];
    bigv[1] = v[1];
    bigv[2] = v[2];
    bigv[3] = v[3];
    bigv[4] = v[4];
    bigv[5] = v[5];
    
    Subdivide(tri1, v);
    bigv[6] = v[1];
    bigv[7] = v[2];
    bigv[10] = v[3];
    bigv[11] = v[4];
    bigv[12] = v[5];
    
    Subdivide(tri3, v);
    bigv[8] = v[1];
    bigv[9] = v[2];
    bigv[13] = v[4];
    bigv[14] = v[5];
    
    OutputSubdivision(bigv, stream);
}

float4 PS(GeoOut pin) : SV_TARGET
{
    return pin.Color;
}