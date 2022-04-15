RWTexture2D<float> tex;
[numthreads(1, 1, 1)]
void main(
    uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    tex[dispatchThreadId.xy] = float4(0.0, 0.0, 0.0, 255.0);
}