cbuffer PerObjectBuffer
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos : POSITION;
};

struct VertexOut
{
	float4 Pos : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.Pos = mul(float4(vin.Pos, 1.0f), gWorldViewProj);

	return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0, 1.0f);
}