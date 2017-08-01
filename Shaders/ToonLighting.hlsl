#include "LightHelper.hlsl"

cbuffer cbPerFrame : register(b0)
{
    DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    float3 gEyePosW;
    float pad;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material gMaterial;
};

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);

    // Transform to homogenous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    return vout;
}

void ComputeToonDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye,
                             out float4 ambient, out float4 diffuse, out float4 spec);
void ComputeToonPointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
                        out float4 ambient, out float4 diffuse, out float4 spec);
void ComputeToonSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
                        out float4 ambient, out float4 diffuse, out float4 spec);

float4 PS(VertexOut pin) : SV_Target
{
    // Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Start with a sum of zero.
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // sum the light contribution from each light source.
    float4 A, D, S;

    ComputeToonDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeToonPointLight(gMaterial, gPointLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeToonSpotLight(gMaterial, gSpotLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;

    float4 litColor = ambient + diffuse + spec;

    // Common to take alpha from diffuse material.
    litColor.a = gMaterial.Diffuse.a;

    return litColor;
}

void ToonDiffuse(inout float factor)
{
    if(factor <= 0.0f)
        factor = 0.4f;
    else if(factor <= 0.5f)
        factor = 0.6f;
}

void ToonSpec(inout float factor)
{
    if (factor >= 0.0f && factor <= 0.1f)
        factor = 0.0f;
    else if (factor <= 0.8)
        factor = 0.5f;
    else
        factor = 0.8f;
}

void ComputeToonDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye,
                             out float4 ambient, out float4 diffuse, out float4 spec)
{
    // Initialize outputs
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

    // Add ambient term.
    ambient = mat.Ambient * L.Ambient;

    // Add diffuse and specular term, provided the surface is in the line of sight of the light.
    float diffuseFactor = dot(lightVec, normal);

    ToonDiffuse(diffuseFactor);

    // Flatten to avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        ToonSpec(specFactor);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
}

void ComputeToonPointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
                        out float4 ambient, out float4 diffuse, out float4 spec)
{
    // Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > L.Range)
        return;

    // Normalize the light vector.
    lightVec /= d;

    // Ambient term.
    ambient = mat.Ambient * L.Ambient;

    // Add diffuse and specular term, provided the surface is in the line of sight of the light.
    float diffuseFactor = dot(lightVec, normal);

    ToonDiffuse(diffuseFactor);

    // Flatten to avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        ToonSpec(specFactor);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

    // Attenuate
    float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));
    
    diffuse *= att;
    spec *= att;
}

void ComputeToonSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
                        out float4 ambient, out float4 diffuse, out float4 spec)
{
    // Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > L.Range)
        return;

    // Normalize the light vector.
    lightVec /= d;

    // Ambient term.
    ambient = mat.Ambient * L.Ambient;

    // Add diffuse and specular term, provided the surface is in the line of sight of the light
    float diffuseFactor = dot(lightVec, normal);

    ToonDiffuse(diffuseFactor);

    // Flatten to avoid dynamic branching
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

        ToonSpec(specFactor);

        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }

    // Scale by spotlight factor and attenuate
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

    float att = spot / dot(L.Att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}