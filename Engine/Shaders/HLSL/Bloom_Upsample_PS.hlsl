#include "Common.hlsl"

Texture2D<float4> LowResTexture : register(t0);
Texture2D<float4> HighResTexture : register(t1);

float4 main(float4 Position : SV_POSITION, float2 UV : TEXCOORD) : SV_Target0
{
    float4 low = LowResTexture.Sample(SamplerLinear, UV);
    float4 high = HighResTexture.Sample(SamplerLinear, UV);
    return low + high;
}
