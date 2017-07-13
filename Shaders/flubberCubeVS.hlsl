// EXERCISE 5.6
cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
    float gTime;
};

struct VertexIn
{
	float3 Pos		: POSITION;
	float4 Color	: COLOR;
};

struct VertexOut
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

    vin.Pos.xy += 0.5f * sin(vin.Pos.x) * sin(3.0f * gTime);
    vin.Pos.z *= 0.6f + 0.4f * sin(2.0f * gTime);

	// Transform to homongenous clip space.
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	// Just pass vertex color into the pixel shader
	vout.Color = vin.Color;

	return vout;
}