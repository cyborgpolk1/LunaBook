cbuffer QuadColor
{
    float4 color;
};

struct VertexIn
{
    float3 pos : POSITION;
};

struct VertexOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.pos = float4(vin.pos, 1.0f);
    vout.color = color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.color;
}