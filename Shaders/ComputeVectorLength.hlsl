struct Data
{
    float3 vec;
};

StructuredBuffer<Data> gStructInput : register(t0);
RWStructuredBuffer<float> gStructOutput : register(u0);

[numthreads(64, 1, 1)]
void StructCS(int3 dtid : SV_DispatchThreadID)
{
    gStructOutput[dtid.x] = length(gStructInput[dtid.x].vec);
}

Buffer<float3> gTypedInput : register(t1);
RWBuffer<float> gTypedOutput : register(u1);

[numthreads(64, 1, 1)]
void TypedCS(int3 dtid : SV_DispatchThreadID)
{
    gTypedOutput[dtid.x] = length(gTypedInput[dtid.x]);
}

ConsumeStructuredBuffer<Data> gConsumeInput : register(u2);
AppendStructuredBuffer<float> gAppendOutput : register(u3);

[numthreads(64, 1, 1)]
void AppendCS()
{
    Data v = gConsumeInput.Consume();
    gAppendOutput.Append(length(v.vec));
}