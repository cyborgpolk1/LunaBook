cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
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

	// Transform to homongenous clip space.
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	// Just pass vertex color into the pixel shader
	vout.Color = vin.Color;

	return vout;
}