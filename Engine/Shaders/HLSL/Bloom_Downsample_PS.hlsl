#include "Common.hlsl"

Texture2D<float4> SourceTexture : register(t0);

float4 main(float4 Position : SV_POSITION, float2 UV : TEXCOORD) : SV_Target0
{
    float2 texelSize = 1.0 / ViewportSize;
    float2 offsets[4] = {
        float2(-0.5, -0.5),
        float2(0.5, -0.5),
        float2(-0.5, 0.5),
        float2(0.5, 0.5)
    };
    float3 color = 0;
    for (int i = 0; i < 4; ++i)
        color += SourceTexture.Sample(SamplerLinear, UV + offsets[i] * texelSize).xyz;
    return float4(color * 0.25, 1.0);
}
