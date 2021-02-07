///////////////////////////////////////////////////////////////////
// PLEASE READ
//
// The way I wrote this shader is complicated just to test
// the limits of what I can do. It is not meant to be a proper
// way to do this. I just wanted to limit the amount of hardcoded 
// values.
//
// You'll definitely get warnings in Debug mode.
//
// Then again, Subdivision, especially without Stream Out,
// really shouldn't be done in the Geometry Shader in the first
// place.
///////////////////////////////////////////////////////////////////

cbuffer PerObjectBuffer
{
    float4x4 gWorldView;
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

// Only gonna do position. Won't bother with lighting.
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

void Subdivide(VertexOut inVerts[3], out VertexOut outVerts[6])
{
    VertexOut m[3];
    
    // Only gonna do position. Won't bother with lighting.
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

void OutputSubdivision(VertexOut v[15], int numSubdivisions, inout TriangleStream<GeoOut> triStream)
{
    GeoOut gout[15];
    float4 currentColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    
    [unroll]
    for (int i = 0; i < 15; ++i)
    {
        gout[i].Pos = mul(float4(normalize(v[i].Pos), 1.0f), gWorldViewProj);
    }
    
    // Start with the top strip of size 1.
    // Each strip is two more triangles than the last.
    // There are 2^numSubdivisions strips
    int topIndex = 0;
    int bottomIndex = 1;
    unsigned int numInStrip = 1;
    [loop]
    for (int strip = 0; strip < pow(2, numSubdivisions); numInStrip += 2, ++strip)
    {
        // Since we are building a strip, we just alternate between
        // the bottom and top rows of the strip.
        // The values will be in the correct place for the next strip
        // when done with current one. See comments in Subdivide().
        triStream.Append(gout[bottomIndex]);
        ++bottomIndex;

        triStream.Append(gout[topIndex]);
        ++topIndex;
        
        [loop]
        for (unsigned int j = 0; j < numInStrip; ++j)
        {
            int currentIndex = (1 - j % 2) * bottomIndex + (j % 2) * topIndex;
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
    VertexOut bigv[15];
    
    // Otherwise it will complain
    int i = 0;
    
    [unroll]
    for (i = 0; i < 15; ++i)
    {
        bigv[i].Pos = float3(1.0f, 0.0f, 0.0f);
    }
    
    // Probably better to do this in the CPU
    float d = length(mul(float4(0.0, 0.0, 0.0, 1.0f), gWorldView));
    
    // Lol. Just to preserve a switch instead of using an if
    int numSubdivisions = 2 - (step(10, d) + step(20, d));
    
    // Ugh
    [call]
    switch (numSubdivisions)
    {
        case 0:
            [unroll]
            for (i = 0; i < 3; ++i)
            {
                bigv[i] = gin[i];
            }
            break;
        
        case 1:
            Subdivide(gin, v);
            [unroll]
            for (i = 0; i < 6; ++i)
            {
                bigv[i] = v[i];
            }
            break;
        
        case 2:
            Subdivide(gin, v);
    
            // No tri2 because we don't need to subdivide
            // the middle, "upside-down" triangle from
            // the first subdivision.
            VertexOut tri0[3] = { v[1], v[0], v[2] };
            VertexOut tri1[3] = { v[3], v[1], v[4] };
            VertexOut tri3[3] = { v[4], v[2], v[5] };
    
            // Make sure vertices are in top-to-bottom order
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
            break;
        
        // Shouldn't reach here. Coarse mesh will do
        // a better job of indicating an error.
        default:
            [unroll]
            for (i = 0; i < 3; ++i)
            {
                bigv[i] = gin[i];
            }
            break;
    }
    
    OutputSubdivision(bigv, numSubdivisions, stream);
}

float4 PS(GeoOut pin) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}