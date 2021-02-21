Texture2D gInputA : register(t0);
Texture2D gInputB : register(t1);
RWTexture2D<float4> gOutput : register(u0);

[numthreads(16, 16, 1)]
void CS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    gOutput[dispatchThreadID.xy] =
        gInputA[dispatchThreadID.xy] +
        gInputB[dispatchThreadID.xy];
}